#!/usr/bin/env ts-node
import * as fs from "fs";
import * as path from "path";
const chalk = require("chalk");
import minimist = require("minimist");
import * as Long from "long";
import WASMC, {SymbolTable} from "./wasmc";
import Parser from "../wvm/parser.js";
import _ from "../util";

const {FLAGS, EXCEPTIONS, SYMBOL_TYPES} = require("./constants.js");

/**
 * @module wasm
 */

require("../util.js");
require("string.prototype.padstart").shim();
require("string.prototype.padend").shim();

/**
 * Represents a `wld` instance.
 */
export default class Linker {
	/**
	 * The parser used to parse the main object file.
	 */
	parser: Parser;

	options: {[key: string]: any};
	objectFilenames: string[];
	outputFilename: string;
	combinedSymbols: SymbolTable;
	combinedCode: Long[];
	combinedData: Long[];
	static assembler: WASMC = new WASMC();

	constructor(options: {[key: string]: any}, objects: string[], out: string) {
		this.options = options;
		this.objectFilenames = objects;
		this.outputFilename = out;

		this.parser = null;
	}

	/**
	 * Opens the main file and handles it depending on whether it's wasm source or wvm bytecode.
	 * @param  filename The filename of the main file.
	 * @return A Parser instance that has been fed the main file.
	 */
	openMain(filename: string): Parser {
		const text: string = fs.readFileSync(filename, "utf8");
		let bytecode: string;
		
		if (text.match(/#(meta|data|includes|code)/)) {
			// The file is probably wasm source code.
			// We take note of its #includes section and add the listed files to `objectFilenames`.
			// We then compile it and create a parser from the compiled code.
			Linker.assembler.compile(text);
			bytecode = WASMC.longs2strs(Linker.assembler.assembled).join("\n");
			if (Linker.assembler.parsed.includes) {
				// Included files are relative to the source file, so we need to resolve the paths.
				this.objectFilenames.splice(1, 0,
				    ...Linker.assembler.parsed.includes.map(f => path.resolve(path.dirname(filename), f)));
				// In case the main source includes a file already specified as an input from the command line,
				// we need to remove duplicates.
				this.objectFilenames = _.uniq(this.objectFilenames);
			}
		} else {
			if (this.objectFilenames.length < 2) {
				console.error(chalk.yellow.bold(" ?"), `Multiple input files are needed.`);
				process.exit(1);
			}

			bytecode = text;
		}

		const parser: Parser = new Parser();
		parser.read(bytecode);
		return parser;
	}

	link() {
		// Step 1
		this.parser = this.openMain(this.objectFilenames[0]);

		// Step 2
		const metaLength = this.parser.getMetaLength();
		const codeLength = this.parser.getCodeLength();
		const dataLength = this.parser.getDataLength();
		const mainSymbols = this.parser.getSymbols();
		const symtabLength = this.parser.rawSymbols.length;
		
		/**
		 * Contains the combination of all parsed symbol tables.
		 * @type {SymbolTable}
		 */
		this.combinedSymbols = Linker.depointerize(_.cloneDeep(mainSymbols), this.parser.rawData, this.parser.offsets);
		
		/**
		 * Contains the combination of all parsed code sections.
		 * @type {Long[]}
		 */
		this.combinedCode = _.cloneDeep(this.parser.rawCode);

		/**
		 * Contains the combination of all parsed data sections.
		 * @type {Long[]}
		 */
		this.combinedData = _.cloneDeep(this.parser.rawData);

		// We need to keep track of symbol types separately because it becomes difficult to recompute them
		// after the symbol table has been expanded with new symbols from included binaries, as the boundaries
		// between code sections and data sections become murky.
		const symbolTypes = Linker.collectSymbolTypes(this.parser.offsets, this.combinedSymbols);

		// Step 3
		Linker.desymbolize(this.combinedCode, mainSymbols, this.parser.offsets);

		// Steps 4–6
		let extraSymbolLength = symtabLength * 8;
		let extraCodeLength = codeLength;
		let extraDataLength = dataLength;

		// Step 7: Loop over every inclusion.
		for (const infile of this.objectFilenames.slice(1)) {
			if (!fs.existsSync(infile)) {
				console.error(chalk.red.bold(" !"), `Couldn't find ${chalk.bold(infile)}.`);
				process.exit(1);
			}

			// Step 7a: Open the included binary.
			const subparser = new Parser();
			subparser.open(infile);

			const subcode = subparser.rawCode;
			const subdata = subparser.rawData;
			const subtable = Linker.depointerize(subparser.getSymbols(), subdata, subparser.offsets);
			const subcodeLength = subparser.getCodeLength();
			const subdataLength = subparser.getDataLength();
			let subtableLength = subparser.rawSymbols.length;

			// We can't have multiple .end labels! This is the only collision we account for;
			// other collisions will cause an exception, though it could be possible to issue
			// only a warning, in which case any collisions won't be added due to the behavior
			// of Object.assign.
			if (".end" in subtable) {
				delete subtable[".end"];
				subtableLength -= 3; // The .end entry is always 3 words long.
			}

			Linker.detectSymbolCollisions(this.combinedSymbols, subtable);

			// Step 7b: Note the difference between the original metadata section's length and the included binary's metadata section's length.
			const metaDifference = metaLength - subparser.getMetaLength(); // in bytes!

			// Step 7c: Replace all immediate/addrs with linker flag KNOWN_SYMBOLIC with their symbols.
			Linker.desymbolize(subcode, subtable, subparser.offsets);

			// Steps 7d–e
			for (const symbol in subtable) {
				const type = Linker.getSymbolType(subparser.offsets, subtable, symbol);

				if (type == "code") {
					// Step 7d: For each code symbol in the included symbol table,
					// increase its address by extraSymbolLength + extraCodeLength + metaDifference.
					subtable[symbol][1] = subtable[symbol][1].add(extraSymbolLength + extraCodeLength + metaDifference - 24);
				} else if (type == "data") {
					// Step 7e: For each data symbol in the included symbol table, increase
					// its address by extraSymbolLength + extraCodeLength + extraDataLength + metaDifference.
					subtable[symbol][1] = subtable[symbol][1].add(extraSymbolLength + extraCodeLength + extraDataLength + metaDifference - 24);

					if (type != "data" && symbol != ".end") {
						console.warn(chalk.yellow.bold(" !"), `Encountered a symbol other than .end of type ${chalk.bold(type)}: ${chalk.bold(symbol)}`);
					}
				}

				symbolTypes[symbol] = type;
			}

			// Steps 7f–7g
			for (const label in this.combinedSymbols) {
				const symbol = this.combinedSymbols[label];
				const type = symbolTypes[label];
				if (type == "code") {
					// Step 7f: For each code symbol in the global symbol table,
					// increase its address by the included symbol table's length.
					symbol[1] = symbol[1].add(subtableLength * 8);
				} else if (type == "data" || label == ".end") { // TODO: is `label == ".end"` correct?
					// Step 7g: For each data symbol in the global symbol table, increase its address
					// by the included data section's length + the included code section's length.
					symbol[1] = symbol[1].add(subtableLength * 8 + subcodeLength);
				}
			}

			// Step 7h: Add the symbol table's length to extraSymbolLength.
			extraSymbolLength += subtableLength * 8;

			// Step 7i: Add code.length to extraCodeLength.
			extraCodeLength += subcodeLength;

			// Step 7j: Add data.length to extraDataLength.
			extraDataLength += subdataLength;

			// Step 7k: Append the symbol table to the combined symbol table.
			this.combinedSymbols = Object.assign(subtable, this.combinedSymbols);

			// Step 7l: Append the code to the global code.
			this.combinedCode = [...this.combinedCode, ...subcode];

			// Step 7m: Append the data to the global data.
			this.combinedData = [...this.combinedData, ...subdata];
		}

		// Step 8: Readjust the .end entry in the symbol table.
		if (!(".end" in this.combinedSymbols)) {
			this.combinedSymbols[".end"] = [WASMC.encodeSymbol(".end"), Long.UZERO];
		}

		const end = 8 * (this.parser.rawMeta.length + WASMC.encodeSymbolTable(this.combinedSymbols).length
			+ this.combinedCode.length + this.combinedData.length);
		this.combinedSymbols[".end"][1] = Long.fromInt(end, true);
		const encodedCombinedSymbols = WASMC.encodeSymbolTable(this.combinedSymbols);
		const codeOffset = (encodedCombinedSymbols.length - symtabLength) * 8;
		
		// Step 9: Replace all symbols in the code with the new addresses.
		Linker.resymbolize(this.combinedCode, this.combinedSymbols);

		// Step 10: Update the offset section in the metadata.
		const meta = this.parser.rawMeta;
		meta[1] = meta[1].add(codeOffset); // Beginning of code
		meta[2] = meta[1].add(this.combinedCode.length * 8); // Beginning of data
		meta[3] = meta[2].add(this.combinedData.length * 8); // Beginning of heap

		// Step 11: Concatenate all the combined sections and write the result to the output file.
		const combined = [
			...this.parser.rawMeta,
			...encodedCombinedSymbols,
			...this.combinedCode,
			...this.combinedData
		];

		// Step 12:
		Linker.repointerize(this.combinedData, this.combinedSymbols, {
			$symtab: meta[0].toInt(),
			$code:   meta[1].toInt(),
			$data:   meta[2].toInt(),
			$end:    meta[3].toInt()
		}, combined);

		this.writeOutput(combined);
		this.printSuccess();
	}

	/**
	 * Replaces pointers inside all pointer variables of a symbol
	 * table with the encoded names of the symbols they point to.
	 * @param {SymbolTable} symtab  A symbol table.
	 * @param {Long[]}      data    An array of longs comprising a data section.
	 * @param {Object}      offsets An object of the basic offsets.
	 * @return {SymbolTable} A clone of the input symbol table with all pointers replaced.
	 */
	static depointerize(symtab, data, offsets) {
		const clone = _.cloneDeep(symtab);
		const {$data, $end} = offsets;

		for (const key in clone) {
			const [id, addr, type] = clone[key];
			const index = (addr.toNumber() - $data) / 8;
			if (type == SYMBOL_TYPES.KNOWN_POINTER) {
				const curValue = data[index];
				
				const matches = _.filter(clone, (v, k) => v[1].eq(curValue));
				if (!matches.length) {
					throw `Found 0 matches for ${curValue.toNumber()} from key "${key}".`;
				}

				// Replace the current data value, which currently contains the old pointer value,
				// with the ID of the symbol it points to.
				data[index] = Long.fromNumber(matches[0][0], true);
			}
		}

		return clone;
	}

	static repointerize(data, symtab, offsets, combined) {
		for (const key in symtab) {
			const [, addr, type] = symtab[key];
			if (type == SYMBOL_TYPES.KNOWN_POINTER || type == SYMBOL_TYPES.UNKNOWN_POINTER) {
				const index = addr.toInt() / 8;
				const ptr = Linker.findSymbolFromID(combined[index], symtab);
				// console.log(key, addr.toString(), combined[index].toString(16), "\n\n\n", symtab);
				if (symtab[ptr]) {
					combined[index] = symtab[ptr][1];
				} else {
					console.warn(`Couldn't find pointer for ${key}`);
				}
			}
		}
	}

	/**
	 * Collects all the symbols in a symbol table and returns
	 * an object mapping each symbol name to its type.
	 * @param  {Object} offsets An offsets object (as made by {@link module:wasm~Parser parsers}).
	 * @param  {SymbolTable} symbolTable A symbol table.
	 * @return {Object<string, SymbolType>} A map between symbol names and symbol types.
	 */
	static collectSymbolTypes(offsets, symbolTable) {
		return _.fromPairs(Object.keys(symbolTable).map(key => [key, Linker.getSymbolType(offsets, symbolTable, key)]));
	}

	/**
	 * Returns the type of a symbol (i.e., the name of the section it occurs in).
	 * @param  {Object} offsets An offsets object (as made by {@link module:wasm~Parser parsers}).
	 * @param  {SymbolTable} symbolTable A symbol table.
	 * @param  {string} symbol A symbol name.
	 * @return {SymbolType} The type of the symbol.
	 */
	static getSymbolType(offsets, symbolTable, symbol) {
		const addr = symbolTable[symbol][1].toInt();
		const {$code, $data, $end} = offsets;

		if ($code <= addr && addr < $data) {
			return "code";
		}

		if ($data <= addr && addr < $end) {
			return "data";
		}

		return "other";
	}

	/**
	 * Checks two symbol tables and throws an exception if the second contains any labels already defined in the first.
	 * @param {SymbolTable} tableOne The first symbol table.
	 * @param {SymbolTable} tableTwo The second symbol table.
	 */
	static detectSymbolCollisions(tableOne, tableTwo) {
		for (const key in tableTwo) {
			if (key != ".end" && key in tableOne) {
				throw `Encountered a symbol collision: "${key}"`;
			}
		}
	}

	/**
	 * Converts the imm/addr values of the I-/J-type instructions marked with the KNOWN_SYMBOL flag in a list of Longs to their symbol representations.
	 * @param {Long} longs An array of compiled code.
	 * @param {SymbolTable} symbolTable An object mapping a symbol name to a tuple of its ID and its address.
	 * @param {Object} offsets An an object of offsets.
	 */
	static desymbolize(longs, symbolTable, offsets) {
		for (let i = 0; i < longs.length; i++) {
			const parsedInstruction = Parser.parseInstruction(longs[i]);
			const {opcode, type, flags, rs, rd, link, conditions} = parsedInstruction;
			if (flags == FLAGS.KNOWN_SYMBOL) {
				if (type != "i" && type != "j") {
					throw `Found an instruction not of type I or J with \x1b[1mKNOWN_SYMBOL\x1b[22m set at \x1b[1m0x${i * 8 + offsets.$code}\x1b[22m.`;
				}

				const val = type == "i" ? parsedInstruction.imm : parsedInstruction.addr;
				const name = Linker.findSymbolFromAddress(val, symbolTable, offsets.$end);

				if (!name || !symbolTable[name]) {
					throw `desymbolize: Couldn't find a symbol corresponding to \x1b[0m\x1b[1m${val}\x1b[0m.`;
				}

				const id = symbolTable[name][0];
				if (type == "i") {
					longs[i] = Linker.assembler.iType(opcode, rs, rd, id, FLAGS.SYMBOL_ID, conditions);
				} else {
					longs[i] = Linker.assembler.jType(opcode, rs, id, link, FLAGS.SYMBOL_ID, conditions);
				}
			}
		}
	}

	/**
	 * Undoes desymbolization; converts the imm/addr values of the I-/J-type instructions marked with
	 * the SYMBOL_ID or UNKNOWN_SYMBOL flags in a list of Longs from symbol IDs to absolute addresses.
	 * @param longs An array of compiled code.
	 * @param symbolTable An object mapping a symbol name to a tuple of its ID and its address.
	 */
	static resymbolize(longs: Long[], symbolTable: SymbolTable) {
		for (let i = 0; i < longs.length; i++) {
			const parsedInstruction = Parser.parseInstruction(longs[i]);
			const {opcode, type, flags, rs, rd, imm, addr, link, conditions} = parsedInstruction;
			if (flags == FLAGS.SYMBOL_ID || flags == FLAGS.UNKNOWN_SYMBOL) {
				if (type != "i" && type != "j") {
					throw new Error(`Found an instruction not of type I or J with \x1b[1m` +
					                (flags == FLAGS.UNKNOWN_SYMBOL? "UNKNOWN_SYMBOL" : "SYMBOL_ID") + "\x1b[22m set at "
					                + `offset \x1b[1m0x${i * 8}\x1b[22m.`);
				}

				const val = type == "i" ? parsedInstruction.imm : parsedInstruction.addr;
				const name = Linker.findSymbolFromID(val, symbolTable);
				if (!name || !symbolTable[name]) {
					if (flags == FLAGS.UNKNOWN_SYMBOL) {
						// Unknown labels in included binaries are okay if they're resolved later.
						// For example, B could reference symbols defined in C without including C,
						// but if A includes B and C, then the symbols will be resolved in the compiled
						// output for A.
						continue;
					}

					throw new Error(`resymbolize: Couldn't find a symbol corresponding to \x1b[0m\x1b[1m0x` +
					                val.toString(16).padStart(16, "0") + "\x1b[0m.");
				}

				const addr = symbolTable[name][1];
				if (addr.high != 0) {
					console.warn("Truncating address of label", chalk.bold(name),
					             "to",   chalk.bold(`0x${addr.toString(16).padStart(16, "0")}`),
					             "from", chalk.bold(`0x${addr.low.toString(16).padStart(16, "0")}`) + ".");
				}

				longs[i] = type == "i"?
					Linker.assembler.iType(opcode, rs, rd, addr.toInt(),       FLAGS.KNOWN_SYMBOL, conditions, false) :
					Linker.assembler.jType(opcode, rs,     addr.toInt(), link, FLAGS.KNOWN_SYMBOL, conditions, false);
			}
		}
	}

	/**
	 * Finds a symbol name based on its ID.
	 * @param  {number} id A numeric ID.
	 * @param  {SymbolTable} symbolTable An object mapping a symbol name to a tuple of its ID and its address.
	 * @return {?string} A symbol name if one was found; `null` otherwise.
	 */
	static findSymbolFromID(id: number, symbolTable: SymbolTable): string | null {
		for (const name in symbolTable) {
			if (symbolTable[name][0] == id) {
				return name;
			}
		}

		return null;
	}

	/**
	 * Finds a symbol name based on its address.
	 * @param  addr        An address.
	 * @param  symbolTable An object mapping a symbol name to a tuple of its ID and its address.
	 * @param  endOffset   The address of the start of the heap.
	 * @return A symbol name if one was found; `null` otherwise.
	 */
	static findSymbolFromAddress(addr: number, symbolTable: SymbolTable, endOffset: number): string | null {
		for (const name in symbolTable) {
			if (symbolTable[name][1].eq(addr)) {
				return name;
			}
		}

		if (addr == endOffset) {
			return ".end";
		}

		return null;
	}

	/**
	 * Writes the output of the linking process to a file.
	 * @param  longs    The final linked output as an array of Longs.
	 * @param [outfile] A filename (`options.out` if not specified).
	 */
	writeOutput(longs: Long[], outfile: string = this.options.out) {
		fs.writeFileSync(outfile, WASMC.longs2strs(longs).join("\n"));
	}

	/**
	 * Prints a message to the console that indicates the linking was successful.
	 * @param infile  The input filename.
	 * @param outfile The output filename.
	 */
	printSuccess(infile: string = this.objectFilenames[0], outfile: string = this.options.out) {
		console.log(chalk.green.bold(" \u2714"), "Successfully linked", chalk.bold(path.relative(".", infile)),
		            "and saved the output to", chalk.bold(path.relative(".", outfile)) + ".");
	}

	warn(...args: any[]) {
		console.warn(...args);
	}

	log(...args: any[]) {
		if (this.options.debug) {
			console.log(...args);
		}
	}
}

module.exports = Linker;

if (require.main === module) {
	const options = minimist(process.argv.slice(2), {
		alias: {
			o: "out",
			d: "debug",
		},
		boolean: ["debug"],
		default: {
			debug: false
		}
	});

	if (options._.length == 0 || !options.out) {
		console.log("Usage: node wld.js [main.why] [compiled.why]... -o out");
		process.exit(0);
	}

	try {
		new Linker(options, options._.map(f => path.resolve(f)), options.out).link();
	} catch(e) {
		if (typeof e == "string") {
			console.error(chalk.red(e));
		} else {
			process.stdout.write("\x1b[31m");
			console.error(e);
			process.stdout.write("\x1b[0m");
		}

		process.exit(1);
	}
}
