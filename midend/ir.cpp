#include <assert.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "midend/ir.h"

namespace cs160::midend {

bool sdiff(bool b1, bool b2) {
  if (b1 == true && b2 == false) {
    return true;
  } else {
    return false;
  }
}

bool sinter(bool b1, bool b2) {
  if (b1 == true && b2 == true) {
    return true;
  } else {
    return false;
  }
}

bool sunion(bool b1, bool b2) {
  if (b1 == false && b2 == false) {
    return false;
  } else {
    return true;
  }
}

std::vector<std::string> split(const std::string& s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);  // std::move ?
  }
  return elems;
}

const std::string OpcodeToString(Opcode op_) {
  switch (op_) {
    case Opcode::NOT:
      return "!";
    case Opcode::AND:
      return "AND";
    case Opcode::OR:
      return "OR";
    case Opcode::CALL:
      return "CALL";
    case Opcode::jump_unconditional:
      return "jump";
    case Opcode::jump_conditional:
      return "jump_if_0";
    case Opcode::arg:
      return "arg";
    case Opcode::ret:
      return "return";
    case Opcode::output:
      return "output";
    case Opcode::ADD:
      return "ADD";
    case Opcode::SUB:
      return "SUB";
    case Opcode::MUL:
      return "MUL";
    case Opcode::LT:
      return "LT";
    case Opcode::LE:
      return "LE";
    case Opcode::EQ:
      return "EQ";
    default:
      return "unknown or uninitialized opcode type";
  }
}

const std::string IRSymbolTable::tmpPrefix = "_tmp";

IR::TmpVar::TmpVar(const std::string& name, IR& ir) : name(name), ir(ir) {}
IR::TmpVar::~TmpVar() {}

IR::TmpVar IR::freshTmp() {
  auto name = IRSymbolTable::tmpPrefix + std::to_string(symbolTable.nextTmp++);
  return TmpVar(name, *this);
}

std::vector<Instruction> IR::generateCFG(const Program& program) {
  insns.clear();
  nextIndex = 0;
  symbolTable = {};

  VisitProgramExpr(program);
  return insns;
}

void IR::VisitIntegerExpr(const IntegerExpr& exp) {
  arg_stack.push_back(Operand(exp.value()));
}

void IR::VisitVariableExpr(const VariableExpr& exp) {
  arg_stack.push_back(Operand(exp.name(), OperandType::Var));
}

void IR::VisitAddExpr(const AddExpr& exp) {
  auto tmpVar = freshTmp();
  exp.lhs().Visit(this);
  exp.rhs().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();
  auto rhs2 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::ADD, rhs2, rhs1));
}

void IR::VisitSubtractExpr(const SubtractExpr& exp) {
  auto tmpVar = freshTmp();
  exp.lhs().Visit(this);
  exp.rhs().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();
  auto rhs2 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::SUB, rhs2, rhs1));
}

void IR::VisitMultiplyExpr(const MultiplyExpr& exp) {
  auto tmpVar = freshTmp();
  exp.lhs().Visit(this);
  exp.rhs().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();
  auto rhs2 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::MUL, rhs2, rhs1));
}

void IR::VisitLessThanExpr(const LessThanExpr& exp) {
  auto tmpVar = freshTmp();
  exp.lhs().Visit(this);
  exp.rhs().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();
  auto rhs2 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::LT, rhs2, rhs1));
}

void IR::VisitLessThanEqualToExpr(const LessThanEqualToExpr& exp) {
  auto tmpVar = freshTmp();
  exp.lhs().Visit(this);
  exp.rhs().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();
  auto rhs2 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::LE, rhs2, rhs1));
}

void IR::VisitEqualToExpr(const EqualToExpr& exp) {
  auto tmpVar = freshTmp();
  exp.lhs().Visit(this);
  exp.rhs().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();
  auto rhs2 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::EQ, rhs2, rhs1));
}

void IR::VisitLogicalAndExpr(const LogicalAndExpr& exp) {
  auto tmpVar = freshTmp();
  exp.lhs().Visit(this);
  exp.rhs().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();
  auto rhs2 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::AND, rhs2, rhs1));
}

void IR::VisitLogicalOrExpr(const LogicalOrExpr& exp) {
  auto tmpVar = freshTmp();
  exp.lhs().Visit(this);
  exp.rhs().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();
  auto rhs2 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::OR, rhs2, rhs1));
}

void IR::VisitLogicalNotExpr(const LogicalNotExpr& exp) {
  auto tmpVar = freshTmp();
  exp.operand().Visit(this);

  auto rhs1 = std::move(arg_stack.back());
  arg_stack.pop_back();

  arg_stack.push_back(Operand(tmpVar.getName(), OperandType::Var));
  insns.push_back(Instruction(Operand(tmpVar.getName(), OperandType::Var),
                              Opcode::NOT, rhs1));
}

void IR::VisitIntTypeExpr(const IntType& exp) {}

void IR::VisitBlockExpr(
    const BlockExpr& exp) {  // Insert declared variables to symbol table and
                             // initialize them to 0
  for (auto& d : exp.decls()) {
    d->Visit(this);
  }

  // Generate code for the statements, note that this may create additional
  // temporaries
  for (auto& s : exp.stmts()) {
    s->Visit(this);
  }
}

void IR::VisitDeclarationExpr(const Declaration& exp) {}

void IR::VisitAssignmentExpr(const Assignment& assignment) {
  assignment.rhs().Visit(this);
  auto rhs = arg_stack.back();

  if (rhs.GetOperandType() == OperandType::Function) {
    insns.push_back(Instruction(
        Operand(assignment.lhs().name(), OperandType::Var), Opcode::CALL, rhs));
  } else {
    insns.push_back(
        Instruction(Operand(assignment.lhs().name(), OperandType::Var), rhs));
  }
  arg_stack.push_back(Operand(assignment.lhs().name(), OperandType::Var));
}

void IR::VisitConditionalExpr(const Conditional& conditional) {
  auto n = std::to_string(freshIndex());
  auto falseLabel = Operand("IF_FALSE_" + n, OperandType::Label);
  auto endLabel = Operand("IF_END_" + n, OperandType::Label);

  conditional.guard().Visit(this);
  auto guard_expr = arg_stack.back();
  arg_stack.pop_back();

  insns.push_back(
      Instruction(guard_expr, Opcode::jump_conditional, falseLabel));
  conditional.true_branch().Visit(this);

  insns.push_back(Instruction(Opcode::jump_unconditional, endLabel));
  insns.push_back(falseLabel);

  conditional.false_branch().Visit(this);
  insns.push_back(endLabel);
}

void IR::VisitLoopExpr(const Loop& loop) {
  auto n = std::to_string(freshIndex());
  auto startLabel = Operand("WHILE_START_" + n, OperandType::Label);
  auto endLabel = Operand("WHILE_END_" + n, OperandType::Label);

  insns.push_back(startLabel);
  loop.guard().Visit(this);

  auto guard_expr = arg_stack.back();
  arg_stack.pop_back();

  insns.push_back(Instruction(guard_expr, Opcode::jump_conditional, endLabel));

  loop.body().Visit(this);

  insns.push_back(Instruction(Opcode::jump_unconditional, startLabel));
  insns.push_back(endLabel);
}

void IR::VisitFunctionCallExpr(const FunctionCall& call) {
  if (!(call.arguments().empty())) {
    for (auto arg = call.arguments().begin(); arg != call.arguments().end();
         ++arg) {
      (*arg)->Visit(this);
      auto tmp_arg = arg_stack.back();
      arg_stack.pop_back();
      insns.push_back(Instruction(Opcode::arg, tmp_arg));
    }
  }
  arg_stack.push_back(Operand(call.callee_name(), OperandType::Function));
}

void IR::VisitFunctionDefExpr(const FunctionDef& def) {
  def.function_body().Visit(this);
  def.retval().Visit(this);
  auto tmp_arg = arg_stack.back();
  arg_stack.pop_back();
  insns.push_back(Instruction(Opcode::ret, tmp_arg));
}

void IR::VisitProgramExpr(const Program& program) {
  for (const auto& fnDef : program.function_defs()) {
    fnDef->Visit(this);

    auto bb = getBB(insns);
    program_blocks[fnDef->function_name()] = bb;

    insns.clear();
    arg_stack.clear();
  }

  // remaining statements treated as one function def
  insns.clear();
  arg_stack.clear();

  program.statements().Visit(this);
  program.arithmetic_exp().Visit(this);

  auto retval = arg_stack.back();
  arg_stack.pop_back();

  insns.push_back(Instruction(Opcode::output, retval));

  auto bb = getBB(insns);
  program_blocks["global"] = bb;
}

std::vector<int> IR::getLeaders(std::vector<Instruction> insns) {
  std::vector<int> leaders;
  assert(!(insns.empty()));
  assert(leaders.empty());

  leaders.push_back(0);

  for (std::size_t i = 1; i < insns.size(); ++i) {
    if (insns[i].isLabel()) {
      if (leaders.back() != i) {  // label follows jump
        leaders.push_back(i);
      }
    }
    if (insns[i].getOpcode() == Opcode::jump_conditional ||
        insns[i].getOpcode() == Opcode::jump_unconditional) {
      if (leaders.back() != i) {  // jump follows label
        leaders.push_back(i + 1);
      }
    }
  }

  return leaders;
}

std::vector<BasicBlock> IR::getBB(std::vector<Instruction> insns) {
  std::vector<BasicBlock> basic_blocks;
  auto leaders = getLeaders(insns);
  for (auto it = leaders.begin(); it != leaders.end(); ++it) {
    std::vector<Instruction> instr;
    std::set<int> succ;
    uint32_t beginning = *it;
    uint32_t end;
    if (std::next(it) != leaders.end()) {
      end = (*std::next(it) - 1);
    } else {
      end = insns.size() - 1;
    }
    // one line long blocks
    for (std::size_t i = beginning; i <= end; ++i) {
      instr.push_back(insns[i]);
    }

    // // successor sets
    // jumps
    auto ins = insns[end];
    if (ins.getOpcode() == Opcode::jump_unconditional ||
        ins.getOpcode() == Opcode::jump_conditional) {
      auto jump_edge = ins.getJumpTarget();

      for (auto lit = leaders.begin(); lit != leaders.end(); ++lit) {
        if (insns[*lit].getLabel() &&
            insns[*lit].getLabel().value().toString() == jump_edge.toString()) {
          auto leaders_idx = std::distance(leaders.begin(), lit);
          succ.insert(leaders_idx);
        }
      }
    }
    // everything else
    if (ins.getOpcode() != Opcode::jump_unconditional) {
      if (std::next(it) != leaders.end()) {
        auto leaders_idx = std::distance(leaders.begin(), std::next(it));
        succ.insert(leaders_idx);
      }
    }
    basic_blocks.push_back(BasicBlock(
        instr, succ, *it));  // blockID is the number of the instruction
                             // overall, just needs to be unique
  }

  // predecessors
  for (auto b = basic_blocks.begin(); b != basic_blocks.end(); ++b) {
    auto curr_idx = std::distance(basic_blocks.begin(), b);
    auto curr_succ = b->getSuccessors();
    for (auto s = curr_succ.begin(); s != curr_succ.end(); ++s) {
      basic_blocks[*s].insertPredecessor(curr_idx);
    }
  }

  return basic_blocks;
}

std::pair<std::vector<bool>, std::vector<bool>> CFG::computeGenKill(
    BasicBlock block) {
  std::vector<bool> gen(allExprs.size(), false);
  std::vector<bool> kill(allExprs.size(), false);
  auto instrs = block.instructions();

  // gen
  for (auto instr = instrs.cbegin(); instr != instrs.cend(); ++instr) {
    if (instr->isBinary()) {
      auto lhs = instr->getOperand0();
      auto rhs1 = instr->getOperand1();
      auto rhs2 = instr->getOperand2();

      // no immediate conflict, try next instruction
      if (lhs.toString() != rhs1.toString() ||
          lhs.toString() != rhs2.toString()) {
        auto next_instr = std::next(instr);
        if (next_instr == instrs.cend()) {
          // we made it to the end, we can relax
          std::string s = instr->toString();
          std::string delimiter = "<-";
          auto pos = s.find(delimiter);
          std::string expr;
          if (pos != std::string::npos) {
            expr = s.substr(pos + 3, s.length());
            gen[legend[expr]] = true;
          }
        }

        for (auto remainder = next_instr; remainder != instrs.cend();
             ++remainder) {
          auto nxt = (remainder->getOperand0()).toString();
          if (nxt == rhs1.toString() || nxt == rhs1.toString()) {
            break;
          } else if (std::next(remainder) == instrs.cend()) {
            // we made it to the end, we can relax
            std::string s = instr->toString();
            std::string delimiter = "<-";
            auto pos = s.find(delimiter);
            std::string expr;
            if (pos != std::string::npos) {
              expr = s.substr(pos + 3, s.length());
              gen[legend[expr]] = true;
            }
          }
        }
      }
    }
  }

  // kill
  for (auto instr = instrs.cbegin(); instr != instrs.cend(); ++instr) {
    if (instr->isUnary() || instr->isBinary() || instr->isSSA()) {
      auto lhs = (instr->getOperand0()).toString();
      for (auto l = legend.cbegin(); l != legend.cend(); ++l) {
        if ((l->first).find(lhs) != std::string::npos) {
          kill[l->second] = true;
        }
      }
    }
  }

  return std::make_pair(gen, kill);
}

std::vector<std::pair<std::vector<bool>, std::vector<bool>>>
CFG::getAllGenKill() {
  std::vector<std::pair<std::vector<bool>, std::vector<bool>>> genkill;
  for (auto block = basic_blocks.cbegin(); block != basic_blocks.cend();
       ++block) {
    auto genkill_pair = computeGenKill(*block);
    genkill.push_back(genkill_pair);
  }
  return genkill;
}

void CFG::getAllExpressions() {
  std::map<std::string, int> map_legend;
  int ctr = 0;
  for (auto block = basic_blocks.cbegin(); block != basic_blocks.cend();
       ++block) {
    for (auto instr = block->instructions().cbegin();
         instr != block->instructions().cend(); ++instr) {
      // we only care about binary expressions
      if (instr->isBinary()) {
        std::string s = instr->toString();
        std::string delimiter = "<-";
        auto pos = s.find(delimiter);
        std::string expr;
        if (pos != std::string::npos) {
          expr = s.substr(pos + 3, s.length());
          if (map_legend.find(expr) == map_legend.end()) {
            map_legend[expr] = ctr;
            ++ctr;
          }
        }
      }
    }
  }
  std::vector<bool> E(map_legend.size(), false);
  legend = map_legend;
  allExprs = E;
}



std::vector<bool> intersect(std::vector<bool> a, std::vector<bool> b) {
  if (a.size() != b.size()) {
    std::cout << "vectors not same size" << std::endl;
  }
  std::vector<bool> output;
  for (int i = 0; i < a.size(); i++) {
    bool out = a.at(i) & b.at(i);
    output.push_back(out);
  }
  return output;
}

std::vector<bool> unionop(std::vector<bool> a, std::vector<bool> b) {
  if (a.size() != b.size()) {
    std::cout << "vectors not same size" << std::endl;
  }
  std::vector<bool> output;
  for (int i = 0; i < a.size(); i++) {
    bool out = a.at(i) | b.at(i);
    output.push_back(out);
  }
  return output;
}

std::vector<bool> subtract(std::vector<bool> a, std::vector<bool> b) {
  if (a.size() != b.size()) {
    std::cout << "vectors not same size" << std::endl;
  }
  for (int i = 0; i < a.size(); i++) {
    if(a.at(i) & b.at(i)) {
      a.at(i) = false;
    }
  }
  return a;
}

bool checkvectorbool(std::vector<bool> a, std::vector<bool> b) {
  for (int i = 0; i < a.size(); i++) {
    if (a != b) {
      return false;
    }
  }
  return true;
}


void CFG::runWorklisthelper(int blocknumber, const std::vector<std::pair<std::vector<bool>, std::vector<bool>>>&
        genkill) {
          std::cout << "now you see me" << blocknumber << std::endl;
          std::vector<bool> tempout = availableExpressions.at(blocknumber).second;
          std::set<int> pre = basic_blocks.at(blocknumber).getPredecessors();
          std::set<int>::iterator it;
          for (it = pre.begin(); it != pre.end(); ++it) {
              int f = *it; // Note the "*" here
              availableExpressions.at(blocknumber).first = intersect(availableExpressions.at(blocknumber).first,availableExpressions.at(f).second);
          }
          auto temp = subtract(availableExpressions.at(blocknumber).first,genkill.at(blocknumber).second);
          temp = unionop(temp,genkill.at(blocknumber).first);
          availableExpressions.at(blocknumber).second = temp;
          if (!checkvectorbool(tempout,availableExpressions.at(blocknumber).second)) {
            pre = basic_blocks.at(blocknumber).getSuccessors();
            for (it = pre.begin(); it != pre.end(); ++it) {
              int f = *it; // Note the "*" here
              runWorklisthelper(f,genkill);
            }
          }
          std::cout << "I'm a happy man" << std::endl;

}

// writes to availableExpressions
void CFG::runWorklist(
    const std::vector<std::pair<std::vector<bool>, std::vector<bool>>>&
        genkill) {
          int blocknumber = 0;
          runWorklisthelper(blocknumber, genkill);
          
          
  // fill me in
}

Instruction makeinstr(int index, Instruction parent) {
  std::string var = "_opt" + std::to_string(index);
  Instruction a(Operand(var, OperandType::Var), parent.getOperand0());
  return a;
}

void BasicBlock::setinstruction(int index, Instruction a) {
  instructions_.at(index) = a;
}


void BasicBlock::addstatement(int index, Instruction input) {
  instructions_.insert(instructions_.begin() + index,input);
}

bool CFG::checkifkilled(int blocknumber, int index, Instruction available) {
  std::cout << "Enter crazy" << std::endl;
  for (int i = 0; i < index-1; i++) {
    std::cout << "checking if i'm dead" << std::endl;
    if (available.isUnary() || available.isBinary()) {
      if (optimized_program.at(blocknumber).instructions().at(i).getOperand0().toString().compare(available.getOperand1().toString()) == 0) {
        return true;
      }
    }
    if (available.isBinary()) {
      if (optimized_program.at(blocknumber).instructions().at(i).getOperand0().toString().compare(available.getOperand2().toString()) == 0) {
        return true;
      }
    }
  }
  return false;
}

void CFG::optimize(int blocknumber, int index, int available) {
  if (availableExpressions.at(blocknumber).first.at(available) == false) {
    return;
  }
  if (checkifkilled(blocknumber,index,optimized_program.at(blocknumber).instructions().at(index))) {
    return;
  }
  std::cout << "Trace" << std::endl;
  std::string var = "_opt" + std::to_string(index);
  Instruction a(optimized_program.at(blocknumber).instructions().at(index).getOperand0(),Operand(var, OperandType::Var));
  optimized_program.at(blocknumber).setinstruction(index,a);
}



void CFG::GCSEhelper(int blocknumber) {
  int index = 0;
  std::cout << "begin outter loop" << std::endl;
  for (int j = 0; j < basic_blocks.at(blocknumber).instructions().size(); j++) {
    std::cout << "being inner" << std::endl;
    std::string compare = "";
    if (basic_blocks.at(blocknumber).instructions().at(j).isUnary() || basic_blocks.at(blocknumber).instructions().at(j).isBinary()) {
      compare = compare + basic_blocks.at(blocknumber).instructions().at(j).getOperand1().toString();
    }
    compare = compare + " " + OpcodeToString(basic_blocks.at(blocknumber).instructions().at(j).getOpcode());
    if (basic_blocks.at(blocknumber).instructions().at(j).isBinary()) {
      compare = compare + " " + basic_blocks.at(blocknumber).instructions().at(j).getOperand2().toString();
    }
    if (legend.find(compare) != legend.end()) {
      std::cout << "Before optimize" << std::endl;
      optimize(blocknumber,index,legend.find(compare)->second);
      std::cout << "go on" << std::endl;
      index++;
      Instruction a = makeinstr(legend.find(compare)->second,basic_blocks.at(blocknumber).instructions().at(j));
      optimized_program.at(blocknumber).addstatement(index,a);
      std::cout << "after optimize" << std::endl;
    }
    index++;
    std::cout << "end inner" << std::endl;
  }
  std::cout << "end outter loop" << std::endl;
}




std::vector<BasicBlock> CFG::computeGCSE(
    const std::vector<std::pair<std::vector<bool>, std::vector<bool>>>&
        genkill) {
          std::cout << "I'm in now!" << std::endl;
          optimized_program = basic_blocks;
          for (int i = 0; i < basic_blocks.size(); i++) {
              GCSEhelper(i);
          }
          return optimized_program;
  // fill me in
}


void CFG::resetAvailExprs(const std::vector<std::pair<std::vector<bool>, std::vector<bool>>>& genkill) {
  //assume all vector sizes internally are same
  int size = genkill.size();
  availableExpressions = genkill;
  std::fill(availableExpressions.at(0).first.begin(), availableExpressions.at(0).first.end(), false);
  std::fill(availableExpressions.at(0).second.begin(), availableExpressions.at(0).second.end(), true);
  for (int i = 1; i < size; i++) {
    std::fill(availableExpressions.at(i).first.begin(), availableExpressions.at(i).first.end(), true);
    std::fill(availableExpressions.at(i).second.begin(), availableExpressions.at(i).second.end(), true);
  }
}

void CFG::computeAvailExprs() {
  std::cout << "I'm in 2" << std::endl;
  getAllExpressions();
  auto genkill = getAllGenKill();
  resetAvailExprs(genkill);
  runWorklist(genkill);
}

}  // namespace cs160::midend
 