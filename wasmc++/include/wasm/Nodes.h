#pragma once

#include <memory>
#include <unordered_map>

#include "compiler/Immediate.h"
#include "parser/ASTNode.h"
#include "parser/Enums.h"
#include "wasm/Args.h"
#include "wasm/Instructions.h"

namespace Wasmc {
	enum class WASMNodeType {
		Immediate, RType, IType, Copy, Load, Store, Set, Li, Si, Lni, Ch, Lh, Sh, Cmp, Cmpi, Sel, J, Jc, Jr, Jrc, Mv,
		SizedStack, MultR, MultI, DiviI, Lui, Stack, Nop, IntI, RitI, TimeI, TimeR, RingI, RingR, Print, Halt, SleepR,
		Page, SetptI, Label, SetptR, Svpg, Query, PseudoPrint, Statement, Call, StringPrint, Jeq, JeqI
	};

	enum class SymbolType: unsigned {Unknown = 0, KnownPointer, UnknownPointer, Code, Data};
	enum class ConstantFlags: unsigned {Ignore = 0, KnownSymbol, UnknownSymbol, SymbolID};

	class WhyInstruction;
	class Function;
	class Variable;

	using VarMap = std::unordered_map<const std::string *, std::shared_ptr<Variable>>;

	Condition getCondition(const std::string &);
	Condition getCondition(const ASTNode *);

	struct WASMBaseNode: ASTNode {
		WASMBaseNode(int sym);
		virtual WASMNodeType nodeType() const = 0;
		virtual operator std::string() const = 0;
	};

	struct HasImmediate {
		Immediate imm;
		HasImmediate(const Immediate &imm_): imm(imm_) {}
	};

	struct HasSecondImmediate {
		Immediate imm2;
		HasSecondImmediate(const Immediate &imm2_): imm2(imm2_) {}
	};

	struct HasRS {
		const std::string *rs;
		HasRS(const ASTNode *node): rs(node? node->lexerInfo : nullptr) {}
		HasRS(const std::string *rs_): rs(rs_) {}
	};

	struct HasRT {
		const std::string *rt;
		HasRT(const ASTNode *node): rt(node? node->lexerInfo : nullptr) {}
		HasRT(const std::string *rt_): rt(rt_) {}
	};

	struct HasRD {
		const std::string *rd;
		HasRD(const ASTNode *node): rd(node? node->lexerInfo : nullptr) {}
		HasRD(const std::string *rd_): rd(rd_) {}
	};

	struct HasOper {
		const std::string *oper;
		int operToken;
		HasOper(const ASTNode *node): HasOper(node->symbol, node->lexerInfo) {}
		HasOper(int oper_token, const std::string *oper_ = nullptr): oper(oper_), operToken(oper_token) {}
	};

	struct HasUnsigned {
		bool isUnsigned;
		HasUnsigned(const ASTNode *node): HasUnsigned(!!node) {}
		HasUnsigned(bool is_unsigned): isUnsigned(is_unsigned) {}
	};

	struct ThreeRegs: HasRS, HasRT, HasRD {
		ThreeRegs(const ASTNode *rs_, const ASTNode *rt_, const ASTNode *rd_): HasRS(rs_), HasRT(rt_), HasRD(rd_) {}
		ThreeRegs(const std::string *rs_, const std::string *rt_, const std::string *rd_):
			HasRS(rs_), HasRT(rt_), HasRD(rd_) {}
	};

	struct TwoRegs: HasRS, HasRD {
		TwoRegs(const ASTNode *rs_, const ASTNode *rd_): HasRS(rs_), HasRD(rd_) {}
		TwoRegs(const std::string *rs_, const std::string *rd_): HasRS(rs_), HasRD(rd_) {}
	};

	struct WASMInstructionNode: WASMBaseNode {
		int bang = -1;
		std::vector<const std::string *> labels;
		bool inSubroutine = false;
		SymbolType symbolType = SymbolType::Unknown;
		ConstantFlags flags = ConstantFlags::Ignore;

		using WASMBaseNode::WASMBaseNode;

		WASMInstructionNode * absorbIntbang(ASTNode *);
		WASMInstructionNode * absorbLabel(ASTNode *);
		WASMInstructionNode * absorb(const WASMInstructionNode &);
		WASMInstructionNode * setInSubroutine(bool);
		WASMInstructionNode * setBang(int);
		WASMInstructionNode * setSymbolType(SymbolType);
		virtual WASMInstructionNode * copy() const = 0;
		WASMNodeType nodeType() const override { return WASMNodeType::Statement; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMImmediateNode: WASMBaseNode, HasImmediate {
		WASMImmediateNode(ASTNode *);
		WASMImmediateNode(const Immediate &);
		WASMNodeType nodeType() const override { return WASMNodeType::Immediate; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMLabelNode: WASMInstructionNode { // Not technically an instruction, but still.
		const std::string *label;

		WASMLabelNode(ASTNode *);
		WASMLabelNode(const std::string *);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::Label; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct AnyType {
		virtual Opcode getOpcode() const = 0;
	};

	struct RType: AnyType, ThreeRegs, HasOper {
		RType(const ASTNode *rs_, const ASTNode *rt_, const ASTNode *rd_, const ASTNode *oper_):
			ThreeRegs(rs_, rt_, rd_), HasOper(oper_) {}
		RType(const std::string *rs_, const std::string *rt_, const std::string *rd_, int oper_token,
		      const std::string *oper_ = nullptr):
			ThreeRegs(rs_, rt_, rd_), HasOper(oper_token, oper_) {}
	};

	struct JType: AnyType, HasImmediate, HasRS {
		Condition condition;
		bool link;
		JType(const ASTNode *cond, const ASTNode *colons, const ASTNode *addr, const ASTNode *rs_):
		HasImmediate(getImmediate(addr)), HasRS(rs_), condition(getCondition(cond)), link(!colons->empty()) {
			delete colons;
			delete cond;
		}
	};

	struct RNode: WASMInstructionNode, RType, HasUnsigned {
		RNode(ASTNode *rs_, ASTNode *oper_, ASTNode *rt_, ASTNode *rd_, ASTNode *unsigned_);
		RNode(const std::string *rs_, const std::string *oper_, const std::string *rt_, const std::string *rd_,
		      int oper_token, bool is_unsigned);
		WASMInstructionNode * copy() const override;
		Opcode getOpcode() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::RType; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct INode: WASMInstructionNode, HasImmediate, TwoRegs, HasOper, HasUnsigned {
		INode(ASTNode *rs_, ASTNode *oper_, ASTNode *imm, ASTNode *rd_, ASTNode *unsigned_);
		INode(const std::string *rs_, const std::string *oper_, const Immediate &imm_, const std::string *rd_,
		      int oper_token, bool is_unsigned);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::IType; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMMemoryNode: WASMInstructionNode, TwoRegs {
		bool isByte;

		WASMMemoryNode(int sym, ASTNode *rs_, ASTNode *rd_, ASTNode *byte_);
		WASMMemoryNode(int sym, const std::string *rs_, const std::string *rd_, bool is_byte);
	};

	struct WASMCopyNode: WASMMemoryNode {
		WASMCopyNode(ASTNode *rs_, ASTNode *rd_, ASTNode *byte_);
		WASMCopyNode(const std::string *rs_, const std::string *rd_, bool is_byte);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::Copy; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMLoadNode: WASMMemoryNode {
		WASMLoadNode(ASTNode *rs_, ASTNode *rd_, ASTNode *byte_);
		WASMLoadNode(const std::string *rs_, const std::string *rd_, bool is_byte);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::Load; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMStoreNode: WASMMemoryNode {
		WASMStoreNode(ASTNode *rs_, ASTNode *rd_, ASTNode *byte_);
		WASMStoreNode(const std::string *rs_, const std::string *rd_, bool is_byte);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::Store; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMSetNode: WASMInstructionNode, HasImmediate, HasRD {
		WASMSetNode(ASTNode *imm_, ASTNode *rd_);
		WASMSetNode(const Immediate &imm_, const std::string *rd_);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::Set; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMLiNode: WASMInstructionNode, HasImmediate, HasRD {
		bool isByte;

		WASMLiNode(ASTNode *imm_, ASTNode *rd_, ASTNode *byte_);
		WASMLiNode(const Immediate &imm_, const std::string *rd_, bool is_byte);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::Li; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMSiNode: WASMInstructionNode, HasImmediate, HasRS {
		bool isByte;

		WASMSiNode(ASTNode *rs_, ASTNode *imm_, ASTNode *byte_);
		WASMSiNode(const Immediate &imm_, const std::string *rd_, bool is_byte);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::Si; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMLniNode: WASMLiNode {
		WASMLniNode(ASTNode *imm_, ASTNode *rd_, ASTNode *byte_);
		WASMLniNode(const Immediate &imm_, const std::string *rd_, bool is_byte);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::Lni; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMHalfMemoryNode: WASMInstructionNode, TwoRegs {
		WASMHalfMemoryNode(int sym, ASTNode *rs_, ASTNode *rd_);
		WASMHalfMemoryNode(int sym, const std::string *rs_, const std::string *rd_);
	};

	struct WASMChNode: WASMHalfMemoryNode {
		WASMChNode(ASTNode *rs_, ASTNode *rd_);
		WASMChNode(const std::string *rs_, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMChNode(rs, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Ch; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMLhNode: WASMHalfMemoryNode {
		WASMLhNode(ASTNode *rs_, ASTNode *rd_);
		WASMLhNode(const std::string *rs_, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMLhNode(rs, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Lh; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMShNode: WASMHalfMemoryNode {
		WASMShNode(ASTNode *rs_, ASTNode *rd_);
		WASMShNode(const std::string *rs_, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMShNode(rs, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Sh; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMCmpNode: WASMInstructionNode, HasRS, HasRT {
		WASMCmpNode(ASTNode *rs_, ASTNode *rt_);
		WASMCmpNode(const std::string *rs_, const std::string *rt_);
		WASMInstructionNode * copy() const override { return (new WASMCmpNode(rs, rt))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Cmp; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMCmpiNode: WASMInstructionNode, HasImmediate, HasRS {
		WASMCmpiNode(ASTNode *rs_, ASTNode *imm_);
		WASMCmpiNode(const std::string *rs_, const Immediate &imm_);
		WASMInstructionNode * copy() const override { return (new WASMCmpiNode(rs, imm))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Cmpi; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMSelNode: WASMInstructionNode, ThreeRegs {
		Condition condition;

		WASMSelNode(ASTNode *rs_, ASTNode *oper_, ASTNode *rt_, ASTNode *rd_);
		WASMSelNode(const std::string *rs_, Condition cond, const std::string *rt_, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMSelNode(rs, condition, rt, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Sel; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMJNode: WASMInstructionNode, HasImmediate {
		Condition condition;
		bool link;

		WASMJNode(ASTNode *cond, ASTNode *colons, ASTNode *addr_);
		WASMJNode(const Immediate &addr, bool link_ = false, Condition cond = Condition::None);
		WASMInstructionNode * copy() const override { return (new WASMJNode(imm, link, condition))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::J; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMJeqNode: WASMInstructionNode { // Pseudoinstruction; not handled by Assembler::compileInstruction
		bool link;
		Either addr, rt;
		const std::string *rs;

		WASMJeqNode(WASMJNode *, ASTNode *rs_, ASTNode *rt_);
		WASMJeqNode(const Either &addr_, bool link_, const std::string *rs_, const Either &rt_);
		WASMInstructionNode * copy() const override { return (new WASMJeqNode(addr, link, rs, rt))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Jeq; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMJcNode: WASMInstructionNode, HasImmediate, HasRS {
		bool link;

		WASMJcNode(WASMJNode *, ASTNode *rs_);
		WASMJcNode(const Immediate &imm_, bool link_, const std::string *rs_);
		WASMInstructionNode * copy() const override { return (new WASMJcNode(imm, link, rs))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Jc; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	// Used for both jr and jrl.
	struct WASMJrNode: WASMInstructionNode, HasRD {
		Condition condition;
		bool link;

		WASMJrNode(ASTNode *cond, ASTNode *colons, ASTNode *rd_);
		WASMJrNode(Condition condition_, bool link_, const std::string &rd_);
		WASMJrNode(Condition condition_, bool link_, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMJrNode(condition, link, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Jr; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	// Used for both jrc and jrlc.
	struct WASMJrcNode: WASMInstructionNode, TwoRegs {
		bool link;

		WASMJrcNode(WASMJrNode *, ASTNode *rs_);
		WASMJrcNode(bool link_, const std::string *rs_, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMJrcNode(link, rs, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Jrc; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	// Used for both sspush and sspop.
	struct WASMSizedStackNode: WASMInstructionNode {
		long size;
		const std::string *rs;
		bool isPush;

		WASMSizedStackNode(ASTNode *size_, ASTNode *rs_, bool is_push);
		WASMSizedStackNode(long size_, const std::string *rs_, bool is_push);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::SizedStack; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMMultRNode: WASMInstructionNode {
		const std::string *rs, *rt;
		bool isUnsigned;

		WASMMultRNode(ASTNode *rs_, ASTNode *rt_, ASTNode *unsigned_ = nullptr);
		WASMMultRNode(const std::string *rs_, const std::string *rt_, bool is_unsigned);
		WASMInstructionNode * copy() const override { return (new WASMMultRNode(rs, rt, isUnsigned))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::MultR; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMMultINode: WASMInstructionNode, HasImmediate {
		const std::string *rs;
		bool isUnsigned;

		WASMMultINode(ASTNode *rs_, ASTNode *imm_, ASTNode *unsigned_ = nullptr);
		WASMMultINode(const std::string *rs_, const Immediate &imm_, bool is_unsigned);
		WASMInstructionNode * copy() const override { return (new WASMMultINode(rs, imm, isUnsigned))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::MultI; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMDiviINode: WASMInstructionNode, HasImmediate {
		const std::string *rs, *rd;
		bool isUnsigned;

		WASMDiviINode(ASTNode *imm_, ASTNode *rs_, ASTNode *rd_, ASTNode *unsigned_ = nullptr);
		WASMDiviINode(const Immediate &imm_, const std::string *rs_, const std::string *rd_, bool is_unsigned);
		WASMInstructionNode * copy() const override;
		WASMNodeType nodeType() const override { return WASMNodeType::DiviI; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMLuiNode: WASMInstructionNode, HasImmediate {
		const std::string *rd;

		WASMLuiNode(ASTNode *imm_, ASTNode *rd_);
		WASMLuiNode(const Immediate &imm_, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMLuiNode(imm, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Lui; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMStackNode: WASMInstructionNode {
		const std::string *reg;
		bool isPush;

		WASMStackNode(ASTNode *reg_, bool is_push);
		WASMStackNode(const std::string *reg_, bool is_push);
		WASMInstructionNode * copy() const override { return (new WASMStackNode(reg, isPush))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Stack; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMNopNode: WASMInstructionNode {
		WASMNopNode();
		WASMInstructionNode * copy() const override { return (new WASMNopNode())->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Nop; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMIntINode: WASMInstructionNode, HasImmediate {
		WASMIntINode(ASTNode *imm_);
		WASMIntINode(const Immediate &imm_);
		WASMInstructionNode * copy() const override { return (new WASMIntINode(imm))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::IntI; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMRitINode: WASMInstructionNode, HasImmediate {
		WASMRitINode(ASTNode *imm_);
		WASMRitINode(const Immediate &imm_);
		WASMInstructionNode * copy() const override { return (new WASMRitINode(imm))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::RitI; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMTimeINode: WASMInstructionNode, HasImmediate {
		WASMTimeINode(ASTNode *imm_);
		WASMTimeINode(const Immediate &imm_);
		WASMInstructionNode * copy() const override { return (new WASMTimeINode(imm))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::TimeI; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMTimeRNode: WASMInstructionNode {
		const std::string *rs;

		WASMTimeRNode(ASTNode *rs_);
		WASMTimeRNode(const std::string *rs_);
		WASMInstructionNode * copy() const override { return (new WASMTimeRNode(rs))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::TimeR; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMRingINode: WASMInstructionNode, HasImmediate {
		WASMRingINode(ASTNode *imm_);
		WASMRingINode(const Immediate &imm_);
		WASMInstructionNode * copy() const override { return (new WASMRingINode(imm))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::RingI; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMRingRNode: WASMInstructionNode {
		const std::string *rs;

		WASMRingRNode(ASTNode *rs_);
		WASMRingRNode(const std::string *rs_);
		WASMInstructionNode * copy() const override { return (new WASMRingRNode(rs))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::RingR; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMPrintNode: WASMInstructionNode {
		const std::string *rs;
		PrintType type;

		WASMPrintNode(ASTNode *rs_, ASTNode *type_);
		WASMPrintNode(const std::string *rs_, PrintType type_);
		WASMInstructionNode * copy() const override { return (new WASMPrintNode(rs, type))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Print; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMHaltNode: WASMInstructionNode {
		WASMHaltNode();
		WASMInstructionNode * copy() const override { return (new WASMHaltNode())->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Halt; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMSleepRNode: WASMInstructionNode {
		const std::string *rs;

		WASMSleepRNode(ASTNode *rs_);
		WASMSleepRNode(const std::string *rs_);
		WASMInstructionNode * copy() const override { return (new WASMSleepRNode(rs))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::SleepR; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMPageNode: WASMInstructionNode {
		bool on;

		WASMPageNode(bool on_);
		WASMInstructionNode * copy() const override { return (new WASMPageNode(on))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Page; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMSetptINode: WASMInstructionNode, HasImmediate {
		WASMSetptINode(ASTNode *imm_);
		WASMSetptINode(const Immediate &imm_);
		WASMInstructionNode * copy() const override { return (new WASMSetptINode(imm))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::SetptI; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMSetptRNode: WASMInstructionNode {
		const std::string *rs;

		WASMSetptRNode(ASTNode *rs_);
		WASMSetptRNode(const std::string *rs_);
		WASMInstructionNode * copy() const override { return (new WASMSetptRNode(rs))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::SetptR; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMMvNode: WASMInstructionNode {
		const std::string *rs, *rd;

		WASMMvNode(ASTNode *rs_, ASTNode *rd_);
		WASMMvNode(const std::string *rs_, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMMvNode(rs, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Mv; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMSvpgNode: WASMInstructionNode {
		const std::string *rd;

		WASMSvpgNode(ASTNode *rd_);
		WASMSvpgNode(const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMSvpgNode(rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Svpg; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMQueryNode: WASMInstructionNode {
		QueryType type;
		const std::string *rd;

		WASMQueryNode(QueryType, ASTNode *rd_);
		WASMQueryNode(QueryType, const std::string *rd_);
		WASMInstructionNode * copy() const override { return (new WASMQueryNode(type, rd))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Query; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMPseudoPrintNode: WASMInstructionNode, HasImmediate {
		WASMPseudoPrintNode(ASTNode *imm_);
		WASMPseudoPrintNode(const Immediate &imm_);
		WASMInstructionNode * copy() const override { return (new WASMPseudoPrintNode(imm))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::PseudoPrint; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMStringPrintNode: WASMInstructionNode {
		const std::string *string;

		WASMStringPrintNode(ASTNode *string_);
		WASMStringPrintNode(const std::string *string_);
		WASMInstructionNode * copy() const override { return (new WASMStringPrintNode(string))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::StringPrint; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	struct WASMCallNode: WASMInstructionNode {
		const std::string *function;
		Args args;

		WASMCallNode(ASTNode *function_, ASTNode *args_ = nullptr);
		WASMCallNode(const std::string *function_, const Args &args_);
		WASMInstructionNode * copy() const override { return (new WASMCallNode(function, args))->absorb(*this); }
		WASMNodeType nodeType() const override { return WASMNodeType::Call; }
		std::string debugExtra() const override;
		operator std::string() const override;
	};

	RNode * makeSeq(const std::string *rs, const std::string *rt, const std::string *rd, int bang = -1);
}
