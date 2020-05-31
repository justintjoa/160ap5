#pragma once

#include <assert.h>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "frontend/ast.h"
#include "frontend/ast_visitor.h"

using namespace cs160::frontend;

namespace cs160::midend {

struct IRError : public std::runtime_error {
  explicit IRError(const std::string& message) : runtime_error(message) {}
};

enum class Opcode {
  NOT = 1,
  CALL,
  ADD,
  SUB,
  MUL,
  LT,
  LE,
  EQ,
  AND,
  OR,
  jump_unconditional,
  jump_conditional,
  arg,
  ret,
  output,
  NIL
};

enum class OperandType {
  Var,
  Int,
  Label,
  Function  // since we're not using a symbol table we need to be a bit
            // underhanded
};

const std::string OpcodeToString(Opcode op_);

class Operand {
 public:
  Operand() {}
  Operand(int constant) : t_(OperandType::Int), constant_(constant) {
    repr = std::to_string(constant_);
  }
  Operand(std::string var_name, OperandType t) : t_(t), var_name_(var_name) {
    if (t == OperandType::Label) {
      repr = var_name + ":";
    } else {
      repr = var_name;
    }
  }
  OperandType GetOperandType() const { return t_; }
  int GetConstant() const {
    assert(t_ == OperandType::Int);
    return constant_;
  }
  std::string GetVariableName() const {
    assert(t_ != OperandType::Int);
    return var_name_;
  }

  std::string const toString() const { return repr; }

 private:
  std::string repr;
  OperandType t_;
  std::string var_name_;
  int constant_;
};

// "<-" operator implicit
class Instruction {
 public:
  Instruction(Operand lhs, Opcode op, Operand rhs1, Operand rhs2)
      : operand0_(lhs),
        op(op),
        operand1_(rhs1),
        operand2_(rhs2),
        is_label(false) {
    repr = operand0_.toString() + " <- " + operand1_.toString() + " " +
           OpcodeToString(op) + " " + operand2_.toString();
  }  // binops
  Instruction(Operand lhs, Opcode op, Operand rhs1)
      : operand0_(lhs), op(op), operand1_(rhs1), operand2_(), is_label(false) {
    if (operand1_.GetOperandType() == OperandType::Label) {
      repr = OpcodeToString(op) + " " + operand0_.toString() + " " +
             operand1_.toString();
    } else {
      repr = operand0_.toString() + " <- " + OpcodeToString(op) + " " +
             operand1_.toString();
    }
  }  // var <- NOT var, var <- CALL foo, but also jump_if_0 op1 op2
  Instruction(Opcode opc, Operand rhs1)
      : operand0_(rhs1), op(opc), operand1_(), operand2_(), is_label(false) {
    repr = OpcodeToString(op) + " " + operand0_.toString();
  }  // arg var, output var, return var, jump tgt
  Instruction(Operand lhs, Operand rhs1)
      : operand0_(lhs), operand1_(rhs1), operand2_(), is_label(false) {
    repr = operand0_.toString() + " <- " + operand1_.toString();
  }  // var <- var

  // so we can have a label in the instruction stream
  Instruction(Operand lhs)
      : operand0_(lhs), operand1_(), operand2_(), is_label(true) {
    assert(operand0_.GetOperandType() == OperandType::Label);
    repr = operand0_.toString();
  }

  std::string const toString() const { return repr; }
  const Operand getJumpTarget() {
    assert(op == Opcode::jump_conditional || op == Opcode::jump_unconditional);
    if (op == Opcode::jump_conditional) {
      return operand1_;
    } else {
      return operand0_;
    }
  }
  std::optional<Operand> const getLabel() const {
    if (operand0_.GetOperandType() == OperandType::Label &&
        getOpcode() != Opcode::jump_unconditional) {
      return operand0_;
    } else {
      return std::nullopt;
    }
  }

  Opcode const getOpcode() const { return op; }
  Operand const getOperand0() const { return operand0_; }

  bool isUnary() const {
    auto op = getOpcode();
    if (op == Opcode::NOT || op == Opcode::CALL) {
      return true;
    } else {
      return false;
    }
  }

  bool isSSA() const {
    auto op1_type = operand1_.GetOperandType();
    if (op1_type == OperandType::Var || op1_type == OperandType::Int ||
        op1_type == OperandType::Function) {
      return true;
    } else {
      return false;
    }
  }

  bool isBinary() const {
    if (op == Opcode::ADD || op == Opcode::SUB || op == Opcode::MUL ||
        op == Opcode::LT || op == Opcode::LE || op == Opcode::EQ ||
        op == Opcode::AND) {
      return true;
    }
    return false;
  }

  Operand const getOperand1() const {
    assert(isUnary() || isBinary());
    return operand1_;
  }

  Operand const getOperand2() const {
    assert(isBinary());
    return operand2_;
  }

  bool isLabel() { return is_label; }

 private:
  Operand operand0_;
  Opcode op = Opcode::NIL;
  Operand operand1_;
  Operand operand2_;
  std::string repr;
  bool is_label;
};

class BasicBlock {
 public:
  bool operator==(const BasicBlock& rhs) {
    if (blockID == rhs.blockID) {
      return true;
    } else {
      return false;
    }
  }
  // bool operator!=(const BasicBlock& rhs) const { return !operator==(rhs); }

  BasicBlock(std::vector<Instruction> instr, std::set<int> succ, int idx)
      : blockID(idx), instructions_(instr), successors_(succ) {}
  BasicBlock(std::vector<Instruction> instr, std::set<int> succ,
             std::set<int> pred, int idx)
      : blockID(idx),
        instructions_(instr),
        successors_(succ),
        predecessors_(pred) {}

  const std::vector<Instruction>& instructions() const { return instructions_; }
  const std::set<int>& getSuccessors() { return successors_; }
  const std::set<int>& getPredecessors() { return predecessors_; }
  void insertPredecessor(int pred) { predecessors_.insert(pred); }
  int getBlockID() { return blockID; }

 private:
  int blockID;
  std::vector<Instruction> instructions_;
  std::set<int> successors_;
  std::set<int> predecessors_;
};

class CFG {
 public:
  CFG(std::vector<BasicBlock> b) : basic_blocks(b) {}

  void getAllExpressions();
  void resetAvailExprs(const std::vector<std::pair<std::vector<bool>, std::vector<bool>>>&);
  std::pair<std::vector<bool>, std::vector<bool>> computeGenKill(
      BasicBlock block);
  std::vector<std::pair<std::vector<bool>, std::vector<bool>>> getAllGenKill();

  void runWorklist(
      const std::vector<std::pair<std::vector<bool>, std::vector<bool>>>&);

  void computeAvailExprs();
  std::vector<BasicBlock> computeGCSE(
      const std::vector<std::pair<std::vector<bool>, std::vector<bool>>>&);

  std::vector<std::pair<std::vector<bool>, std::vector<bool>>>
  getAvailableExpressions() {
    return availableExpressions;
  }

 private:
  std::map<std::string, int> legend;  // e.g. "a+b" -> 3
  std::vector<bool> allExprs;
  std::vector<std::pair<std::vector<bool>, std::vector<bool>>>
      availableExpressions;  // each pair holds the in set and the out set for
                             // a block
  std::vector<BasicBlock> optimized_program;  // final answer
  std::vector<BasicBlock> basic_blocks;       //
};

// similar to codegen context
struct IRContext {
  std::unique_ptr<IRContext> parent;
  // Information about the stack space and the current local variable context
  // uint32_t nextOffset = 4;
};

// "Symbol table" reused from code gen just for generating tmp variables
struct IRSymbolTable {
  IRContext ctx;
  static const std::string tmpPrefix;
  uint32_t nextTmp = 0;
};

// The TAC IR is implemented as an AST visitor that will generate the
// relevant pieces of code as it traverses a node
class IR final : public AstVisitor {
 public:
  // Entry point of the code generator. This function should visit given
  // program and return generated code as a list of three address code
  // instructions
  std::vector<Instruction> generateCFG(const Program& program);

  // Visitor functions
  void VisitIntegerExpr(const IntegerExpr& exp) override;
  void VisitVariableExpr(const VariableExpr& exp) override;
  void VisitAddExpr(const AddExpr& exp) override;
  void VisitSubtractExpr(const SubtractExpr& exp) override;
  void VisitMultiplyExpr(const MultiplyExpr& exp) override;

  void VisitLessThanExpr(const LessThanExpr& exp) override;
  void VisitLessThanEqualToExpr(const LessThanEqualToExpr& exp) override;
  void VisitEqualToExpr(const EqualToExpr& exp) override;

  void VisitLogicalAndExpr(const LogicalAndExpr& exp) override;
  void VisitLogicalOrExpr(const LogicalOrExpr& exp) override;
  void VisitLogicalNotExpr(const LogicalNotExpr& exp) override;

  void VisitIntTypeExpr(const IntType& exp) override;
  void VisitBlockExpr(const BlockExpr& exp) override;
  void VisitDeclarationExpr(const Declaration& exp) override;
  void VisitAssignmentExpr(const Assignment& assignment) override;

  void VisitConditionalExpr(const Conditional& conditional) override;
  void VisitLoopExpr(const Loop& loop) override;

  void VisitFunctionCallExpr(const FunctionCall& call) override;
  void VisitFunctionDefExpr(const FunctionDef& def) override;

  void VisitProgramExpr(const Program& program) override;

  std::vector<int> getLeaders(std::vector<Instruction>);
  std::vector<BasicBlock> getBB(std::vector<Instruction>);

  std::map<std::string, std::vector<BasicBlock>> ProgramBlocks() {
    return program_blocks;
  }

 private:
  std::vector<Operand> arg_stack;

  // Next index for label generation
  uint32_t nextIndex = 0;
  uint32_t freshIndex() { return nextIndex++; }

  // List of instructions generated
  std::vector<Instruction> insns;

  // for each function def
  std::map<std::string, std::vector<BasicBlock>> program_blocks;

  // Symbol table
  IRSymbolTable symbolTable;

  // reused from codegen
  class TmpVar {
    std::string name;
    IR& ir;

   public:
    std::string getName() { return name; };
    explicit TmpVar(const std::string& name, IR& ir);
    TmpVar(const TmpVar&) = delete;
    ~TmpVar();
    int32_t operator*() const;
  };

  // Create a fresh temporary variable
  TmpVar freshTmp();
};

}  // namespace cs160::midend
