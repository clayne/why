#!/usr/bin/env node
let fs = require("fs"),
	chalk = require("chalk"),
	minimist = require("minimist"),
	Long = require("long"),
	WASMC = require("./wasmc.js"),
	Parser = require("../wvm/parser.js"),
	_ = require("lodash");

const {FLAGS} = require("./constants.js");

/**
 * @module wasm
 */

require("../util.js");
require("string.prototype.padstart").shim();
require("string.prototype.padend").shim();

/**
 * Represents a `wld` instance.
 */
class Linker {
	constructor(options, objects, out) {
		this.options = options;
		this.objectFilenames = objects;
		this.outputFilename = out;

		this.parser = null;
		this.combinedSymbols = {};
	}

	link() {
		if (this.objectFilenames.length < 2) {
			console.error(chalk.yellow.bold(" ?"), `Multiple input files are needed.`);
			process.exit(1);
		}

		// Step 1
		this.parser = new Parser();
		this.parser.open(this.objectFilenames[0]);

		const {raw, parsed} = this.parser;

		// Step 2
		const metaLength = this.parser.getMetaLength();
		const codeLength = this.parser.getCodeLength();
		const dataLength = this.parser.getDataLength();
		const mainSymbols = this.parser.getSymbols();
		const symtabLength = this.parser.rawSymbols.length;
		this.combinedSymbols = _.cloneDeep(mainSymbols);

		// We need to keep track of symbol types separately because it becomes difficult to recompute them
		// after the symbol table has been expanded with new symbols from included binaries, as the boundaries
		// between code sections and data sections become murky.
		const symbolTypes = Linker.collectSymbolTypes(this.parser.offsets, this.combinedSymbols);

		// Step 3
		Linker.desymbolize(this.parser.rawCode, mainSymbols, this.parser.offsets);

		// Steps 4–6
		let extraSymbolLength = this.combinedSymbols.length;
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

			const {raw: subraw, parsed: subparsed} = subparser;
			const subcodeLength = subparser.getCodeLength();
			const subdataLength = subparser.getDataLength();

			const subtable = subparser.getSymbols();
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
			const metaDifference = metaLength - subparser.getMetaLength();

			// Step 7c: Replace all immediate/addrs with linker flag KNOWN_SYMBOLIC with their symbols.
			Linker.desymbolize(subparser.rawCode, subtable, subparser.offsets);

			for (const symbol of Object.keys(subtable)) {
				const type = Linker.getSymbolType(subparser.offsets, subtable, symbol);
				if (type == "code") {
					// Step 7d: For each code symbol in the included symbol table,
					// increase its address by extraSymbolLength + extraCodeLength + metaDifference.
					subtable[symbol][1] = subtable[symbol][1].add(extraSymbolLength + extraCodeLength + metaDifference);
				} else if (type == "data" || symbol == ".end") {
					// Step 7e: For each data symbol in the included symbol table, increase
					// its address by extraSymbolLength + extraCodeLength + extraDataLength + metaDifference.
					subtable[symbol][1] = subtable[symbol][1].add(extraSymbolLength + extraCodeLength + extraDataLength + metaDifference);
				} else {
					throw `Encountered a symbol other than .end of type "${type}": "${symbol}"`;
				}
			}

			for (const label of Object.keys(this.combinedSymbols)) {
				const symbol = this.combinedSymbols[label];
				const type = symbolTypes[label];
				if (type == "code") {
					// Step 7f: For each code symbol in the global symbol table, increase its address by the included symbol table's length.
					symbol[1] = symbol[1].add(subtableLength);
				} else if (type == "data" || symbol == ".end") {
					// Step 7g: For each data symbol in the global symbol table, increase its address
					// by the included data section's length + the included code section's length.
					symbol[1] = symbol[1].add(subtableLength + subcodeLength);
				}
			}

			// Step 7h: Add the symbol table’s length to extraSymbolLength.
			extraSymbolLength += subtableLength;

			// Step 7i: Add code.length to extraCodeLength.
			extraCodeLength += subcodeLength;

			// Step 7j: Add data.length to extraDataLength.
			extraDataLength += subdataLength;

			// Step 7k: Append the symbol table to the combined symbol table.
			console.log("Old:", Object.keys(this.combinedSymbols).length, this.combinedSymbols);
			this.combinedSymbols = Object.assign(subtable, this.combinedSymbols);
			console.log("New:", Object.keys(this.combinedSymbols).length, this.combinedSymbols);

			// Step 7l: Append the code to the global code.
			// Step 7m: Append the data to the global data.
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
		return _.fromPairs(Object.keys(symbolTable).map((key) => [key, Linker.getSymbolType(offsets, symbolTable, key)]));
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
	 * Converts the imm/addr values of the I-/J-type instructions in a list of Longs to their symbol representations
	 * @param {Long} longs An array of compiled code.
	 * @param {SymbolTable} symbolTable An object mapping a symbol name to a tuple of its ID and its address.
	 * @param {Object} offsets An an object of offsets.
	 */
	static desymbolize(longs, symbolTable, offsets) {
		for (let i = 0; i < longs.length; i++) {
			const parsedInstruction = Parser.parseInstruction(longs[i]);
			const {opcode, type, flags, rs, rd, imm, addr} = parsedInstruction;
			if (flags == FLAGS.KNOWN_SYMBOL) {
				if (type != "i" && type != "j") {
					throw `Found an instruction not of type I or J with \x1b[1mKNOWN_SYMBOL\x1b[22m set at \x1b[1m0x${i * 8 + offsets.$code}\x1b[22m.`;
				}

				const name = Linker.findSymbolFromAddress(type == "i" ? imm : addr, symbolTable, offsets.$end);
				if (!name || !symbolTable[name]) {
					throw `Couldn't find a symbol corresponding to \x1b[0m\x1b[1m${imm}\x1b[0m.`;
				}

				const id = symbolTable[name][0];
				// We disable warnings because the encoded IDs are somehow negative (even if I add a Math.abs() call in WASMC.encodeSymbol, strangely).
				if (type == "i") {
					longs[i] = WASMC.iType(opcode, rs, rd, id, FLAGS.SYMBOL_ID, false);
				} else {
					longs[i] = WASMC.jType(opcode, rs, id, FLAGS.SYMBOL_ID, false);
				}
			}
		}
	}

	/**
	 * Finds a symbol name based on its ID.
	 * @param  {number} id A numeric ID.
	 * @param  {SymbolTable} symbolTable An object mapping a symbol name to a tuple of its ID and its address.
	 * @return {?string} A symbol name if one was found; `null` otherwise.
	 */
	static findSymbolFromID(id, symbolTable) {
		for (let name of Object.keys(symbolTable)) {
			if (symbolTable[name][0] == id) {
				return name;
			}
		}

		return null;
	}

	/**
	 * Finds a symbol name based on its address.
	 * @param  {number} addr An address.
	 * @param  {SymbolTable} symbolTable An object mapping a symbol name to a tuple of its ID and its address.
	 * @param  {number} endOffset The address of the start of the heap.
	 * @return {?string} A symbol name if one was found; `null` otherwise.
	 */
	static findSymbolFromAddress(addr, symbolTable, endOffset) {
		for (let name of Object.keys(symbolTable)) {
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
	 * @param {Long[]} longs The final linked output as an array of Longs.
	 * @param {string} [outfile] A filename (`options.out` if not specified).
	 */
	writeOutput(longs, outfile=this.options.out) {
		fs.writeFileSync(outfile, WASMC.longs2strs(longs).join("\n"));
	}

	/**
	 * Prints a message to the console that indicates the linking was successful.
	 * @param {string} infile The input filename.
	 * @param {string} outfile The output filename.
	 */
	printSuccess(infile, outfile) {
		console.log(chalk.green.bold(" \u2714"), `Successfully linked ${chalk.bold(infile)} and saved the output to ${chalk.bold(outfile)}.`);
	}

	warn(...args) {
		console.warn(...args);
	}

	log(...args) {
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
		console.log("Usage: node wld.js ...[compiled.why] -o out");
		process.exit(0);
	}

	try {
		new Linker(options, options._, options.out).link();
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
