#pragma once

#include <cstdint>
#include <string>

namespace Wasmc {
	struct DebugEntry {
		enum class Type: unsigned char {Filename = 1, Function = 2, Location = 3};
		virtual ~DebugEntry() {}
		virtual Type getType() const = 0;
	};

	struct DebugFilename: DebugEntry {
		const std::string filename;
		DebugFilename(const std::string &filename_): filename(filename_) {}
		Type getType() const override { return Type::Filename; }
	};

	struct DebugFunction: DebugEntry {
		const std::string function;
		DebugFunction(const std::string &function_): function(function_) {}
		Type getType() const override { return Type::Function; }
	};

	struct DebugLocation: DebugEntry {
		uint32_t fileIndex, line, column, functionIndex;
		uint8_t count = 1;
		uint64_t address = 0;
		DebugLocation(uint32_t file_index, uint32_t line_, uint32_t column_, uint32_t function_index):
			fileIndex(file_index), line(line_), column(column_), functionIndex(function_index) {}
		DebugLocation * setCount(uint8_t count_) {
			count = count_;
			return this;
		}
		DebugLocation * setAddress(uint8_t address_) {
			address = address_;
			return this;
		}
		Type getType() const override { return Type::Location; }
	};
}
