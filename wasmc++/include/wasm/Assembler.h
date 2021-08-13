#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "parser/Parser.h"

namespace Wasmc {
	struct WASMInstructionNode;

	using Long = uint64_t;
	using Statements = std::vector<std::shared_ptr<WASMInstructionNode>>;
	using Strings = std::vector<const std::string *>;

	enum class SymbolType: unsigned {Unknown, KnownPointer, UnknownPointer, Code, Data};

	class Assembler {
		public:
			Assembler(Parser &);

			void assemble();

		private:
			Parser &parser;
			std::unordered_map<const std::string *, Long> offsets, dataOffsets;
			std::vector<Long> meta, data, code, symbolTable, debugData, assembled;
			std::unordered_set<const std::string *> allLabels, unknownSymbols;
			std::unordered_map<const std::string *, const std::string *> dataVariables;
			std::unordered_map<uint32_t, const std::string *> hashes;
			bool verbose = false;

			Long & metaOffsetSymbols() { return meta.at(0); }
			Long & metaOffsetCode()    { return meta.at(1); }
			Long & metaOffsetData()    { return meta.at(2); }
			Long & metaOffsetDebug()   { return meta.at(3); }
			Long & metaOffsetEnd()     { return meta.at(4); }

			ASTNode *metaNode = nullptr, *includeNode = nullptr, *dataNode = nullptr, *debugNode = nullptr,
			        *codeNode = nullptr;

			/** Throws an exception if there exist more than one of any section type in the AST. */
			void validateSectionCounts();

			void findAllLabels();

			std::vector<Long> createSymbolTable(std::unordered_set<const std::string *> labels, bool skeleton);

			uint32_t encodeSymbol(const std::string *);

			std::vector<Long> str2longs(const std::string &) const;

			void processMetadata();

			void processData();

			std::vector<Long> convertDataPieces(const ASTNode *);

			Statements expandCode();

			void addCall(Statements &, const WASMInstructionNode *);

			void addStack(Statements &, const std::vector<int> &regs, const Strings &labels, bool is_push);
	};

}
