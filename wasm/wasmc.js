#!/usr/local/bin/node
let fs = require("fs"),
	nearley = require("nearley"),
	nearleyg = require("nearley/lib/nearley-language-bootstrapped.js"),
	nearleyc = require("nearley/lib/compile.js"),
	gen = require("nearley/lib/generate.js"),
	chalk = require("chalk"),
	getline = require("get-line-from-pos"),
	minimist = require("minimist"),
	prettyjson = require("prettyjson"),
	Long = require("long"),
	_ = require("lodash");

require("string.prototype.padstart").shim();
require("string.prototype.padend").shim();

const { EXCEPTIONS, R_TYPES, I_TYPES, J_TYPES, OPS, FUNCTS, REGISTER_OFFSETS } = require("./constants.js");

class Wasmc {
	static die(...a) { console.error(...a); process.exit(1) };

	// Converts an array of 8 characters into a Long.
	static chunk2long(chunk) {
		return Long.fromString(chunk.map((c) => c.charCodeAt(0).toString(16).padStart(2, "0")).join(""), true, 16);
	};

	// Adds nulls to the end of the string to lengthen it to a multiple of 8.
	static nullpad(str) {
		return str.padEnd(Math.ceil(str.length / 8) * 8, "\0");
	};

	// Given any string, str2longs nullpads and chunks it and returns an array of Longs.
	static str2longs(str) {
		return _.chunk(Wasmc.nullpad(str).split(""), 8).map(Wasmc.chunk2long);
	};

	// Given an array of longs, returns an array containing the 16-length zero-padded hex representations.
	static longs2strs(longs) {
		return longs.map((l) => l instanceof Long? l.toString(16).padStart(16, "0") : "x".repeat(16));
	};

	// If the input is an array (expected format: ["register", ...]), then the output is the number corresponding to that array.
	// Otherwise, if the input is something other than an array, then the output is same as the input.
	static convertRegister(x) { // [, type, n]
		return x instanceof Array? (x.length == 0? 0 : REGISTER_OFFSETS[x[1]] + x[2]) : x;
	};
	
	constructor(opt, filename) {
		this.opt = opt;
		this.filename = filename;
		this.offsets = { };
		this.parsed = { };
		this.meta = [];
		this.data = [];
		this.offsets = { };
		this.code = [];
		this.expanded = [];
	};

	get binary() { return this.opt.binary };
	get debug() { return this.opt.debug };

	parse() {
		let grammar;
		try {
			grammar = require("./wasm.js");
		} catch (e) {
			console.error("Couldn't read wasm.js.");
			if (this.opt.debug) {
				console.error(e);
			};

			process.exit(1);
		};

		const parser = new nearley.Parser(grammar.ParserRules, grammar.ParserStart);
		const source = `\n${fs.readFileSync(this.filename, {encoding: "utf8"})}\n`;

		let trees;
		try {
			trees = parser.feed(source).results;
		} catch (e) {
			console.error(chalk.red("Syntax error"), "at", chalk.white(`${getline(source, e.offset) - 1}:${e.offset - source.split(/\n/).slice(0, getline(source, e.offset) - 1).join("\n").length - 1}`) + ":");
			if (this.opt.dev) {
				console.log(e.message.replace(/\(@(\d+):([^)]+)\)/g, ($0, $1, $2) => { const _line = getline(source, e.offset) - 1; return `(@${_line}:${e.offset - source.split(/\n/).slice(0, _line).join("\n").length - 1 + $2})` }));
			};

			process.exit(1);
		};

		if (trees.length > 1) {
			trees.forEach((tree) => console.log(JSON.stringify(trees[tree], null, 4)));
			console.error(chalk.red.italic(`\nAmbiguous grammar (${trees.length}).\n`));
			process.exit(1);
		} else if (trees.length === 0) {
			console.warn(chalk.red.italic("Nothing parsed."));
			process.exit(1);
		};

		this.parsed = trees[0];

		if (typeof this.parsed != "object") {
			Wasmc.die("Parser output isn't an object.");
		};

		if (typeof this.parsed.metadata == "undefined") {
			this.parsed.metadata = { };
		};

		if (typeof this.parsed.data == "undefined") {
			this.parsed.data = { };
		};

		if (typeof this.parsed.code == "undefined") {
			this.parsed.code = { };
		};
	};

	compile() {
		this.parse();
		this.processMetadata();
		this.processData();

		let handlers = [...Array(EXCEPTIONS.length)].map(() => Long.UZERO); // just a placeholder for now.

		// this.processLabels();
		this.expandCode();
		this.processCode();

		const out = this.meta.concat(this.handlers).concat(this.data).concat(this.code);

		console.log({
			meta: Wasmc.longs2strs(this.meta),
			handlers: Wasmc.longs2strs(handlers),
			data: Wasmc.longs2strs(this.data),
			code: Wasmc.longs2strs(this.code),
			out: Wasmc.longs2strs(out),
			offsets: this.offsets
		});
	};

	processMetadata(parsed) {
		let [name, version, author] = [this.parsed.meta.name || "?", this.parsed.meta.version || "?", this.parsed.meta.author || "?"];

		const orcid = typeof this.parsed.meta.orcid == "undefined"? "0000000000000000" : this.parsed.meta.orcid.replace(/\D/g, "");
		if (orcid.length != 16) {
			Wasmc.die("Error: invalid ORCID.");
		};

		// Convert the ORCID into two Longs and stash them in the correct positions in meta.
		[this.meta[4], this.meta[5]] = [orcid.substr(0, 8), orcid.substr(8)].map((half) => Wasmc.chunk2long(half.split("")));

		// Append the name-version-author string.
		this.meta = this.meta.concat(Wasmc.str2longs(`${name}\0${version}\0${author}\0`));
		
		// The beginning of the handler pointer section comes right after the end of the meta section.
		this.meta[0] = Long.fromInt(this.meta.length, true);

		// The handlers section is exactly as large as the set of exceptions; the data section begins
		// at the sum of the lengths of the meta section and the handlers section.
		this.meta[1] = Long.fromInt(this.meta.length + EXCEPTIONS.length);

		this.log({ meta: this.meta, version, author });
	};

	processData() {
		let offset = this.meta[1].toInt();
		_(this.parsed.data).forEach(([type, value], key) => {
			let pieces;
			if (type.match(/^(in|floa)t$/)) {
				pieces = [Long.fromValue(value)];
			} else if (type == "string") {
				pieces = Wasmc.str2longs(value);
			} else {
				Wasmc.die(`Error: unknown data type "${type}" for "${key}".`);
			};

			this.offsets[key] = offset;
			this.data = this.data.concat(pieces);
			offset += pieces.length;
		});

		this.meta[2] = Long.fromInt(offset);
	};

	processLabels() {
		// this.parsed.code.forEach((item, i) => {
		// 	if (item[0]) {
		// 		this.offsets[item[0]] = this.meta[2].toInt() + i;
		// 		this.log(`Label ${item[0]} at offset ${this.meta[2].toInt() + i}`);
		// 	};
		// });
	};

	processCode() {
		this.expanded.forEach((item, i) => {
			this.addCode(item);
		});
	};

	expandCode() {
		// In the first pass, we expand pseudoinstructions into their constituent parts. Some instructions will need to be
		// gone over once again after labels have been sorted out so we can replace variable references with addresses.
		this.parsed.code.forEach((item, i) => {
			let [label, op, ...args] = item;
			if (label) {
				this.offsets[label] = this.meta[2].toInt() + this.expanded.length;
			};

			if (op == "jeq") {
				// console.log({item});
				// [label, "seq", rt, rs, rd]
				// args[0]: rt
				// args[1]: rs
				// args[2]: rd

				// jc: target, rs [. jc rs target]
				// m0 -> rs
				// rd -> target

				// R-types: [..., rt, rs, rd]
				// I-types: [..., rs, rd, imm]

				this.expanded.push([label, "seq", ...args.slice(0, 2), _M[0]]);
				if (args[2][0] == "value") {
					// Set $m1 to the immediate value and then conditionally jump to $m1.
					// We set $m1 to the immediate value with ori on $0.
					this.expanded.push([null, "ori", _0, _M[1], args[2][1]]);
					this.expanded.push([null, "jrc", _0, _M[0], _M[1]]);
				} else if (args[2][0] == "register") {
					// We're already given a register, so we 
					// don't have to do anything with $m1.
					this.expanded.push([null, "jrc", _0, _M[0], args[2]]);
				} else if (args[2][0] == "label") {
					// Load the value at the given variable into $m1
					// and then conditionally jump to $m1.
					// this.expanded.push([null, "ori", _0, _M[1], args[2][1]
					// this.expanded.push([null, 
				};
			} else if (R_TYPES.includes(OPS[op]) && _.some(args, (reg) => reg[0] == "label")) {
				// console.log(chalk.bold.italic.green("PASSED"), "for", item);
				let [rt, rs, rd] = args;
				let _label = label;
				let getLabel = () => [_label, _label = null][0]

				let [lt, ls, ld] = [rt, rs, rd].map((reg) => reg[0] == "label");
				[rt, rs].forEach((reg, i) => {
					if (reg[0] == "label") {
						// Whoops, this register isn't actually a register
						this.expanded.push([getLabel(), "li", _0, _M[i], reg]);
					};
				});

				this.expanded.push([getLabel(), op, ...[rt, rs, rd].map((reg, i) => [lt, ls, ld][i]? _M[i] : reg)]);

				if (ld) {
					this.expanded.push([getLabel(), "si", _M[2], _0, rd]);
				};


				// args.filter((arg) => args instanceof Array && args.length == 2 && args[0] == "label")
				// let label_args;
				// if (label_args = args.filter((arg) => args instanceof Array && args.length == 2 && args[0] == "label")) {

				// } else {
					// this.expanded.push([[]].concat(item));
				// };
			} else {
				// console.log(chalk.bold.italic.red("FAILED"), `for (${op})`, item);
				this.expanded.push(item);
			};
		});

		// In the second pass, we replace label references with the corresponding
		// addresses now that we know the address of all the labels.
		this.expanded.forEach((item, i) => {
			// First off, now that we've recorded all the label positions,
			// we can remove the label tags.
			item.shift();

			// Look at everything past the operation name (the arguments).
			item.slice(1).forEach((arg, j) => {
				// If the argument is a label reference,
				if (arg instanceof Array && arg.length == 2 && arg[0] == "label") {
					// replace it with an address from the offsets map. 
					item[2 + j] = this.offsets[arg[1]];
				};
			});
		});

		console.log(chalk.bold.yellow("<expanded>\n"), this.expanded, chalk.bold.yellow("\n</expanded>"));// JSON.stringify(this.expanded, null, 4)});
	};

	addCode([op, ...args]) {
		// if (label) {
		// 	this.offsets[label] = this.meta[2].toInt() + this.code.length;
		// 	console.log(`Label ${item[0]} at offset ${this.meta[2].toInt() + i}`);
		// };

		// if (op.match(/^(add|mult|n?and|(x?n|x)?or|not|mf(hi|lo)|s(le?|eq|ub)|[cls]|jr)$/)) {
		// } else if (op.match(/^(add|sub|mult|n?and|(x?n|x)?or|lu)i$/)) {
		if (R_TYPES.includes(OPS[op])) {
			this.code.push(this.rType(OPS[op], ...args.map(Wasmc.convertRegister), 0, FUNCTS[op]));
		} else if (I_TYPES.includes(OPS[op])) {
			this.code.push(this.iType(OPS[op], ...args.map(this.convertValue, this)));
		} else if (op.match(/^jc?$/)) {
			this.code.push(this.jType(OPS[op], ...args.map(this.convertValue, this)));
		} else if (op == "nop") {
			this.code.push(Long.UZERO);
		} else if (op == "jeq") {
/*
jeq rd, rs, rt
$rs == $rt? :$rd
If the value in rs is equal to the value in rt, jumps to the address stored in rd (or to the address of var). (Modifies m0.)

Translation:
seq m0, rs, rt
jc rd, m0


`seq rd, rs, rt`  
[op, rt, rs, rd]

-> [seq, rt, rs, m0]

> `seq rd, rs, rt`  
> `$rs == $rt -> $rd`  
> `000000001110` `ttttttt` `sssssss` `ddddddd` `000` `0000000000000000` `000000000010`

[jeq, rt, rs, rd]
*/
			this.addCode([null, "seq", ...[args[0], args[1], [, "m", 0]].map(Wasmc.convertRegister)]);
			if (args[2][0] == "register") {
				// > `jc target, rs`  
				// [jc, rs, addr]
				// this.addCode(["jc", args[2]]);
			} else {

			};
		} else {
			console.log(`Unknown instruction ${chalk.bold.red(op)}.`, [label, op, ...args]);
			this.code.push(Long.fromInt(0xdead, true));
		};
	};

	// If x is an array accepted by convertRegister, the output is the index of that array.
	// If x is a string, then the label map is checked and the address for the label x is returned.
	// If x is a number, return it.
	convertValue(x) {
		if (x instanceof Array || typeof x == "number") {
			return Wasmc.convertRegister(x);
		};

		if (typeof x == "string") {
			if (typeof this.offsets[x] == "undefined") {
				throw `Undefined label: ${x}`;
			};

			return this.offsets[x];
		};

		if (typeof x == "number") {
			return x;
		};

		throw `Unrecognized value: ${x}`;
	};

	rType(opcode, rt, rs, rd, shift, func) {
		if (!R_TYPES.includes(opcode)) throw `opcode ${opcode} isn't a valid r-type`;
		if (rt < 0 || 127 < rt) throw `rt (${rt}) not within the valid range (0–127)`;
		if (rs < 0 || 127 < rs) throw `rs (${rs}) not within the valid range (0–127)`;
		if (rd < 0 || 127 < rd) throw `rd (${rd}) not within the valid range (0–127)`;
		if (shift < 0 || 65535 < shift) throw `shift (${shift}) not within the valid range (0–65535)`;
		if (func < 0 || 4095 < func) throw `func (${func}) not within the valid range (0–4095)`;

		let lower = func | (shift << 12) | ((rd & 1) << 31);
		let upper = (rd >> 1) | (rs << 6) | (rt << 13) | (opcode << 20);
		let long = Long.fromBits(lower, upper, true);

		this.log(`Lower: ${lower.toString(2).padStart(32, "_")} (${lower.toString(2).length})`);
		this.log(`Upper: ${upper.toString(2).padStart(32, "_")} (${upper.toString(2).length})`);
		this.log(`Long: ${long.toString(16)}, ${long.toString(2)} <--`, long);

		return long;
	};

	iType(opcode, rs, rd, imm) {
		if (!I_TYPES.includes(opcode)) throw `opcode ${opcode} isn't a valid i-type`;
		if (rs < 0 || 127 < rs) throw `rs (${rs}) not within the valid range (0–127)`;
		if (rd < 0 || 127 < rd) throw `rd (${rd}) not within the valid range (0–127)`;
		if (imm < 0 || 4294967295 < imm) throw `imm (${imm}) not within the valid range (-2147483648–2147483647)`;

		let lower = imm;
		let upper = rd | (rs << 7) | (opcode << 20);
		let long = Long.fromBits(lower, upper, true);

		this.log(`Lower: ${lower.toString(2).padStart(32, "_")} (${lower.toString(2).length})`);
		this.log(`Upper: ${upper.toString(2).padStart(32, "_")} (${upper.toString(2).length})`);
		this.log(`Long: ${long.toString(16)}, ${long.toString(2)} <--`, long);

		return long;
	};

	jType(opcode, rs, addr) {
		if (!J_TYPES.includes(opcode)) throw `opcode ${opcode} isn't a valid j-type`;
		if (rs < 0 || 127 < rs) throw `rs (${rs}) not within the valid range (0–127)`;
		if (addr < 0 || 4294967295 < addr) throw `addr (${addr}) not within the valid range (0–4294967295)`;

		let lower = addr;
		let upper = (rs << 13) | (opcode << 20);
		let long = Long.fromBits(lower, upper, true);

		this.log(`Lower: ${lower.toString(2).padStart(32, "_")} (${lower.toString(2).length})`);
		this.log(`Upper: ${upper.toString(2).padStart(32, "_")} (${upper.toString(2).length})`);
		this.log(`Long: ${long.toString(16)}, ${long.toString(2)} <--`, long);

		return long;
	};

	log(...args) {
		if (this.debug) {
			console.log(...args);
		};

		return !!this.debug;
	};
};

exports.Wasmc = Wasmc;
const _K = _.range(0, 17).map((n) => ["register", "k", n]);
const _M = _.range(0, 16).map((n) => ["register", "m", n]);
const _E = _.range(0,  6).map((n) => ["register", "e", n]);
const _S = ["register", "stack", 0];
const _0 = ["register", "zero",  0];


if (require.main === module) {
	const opt = minimist(process.argv.slice(2), {
		alias: { b: "binary", d: "debug" },
		boolean: ["binary", "debug"],
		default: { binary: false, debug: false }
	}), filename = opt._[0];

	if (!filename) {
		return console.log("Usage: node wasmc.js [filename]");
	};

	new exports.Wasmc(opt, filename).compile();
};
