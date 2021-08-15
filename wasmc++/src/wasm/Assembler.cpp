#include <cctype>
#include <iostream>

#include "lib/picosha2.h"
#include "parser/Lexer.h"
#include "parser/StringSet.h"
#include "wasm/Assembler.h"
#include "wasm/Nodes.h"
#include "wasm/Registers.h"
#include "wasm/Why.h"

namespace Wasmc {
	Assembler::Assembler(Parser &parser_): parser(parser_) {}

	void Assembler::assemble() {
		meta.resize(5, 0);
		validateSectionCounts();
		findAllLabels();
		symbolTable = createSymbolTable(allLabels, true);
		processMetadata();
		processData();

		auto expanded = expandCode();
		metaOffsetData() = metaOffsetCode() + expanded.size() * 8;
		metaOffsetDebug() = metaOffsetData() + dataLength;

		debugData = createDebugData(debugNode, expanded);
		offsets[StringSet::intern(".end")] = metaOffsetEnd() = metaOffsetDebug() + debugData.size() * 8;

		setDataOffsets();
		reprocessData();
		processCode(expandLabels(expanded));
	}

	Long Assembler::compileInstruction(const WASMInstructionNode &node) {
		return 0;
	}

	void Assembler::addCode(const WASMInstructionNode &node) {
		code.push_back(compileInstruction(node));
	}

	Statements & Assembler::expandLabels(Statements &statements) {
		// In the second pass, we replace label references with the corresponding
		// addresses now that we know the address of all the labels.
		for (auto &statement: statements) {
			statement->labels.clear();
			if (auto *has_immediate = dynamic_cast<HasImmediate *>(statement.get())) {
				if (std::holds_alternative<const std::string *>(has_immediate->imm)) {
					const std::string *label = std::get<const std::string *>(has_immediate->imm);
					if (offsets.count(label) == 0) {
						auto encoded = encodeSymbol(label);
						has_immediate->imm = static_cast<int>(encoded);
						statement->flags = ConstantFlags::UnknownSymbol;
						unknownSymbols.insert(label);
					} else {
						const Long offset = offsets.at(label);
						if (INT_MAX < offset)
							warn() << "Offset for label " << *label << " exceeds INT_MAX\n";
						has_immediate->imm = static_cast<int>(offset);
						statement->flags = ConstantFlags::KnownSymbol;
					}
				}
			}
		}

		return statements;
	}

	void Assembler::processCode(const Statements &statements) {
		for (const auto &statement: statements)
			addCode(*statement);
	}

	void Assembler::reprocessData() {
		for (const auto &[key, ref]: dataVariables)
			data.at(dataOffsets.at(key) / 8) = offsets.at(ref);
	}

	void Assembler::setDataOffsets() {
		for (const auto &[name, offset]: dataOffsets)
			offsets[name] = offset + metaOffsetData();
	}

	void Assembler::validateSectionCounts() {
		bool meta_found = false, include_found = false, data_found = false, debug_found = false, code_found = false;
		for (ASTNode *node: *parser.root)
			switch (node->symbol) {
				case WASMTOK_META_HEADER:
					if (meta_found)
						throw std::runtime_error("Multiple meta sections detected");
					meta_found = true;
					metaNode = node;
					break;
				case WASMTOK_INCLUDE_HEADER:
					if (include_found)
						throw std::runtime_error("Multiple include sections detected");
					include_found = true;
					includeNode = node;
					break;
				case WASMTOK_DATA_HEADER:
					if (data_found)
						throw std::runtime_error("Multiple data sections detected");
					data_found = true;
					dataNode = node;
					break;
				case WASMTOK_DEBUG_HEADER:
					if (debug_found)
						throw std::runtime_error("Multiple debug sections detected");
					debug_found = true;
					debugNode = node;
					break;
				case WASMTOK_CODE_HEADER:
					if (code_found)
						throw std::runtime_error("Multiple code sections detected");
					code_found = true;
					codeNode = node;
					break;
				default:
					throw std::runtime_error("Unexpected symbol found at root level: "
						+ std::string(parser.getName(node->symbol)));
			}
	}

	void Assembler::findAllLabels() {
		allLabels.clear();

		if (dataNode) {
			for (const ASTNode *node: *dataNode) {
				if (node->symbol != WASMTOK_IDENT)
					throw std::runtime_error("Unexpected symbol found in data section at "
						+ std::string(node->location) + ": " + std::string(parser.getName(node->symbol)));
				allLabels.insert(node->lexerInfo);
			}
		}

		if (codeNode) {
			for (const ASTNode *node: *codeNode) {
				const auto *instruction = dynamic_cast<const WASMInstructionNode *>(node);
				if (!instruction)
					throw std::runtime_error("Unexpected symbol found in code section at "
						+ std::string(node->location) + ": " + std::string(parser.getName(node->symbol)));
				for (const std::string *label: instruction->labels)
					allLabels.insert(label);
			}
		}
	}

	std::vector<Long> Assembler::createSymbolTable(std::unordered_set<const std::string *> labels, bool skeleton) {
		labels.insert(StringSet::intern(".end"));
		std::vector<Long> out;

		for (const std::string *label: labels) {
			const size_t length = Util::updiv(label->size(), 8ul);
			if (0xffff < length)
				throw std::runtime_error("Symbol length too long: " + std::to_string(length));
			SymbolType type = SymbolType::Unknown;
			if (!skeleton && dataVariables.count(label)) {
				const std::string *ptr = dataVariables.at(label);
				type = offsets.count(ptr)? SymbolType::KnownPointer : SymbolType::UnknownPointer;
				if (!offsets.count(ptr) && !unknownSymbols.count(ptr)) {
					const size_t index = (offsets.at(label) - metaOffsetData()) / 8;
					data.resize(index, 0);
					data[index] = encodeSymbol(ptr);
				}
			}

			out.push_back(length | (static_cast<int>(type) << 16) | (static_cast<uint64_t>(encodeSymbol(label)) << 32));
			out.push_back(skeleton? 0 : offsets.at(label));
			for (const Long piece: getLongs(*label))
				out.push_back(piece);
		}

		return out;
	}

	uint32_t Assembler::encodeSymbol(const std::string *name) {
		std::vector<uint8_t> hash_vec(picosha2::k_digest_size);
		picosha2::hash256(name->begin(), name->end(), hash_vec.begin(), hash_vec.end());
		// TODO: verify
		const uint32_t hash = hash_vec[4] | (hash_vec[5] << 8) | (hash_vec[6] << 16) | (hash_vec[7] << 24);
		if (hashes.count(hash) != 0 && hashes.at(hash) != name)
			throw std::runtime_error("\"" + *name + "\" and \"" + *hashes.at(hash) + "\" have the same hash!");
		hashes.emplace(hash, name);
		return hash;
	}

	void Assembler::processMetadata() {
		std::string orcid = "0000000000000000", name = "?", version = "?", author = "?";

		if (metaNode)
			for (const ASTNode *node: *metaNode)
				switch (node->symbol) {
					case WASMTOK_NAME:
						name = node->front()->unquote();
						break;
					case WASMTOK_AUTHOR:
						author = node->front()->unquote();
						break;
					case WASMTOK_VERSION:
						version = node->front()->unquote();
						break;
					case WASMTOK_ORCID:
						orcid = node->front()->unquote();
						break;
					default:
						throw std::runtime_error("Unexpected symbol found in meta section at "
							+ std::string(node->location) + ": " + std::string(parser.getName(node->symbol)));
				}

		if (orcid.find_first_not_of("0123456789") != std::string::npos) {
			std::string new_orcid;
			new_orcid.reserve(16);
			for (const char ch: orcid)
				if (isdigit(ch))
					new_orcid.push_back(ch);
			orcid.swap(new_orcid);
		}

		if (orcid.size() != 16)
			throw std::runtime_error("Invalid ORCID length");

		const auto longs = getLongs(orcid);
		if (longs.size() != 2)
			throw std::runtime_error("ORCID longs count expected to be 2, not " + std::to_string(longs.size()));
		meta = {0, 0, 0, 0, 0, longs[0], longs[1]};

		std::string nva = name;
		nva.push_back('\0');
		nva.insert(nva.end(), version.begin(), version.end());
		nva.push_back('\0');
		nva.insert(nva.end(), author.begin(), author.end());
		nva.push_back('\0');

		for (const Long piece: getLongs(nva))
			meta.push_back(piece);

		metaOffsetSymbols() = meta.size() * 8;
		metaOffsetCode() = metaOffsetSymbols() + symbolTable.size() * 8;
	}

	void Assembler::processData() {
		if (!dataNode)
			return;

		data.clear();
		dataOffsets.clear();
		dataLength = 0;

		for (ASTNode *node: *dataNode) {
			const std::string *ident = node->lexerInfo;
			if (ident->front() != '%') {
				if (verbose)
					std::cerr << "Assigning " << dataLength << " to " << *ident << "\n";
				dataOffsets.emplace(ident, dataLength);
			}

			const auto pieces = convertDataPieces(node);
			for (const Long piece: pieces)
				data.push_back(piece);
			dataLength += pieces.size() * 8;
		}
	}

	std::vector<Long> Assembler::convertDataPieces(const ASTNode *node) {
		const ASTNode *child = node->front();
		static_assert(sizeof(Long) == sizeof(double));
		switch (child->symbol) {
			case WASMTOK_FLOAT: {
				double parsed = Util::parseDouble(child->lexerInfo);
				return {*reinterpret_cast<Long *>(&parsed)};
			}
			case WASMTOK_NUMBER:
				return {static_cast<Long>(child->atoi())};
			case WASMTOK_STRING: {
				std::string str = child->unquote();
				str.push_back('\0');
				return getLongs(str);
			}
			case WASMTOK_LPAR:
				return getLongs(std::string(child->front()->atoi(), '\0'));
			case WASMTOK_AND:
				dataVariables[node->lexerInfo] = child->front()->lexerInfo;
				return {0};
			default:
				throw std::runtime_error("Unexpected symbol found in data section at " + std::string(child->location)
					+ ": " + std::string(parser.getName(child->symbol)));
		}
	}

	Statements Assembler::expandCode() {
		// Known bug: locations for statement nodes are inaccurate.

		if (!codeNode)
			return {};

		Statements expanded;
		expanded.reserve(codeNode->size());

		for (const ASTNode *node: *codeNode) {
			const auto *instruction = dynamic_cast<const WASMInstructionNode *>(node);
			if (!instruction) {
				node->debug();
				throw std::runtime_error("Unexpected symbol found in code section at " + std::string(node->location)
					+ ": " + std::string(parser.getName(node->symbol)));
			}

			for (const std::string *label: instruction->labels) {
				if (offsets.count(label) != 0)
					throw std::runtime_error("Label " + *label + " redefined at " + std::string(node->location));
				offsets[label] = metaOffsetCode() + expanded.size() * 8;
				if (verbose)
					std::cerr << "Assigning " << offsets[label] << " to " << *label << " based on an expanded length "
					             "equal to " << expanded.size() << "\n";
			}

			switch (instruction->nodeType()) {
				case WASMNodeType::Call:
					addCall(expanded, instruction);
					break;

				case WASMNodeType::PseudoPrint:
					addPseudoPrint(expanded, instruction);
					break;

				case WASMNodeType::StringPrint:
					addStringPrint(expanded, instruction);
					break;

				case WASMNodeType::Mv:
					addMove(expanded, instruction);
					break;

				case WASMNodeType::Jeq:
					addJeq(expanded, instruction);

				default:
					expanded.emplace_back(instruction->copy());
					break;
			}
		}

		return expanded;
	}

	void Assembler::addJeq(Statements &expanded, const WASMInstructionNode *instruction) {
		const auto *jeq = dynamic_cast<const WASMJeqNode *>(instruction);
		const std::string *m7 = registerArray[Why::assemblerOffset + 7];
		const int bang = instruction->bang;
		if (std::holds_alternative<Register>(jeq->addr)) {
			// Address is a register
			if (std::holds_alternative<Register>(jeq->rt)) {
				// RHS is a register
				// rs == rt -> $m7
				expanded.emplace_back(makeSeq(jeq->rs, std::get<Register>(jeq->rt), m7, bang));
			} else {
				addJeqImmediateRHS(expanded, jeq, m7);
				// rs == $m7 -> $m7
				expanded.emplace_back(makeSeq(jeq->rs, m7, m7, bang));
			}
			// : rd if $m7
			expanded.emplace_back(new WASMJrcNode(jeq->link, m7, std::get<Register>(jeq->addr)));
		} else if (std::holds_alternative<Register>(jeq->rt)) {
			// Address is an immediate, RHS is a register
			// rs == rt -> $m7
			expanded.emplace_back(makeSeq(jeq->rs, std::get<Register>(jeq->rt), m7, bang));
			// : addr if $m7
			expanded.emplace_back((new WASMJcNode(std::get<Immediate>(jeq->addr), jeq->link, m7))->setBang(bang));
		} else {
			// Address is an immediate, RHS is an immediate
			addJeqImmediateRHS(expanded, jeq, m7);
			// : addr if $m7
			expanded.emplace_back((new WASMJcNode(std::get<Immediate>(jeq->addr), jeq->link, m7))->setBang(bang));
		}
	}

	void Assembler::addJeqImmediateRHS(Statements &expanded, const WASMJeqNode *jeq, const std::string *m7) {
		const Immediate &rhs = std::get<Immediate>(jeq->rt);
		if (std::holds_alternative<const std::string *>(rhs)) {
			// RHS is a label
			// [label] -> $m7
			expanded.emplace_back(new WASMLiNode(rhs, m7, false));
		} else if (std::holds_alternative<int>(rhs)) {
			// RHS is a number
			// imm -> $m7
			expanded.emplace_back(new WASMSetNode(rhs, m7));
		} else {
			jeq->debug();
			throw std::runtime_error("Invalid right hand side in jeq instruction");
		}
		expanded.back()->setBang(jeq->bang);
	}

	void Assembler::addMove(Statements &expanded, const WASMInstructionNode *instruction) {
		const auto *move = dynamic_cast<const WASMMvNode *>(instruction);
		expanded.emplace_back((new RNode(move->rs, StringSet::intern("|"), registerArray[Why::zeroOffset], move->rd,
			WASMTOK_OR, false))->setBang(instruction->bang));
	}

	void Assembler::addPseudoPrint(Statements &expanded, const WASMInstructionNode *instruction) {
		const auto *print = dynamic_cast<const WASMPseudoPrintNode *>(instruction);
		if (std::holds_alternative<char>(print->imm)) {
			const std::string *m7 = registerArray[Why::assemblerOffset + 7];
			expanded.emplace_back((new WASMSetNode(print->imm, m7))->setBang(instruction->bang));
			expanded.emplace_back((new WASMPrintNode(m7, PrintType::Char))->setBang(instruction->bang));
		} else
			throw std::runtime_error("Invalid WASMPseudoPrintNode immediate type: expected char");
	}

	void Assembler::addStringPrint(Statements &expanded, const WASMInstructionNode *instruction) {
		const auto *print = dynamic_cast<const WASMStringPrintNode *>(instruction);
		const std::string *m7 = registerArray[Why::assemblerOffset + 7];
		const std::string &str = *print->string;
		if (str.empty())
			return;
		char last_char = str.front() - 1;
		bool first = true;
		for (char ch: str) {
			if (ch != last_char) {
				auto *set = (new WASMSetNode(ch, m7))->setBang(print->bang);
				if (first)
					set->labels = print->labels;
				first = false;
				expanded.emplace_back(set);
				last_char = ch;
			}

			expanded.emplace_back((new WASMPrintNode(m7, PrintType::Char))->setBang(print->bang));
		}
	}

	void Assembler::addCall(Statements &expanded, const WASMInstructionNode *instruction) {
		const auto *call = dynamic_cast<const WASMCallNode *>(instruction);
		const Args &args = call->args;

		if (Why::argumentCount < args.size())
			throw std::runtime_error("Too many arguments given in subroutine call (given "
				+ std::to_string(args.size()) + ", maximum is " + std::to_string(Why::argumentCount) + ")");

		std::vector<int> current_values;

		if (instruction->inSubroutine)
			current_values.push_back(Why::returnAddressOffset);

		for (size_t i = 0; i < args.size(); ++i)
			current_values.push_back(Why::argumentOffset + i);

		if (!current_values.empty())
			addStack(expanded, current_values, instruction->labels, true, call->bang);

		for (size_t i = 0; i < args.size(); ++i) {
			const std::string *reg = registerArray[Why::argumentOffset + i];
			Arg &arg = args[i];
			switch (arg.getType()) {
				case Arg::Type::Address:
					expanded.emplace_back(new WASMSetNode(dynamic_cast<AddressArg &>(arg).ident, reg));
					break;
				case Arg::Type::Value:
					expanded.emplace_back(new WASMLiNode(dynamic_cast<ValueArg &>(arg).ident, reg, false));
					break;
				case Arg::Type::Number:
					expanded.emplace_back(new WASMSetNode(dynamic_cast<NumberArg &>(arg).value, reg));
					break;
				case Arg::Type::Register:
					expanded.emplace_back(new WASMMvNode(
						registerArray[dynamic_cast<RegisterArg &>(arg).reg], reg));
					break;
				default:
					throw std::runtime_error("Invalid Arg type");
			}
			expanded.back()->setBang(call->bang);
		}

		expanded.emplace_back((new WASMJNode(call->function, true))->setBang(call->bang));

		std::reverse(current_values.begin(), current_values.end());
		addStack(expanded, current_values, {}, false, call->bang);
	}

	void Assembler::addStack(Statements &expanded, const std::vector<int> &regs, const Strings &labels, bool is_push,
	                         int bang) {
		bool first = true;
		for (const int reg: regs) {
			auto node = std::make_shared<WASMStackNode>(registerArray[reg], is_push);
			if (first)
				node->labels = labels;
			else
				first = false;
			node->setBang(bang);
			expanded.push_back(node);
		}
	}

	std::vector<Long> Assembler::createDebugData(const ASTNode *node, const Statements &expanded) {
		std::vector<Long> out;
		debugEntries.clear();
		debugEntries.reserve(node->size());

		for (const ASTNode *child: *node) {
			if (child->symbol != WASMTOK_NUMBER) {
				child->debug();
				throw std::runtime_error("Unexpected symbol found in debug section at " + std::string(node->location)
					+ ": " + std::string(parser.getName(node->symbol)));
			}
			const int type = child->atoi();
			switch (type) {
				case 1:
				case 2: {
					const ASTNode *subchild = child->front();
					const std::string unquoted = subchild->unquote();
					const size_t length = unquoted.size();
					if (0xffffff < length)
						throw std::runtime_error("Name too long (" + std::to_string(length) + " characters, type = "
							+ std::to_string(type) + ")");
					std::vector<uint8_t> encoded {
						static_cast<uint8_t>(type), static_cast<uint8_t>(length & 0xff),
						static_cast<uint8_t>((length >> 8) & 0xff), static_cast<uint8_t>((length >> 16) & 0xff)
					};
					for (char ch: unquoted)
						encoded.push_back(static_cast<uint8_t>(ch));
					for (const Long piece: getLongs(encoded))
						out.push_back(piece);
					if (type == 1)
						debugEntries.emplace_back(new DebugFilename(unquoted));
					else
						debugEntries.emplace_back(new DebugFunction(unquoted));
					break;
				}

				case 3:
					debugEntries.emplace_back(new DebugLocation((*child)[0]->atoi(), (*child)[1]->atoi(),
						(*child)[2]->atoi(), (*child)[3]->atoi()));
					break;

				default:
					throw std::runtime_error("Unexpected debug node type: " + std::to_string(type));
			}
		}

		const size_t expanded_size = expanded.size();
		const size_t debug_size = debugEntries.size();
		for (size_t i = 0; i < expanded_size; ++i) {
			const auto &instruction = expanded[i];
			const int bang = instruction->bang;
			if (bang == -1)
				continue;
			if (bang < 0 || debug_size <= static_cast<size_t>(bang)) {
				instruction->debug();
				throw std::runtime_error("Debug intbang out of bounds: " + std::to_string(bang));
			}
			if (debugEntries[bang]->getType() != DebugEntry::Type::Location) {
				instruction->debug();
				throw std::runtime_error("Debug intbang doesn't refer to a location");
			}
			DebugLocation *location = dynamic_cast<DebugLocation *>(debugEntries[bang].get());
			if (!location) {
				instruction->debug();
				throw std::runtime_error("DebugLocation cast failed");
			}
			const size_t address = metaOffsetCode() + 8 * i;
			size_t count = 1;
			for (size_t j = i + 1; j < expanded_size; ++j)
				if (bang == expanded[j]->bang)
					++count;
				else
					break;

			if (0xff < count)
				throw std::runtime_error("Instruction count too high: " + std::to_string(count));

			if (0xffffff < location->fileIndex)
				throw std::runtime_error("File index too large: " + std::to_string(location->fileIndex));

			if (0xffffffff < location->line)
				throw std::runtime_error("Line number too large: " + std::to_string(location->line));

			if (0xffffff < location->column)
				throw std::runtime_error("Column number too large: " + std::to_string(location->column));

			if (0xffffff < location->functionIndex)
				throw std::runtime_error("Function index too large: " + std::to_string(location->functionIndex));

			std::vector<uint8_t> bytes {static_cast<uint8_t>(location->getType())};

			auto add = [&](size_t n, size_t byte_count) {
				for (size_t j = 0; j < byte_count; ++j)
					bytes.push_back((n >> (8 * j)) & 0xff);
			};

			add(location->fileIndex, 3);
			add(location->line, 4);
			add(location->column, 3);
			bytes.push_back(count & 0xff);
			add(location->functionIndex, 4);
			add(address, 8);

			for (const Long piece: getLongs(bytes))
				out.push_back(piece);
			i += count - 1;
		}

		return out;
	}
}
