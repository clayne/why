#pragma once

#include <string>
#include <vector>

#include "wasm/Debug.h"
#include "wasm/SymbolTable.h"
#include "wasm/Types.h"

namespace Wasmc {
	class BinaryParser {
		public:
			std::vector<Long> raw, rawMeta, rawSymbols, rawCode, rawData, rawDebugData;
			std::string name, version, author, orcid;
			SymbolTable symbols;
			std::vector<std::unique_ptr<AnyBase>> code;
			std::vector<std::unique_ptr<DebugEntry>> debugData;

			BinaryParser() = delete;
			BinaryParser(const BinaryParser &) = default;
			BinaryParser(BinaryParser &&) = default;

			BinaryParser(const std::vector<Long> &);

			BinaryParser & operator=(const BinaryParser &) = default;
			BinaryParser & operator=(BinaryParser &&) = default;

			static AnyBase * parse(Long);

			void parse();

			inline Long getMetaLength() const;
			inline Long getSymbolTableLength() const;
			inline Long getCodeLength() const;
			inline Long getDataLength() const;
			inline Long getDebugLength() const;

			inline Long getMetaOffset() const;
			inline Long getSymbolTableOffset() const;
			inline Long getCodeOffset() const;
			inline Long getDataOffset() const;
			inline Long getDebugOffset() const;
			inline Long getEndOffset() const;

		private:
			std::vector<Long> slice(size_t begin, size_t end);
			SymbolTable getSymbols() const;
			std::vector<std::unique_ptr<DebugEntry>> getDebugData() const;

			static std::string toString(Long);
	};
}
