#pragma once

#include <memory>
#include <vector>

namespace Wasmc {
	class ASTNode;

	struct Arg {
		enum class Type {Register, Address, Value, Number};
		virtual ~Arg() {}
		virtual Type getType() = 0;
		virtual operator std::string() const = 0;
	};

	struct RegisterArg: Arg {
		int reg;
		RegisterArg(ASTNode *);
		Type getType() override { return Type::Register; }
		operator std::string() const override;
	};

	struct AddressArg: Arg {
		const std::string *ident;
		AddressArg(ASTNode *);
		Type getType() override { return Type::Address; }
		operator std::string() const override;
	};

	struct ValueArg: Arg {
		const std::string *ident;
		ValueArg(ASTNode *);
		Type getType() override { return Type::Value; }
		operator std::string() const override;
	};

	struct NumberArg: Arg {
		int64_t value;
		NumberArg(ASTNode *);
		Type getType() override { return Type::Number; }
		operator std::string() const override;
	};

	struct Args {
		std::vector<std::unique_ptr<Arg>> args;
		Args(ASTNode *);
		size_t size() const { return args.size(); }
		bool empty() const { return args.empty(); }
		decltype(args)::const_iterator begin() const { return args.begin(); }
		decltype(args)::const_iterator end() const { return args.end(); }
		decltype(args)::const_iterator cbegin() const { return args.cbegin(); }
		decltype(args)::const_iterator cend() const { return args.cend(); }
	};
}
