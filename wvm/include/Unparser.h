#pragma once

#include <string>

#include "Defs.h"

namespace WVM::Unparser {
	std::string stringify(UWord instruction, const VM *vm = nullptr);

	std::string stringifyRType(int opcode, int rs, int rt, int rd, Conditions, int funct);
	std::string stringifyIType(int opcode, int rs, int rd, Conditions, int flags, HWord immediate,
		const VM * = nullptr);
	std::string stringifyJType(int opcode, int rs, bool link, Conditions, int flags, HWord address,
		const VM * = nullptr);

	std::string rAltOp(int rs, int rt, int rd, const std::string &oper, const std::string &suffix = "");
	std::string iAltOp(int rs, int rd, const std::string &immediate, const std::string &oper);
	std::string iAltOpInv(int rs, int rd, const std::string &immediate, const std::string &oper);
	std::string iMath(int rs, int rd, const std::string &immediate, const std::string &oper);
	std::string iComp(int rs, int rd, const std::string &immediate, const std::string &oper);

	std::string jumpConditions(Conditions);
	std::string color(int reg);
	std::string colorNum(int reg);
	std::string colorOper(const std::string &oper);

	std::string getSymbol(Word location, const VM &);

	constexpr const char *immColor = "\e[38;5;164m";
	constexpr const char *into = " \e[2m->\e[22m ";
	constexpr const char *left = "[", *right = "]";
}
