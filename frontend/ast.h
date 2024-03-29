#pragma once

// This grammar for the abstract syntax is:
//
// n ∈ Integer
// id ∈ Variable
//
// ae ∈ ArithmeticExp ::= n | id | ae1 aop ae2
// aop ∈ BinaryArithmeticOperator ::= + | - | ⨯
//
// re ∈ RelationalExp ::= ae1 rop ae2 | re1 lop re2 | !re
// rop ∈ BinaryRelationalOperator ::= < | <= | =
// lop ∈ BinaryLogicalOperator ::= && | ||
//
// decl ∈ Declaration ::= type id
// type ∈ Type ::= int
//
// stmt ∈ Statement ::= assign | cond | loop
// block ∈ Block ::= decl... stmt...
//
// assign ∈ Assignment ::= id := ae | call
// cond ∈ Conditional ::= if re block1 block2
// loop ∈ Loop ::= while re block
// call ∈ FunctionCall ::= id(args...)
//
// fundef ∈ FunctionDef ::= def id(params...) type block ae
//
// prog ∈ Program ::= fundef... block ae
//
// where ... means 'repeating', i.e., a block is a (possibly empty) sequence of
// declarations followed by statements, args is a vector of exprs, params is,
// say, a vector of pairs, a function call takes a (possibly empty)
// sequence of arithmetic expressions as arguments, and a function definition
// has a (possibly empty) sequence of variables as parameters.

#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <variant>
#include <vector>

namespace cs160::frontend {

template <class T>
using vec_of_ptrs = std::vector<std::unique_ptr<T>>;

template <class T>
bool pointeesEqual(const vec_of_ptrs<const T>& lhs,
                   const vec_of_ptrs<const T>& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  for (auto lIt = lhs.begin(), rIt = rhs.begin(); lIt != lhs.end();
       ++lIt, ++rIt) {
    // check whether the objects the elements point to are equal
    if (**lIt != **rIt) {
      return false;
    }
  }
  return true;
}

// forward declaration
class AstVisitor;

// The definition of the abstract syntax tree abstract base class.
class AstNode {
 public:
  virtual ~AstNode() {}

  std::string toString() const;

  virtual void Visit(AstVisitor* visitor) const = 0;
};

// This is the abstract base class from which all arithmetic expressions will
// inherit (integers, variables, and binary arithmetic operations) as well as
// the humble function call.
class RhsExpr : public AstNode {};

class ArithmeticExpr : public RhsExpr {};

// An integer constant expression.
class IntegerExpr final : public ArithmeticExpr {
 public:
  explicit IntegerExpr(int value) : value_(value) {}

  void Visit(AstVisitor* visitor) const override;

  int value() const { return value_; }

 private:
  // The value of the integer constant.
  int value_;
};

// A program variable expression.
class VariableExpr final : public ArithmeticExpr {
 public:
  explicit VariableExpr(const std::string& name) : name_(name) {}

  void Visit(AstVisitor* visitor) const override;

  const std::string& name() const { return name_; }

 private:
  // The name of the variable.
  std::string name_;
};

// An abstract arithmetic binary operator node.
class ArithmeticBinaryOperatorExpr : public ArithmeticExpr {
 public:
  ArithmeticBinaryOperatorExpr(std::unique_ptr<const ArithmeticExpr> lhs,
                               std::unique_ptr<const ArithmeticExpr> rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  const ArithmeticExpr& lhs() const { return *lhs_; }
  const ArithmeticExpr& rhs() const { return *rhs_; }

 protected:
  // The left-hand side and right-hand side of the expression.
  const std::unique_ptr<const ArithmeticExpr> lhs_;
  const std::unique_ptr<const ArithmeticExpr> rhs_;
};

// An addition expression.
class AddExpr final : public ArithmeticBinaryOperatorExpr {
 public:
  AddExpr(std::unique_ptr<const ArithmeticExpr> lhs,
          std::unique_ptr<const ArithmeticExpr> rhs)
      : ArithmeticBinaryOperatorExpr(std::move(lhs), std::move(rhs)) {}

  void Visit(AstVisitor* visitor) const override;
};

// A subtraction expression.
class SubtractExpr final : public ArithmeticBinaryOperatorExpr {
 public:
  SubtractExpr(std::unique_ptr<const ArithmeticExpr> lhs,
               std::unique_ptr<const ArithmeticExpr> rhs)
      : ArithmeticBinaryOperatorExpr(std::move(lhs), std::move(rhs)) {}

  void Visit(AstVisitor* visitor) const override;
};

// A multiplication expression.
class MultiplyExpr final : public ArithmeticBinaryOperatorExpr {
 public:
  MultiplyExpr(std::unique_ptr<const ArithmeticExpr> lhs,
               std::unique_ptr<const ArithmeticExpr> rhs)
      : ArithmeticBinaryOperatorExpr(std::move(lhs), std::move(rhs)) {}

  void Visit(AstVisitor* visitor) const override;
};

// This is the abstract base class from which all relational expressions will
// inherit (relational and logical operations).
class RelationalExpr : public AstNode {};

// An abstract relational binary operator node (<, <=, =).
class RelationalBinaryOperator : public RelationalExpr {
 public:
  RelationalBinaryOperator(std::unique_ptr<const ArithmeticExpr> lhs,
                           std::unique_ptr<const ArithmeticExpr> rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  const ArithmeticExpr& lhs() const { return *lhs_; }
  const ArithmeticExpr& rhs() const { return *rhs_; }

 protected:
  // The left-hand side and right-hand side of the expression.
  const std::unique_ptr<const ArithmeticExpr> lhs_;
  const std::unique_ptr<const ArithmeticExpr> rhs_;
};

// A less-than relational expression.
class LessThanExpr final : public RelationalBinaryOperator {
 public:
  LessThanExpr(std::unique_ptr<const ArithmeticExpr> lhs,
               std::unique_ptr<const ArithmeticExpr> rhs)
      : RelationalBinaryOperator(std::move(lhs), std::move(rhs)) {}

  void Visit(AstVisitor* visitor) const override;
};

// A less-than-or-equal-to relational expression.
class LessThanEqualToExpr final : public RelationalBinaryOperator {
 public:
  LessThanEqualToExpr(std::unique_ptr<const ArithmeticExpr> lhs,
                      std::unique_ptr<const ArithmeticExpr> rhs)
      : RelationalBinaryOperator(std::move(lhs), std::move(rhs)) {}

  void Visit(AstVisitor* visitor) const override;
};

// An equal-to relational expression.
class EqualToExpr final : public RelationalBinaryOperator {
 public:
  EqualToExpr(std::unique_ptr<const ArithmeticExpr> lhs,
              std::unique_ptr<const ArithmeticExpr> rhs)
      : RelationalBinaryOperator(std::move(lhs), std::move(rhs)) {}

  void Visit(AstVisitor* visitor) const override;
};

// An abstract logical binary operator node (&&, ||).
class LogicalBinaryOperator : public RelationalExpr {
 public:
  LogicalBinaryOperator(std::unique_ptr<const RelationalExpr> lhs,
                        std::unique_ptr<const RelationalExpr> rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  const RelationalExpr& lhs() const { return *lhs_; }
  const RelationalExpr& rhs() const { return *rhs_; }

 protected:
  // The left-hand side and right-hand side of the expression.
  const std::unique_ptr<const RelationalExpr> lhs_;
  const std::unique_ptr<const RelationalExpr> rhs_;
};

// a logical-and expression.
class LogicalAndExpr final : public LogicalBinaryOperator {
 public:
  LogicalAndExpr(std::unique_ptr<const RelationalExpr> lhs,
                 std::unique_ptr<const RelationalExpr> rhs)
      : LogicalBinaryOperator(std::move(lhs), std::move(rhs)) {}

  void Visit(AstVisitor* visitor) const override;
};

// a logical-or expression.
class LogicalOrExpr final : public LogicalBinaryOperator {
 public:
  LogicalOrExpr(std::unique_ptr<const RelationalExpr> lhs,
                std::unique_ptr<const RelationalExpr> rhs)
      : LogicalBinaryOperator(std::move(lhs), std::move(rhs)) {}

  void Visit(AstVisitor* visitor) const override;
};

// A logical negation expression.
class LogicalNotExpr final : public RelationalExpr {
 public:
  explicit LogicalNotExpr(std::unique_ptr<const RelationalExpr> operand)
      : operand_(std::move(operand)) {}

  void Visit(AstVisitor* visitor) const override;

  const RelationalExpr& operand() const { return *operand_; }

 private:
  // The expression being negated.
  const std::unique_ptr<const RelationalExpr> operand_;
};

// we might add more types in the future
class TypeExpr : public AstNode {
 public:
  std::string value_;
};

class IntType final : public TypeExpr {
 public:
  explicit IntType() : value_("int") {}

  void Visit(AstVisitor* visitor) const override;

  std::string value() const { return value_; }

 private:
  // string repr of type
  std::string value_;
};

// A statement can be an assignment, a conditional, a loop
class Statement : public AstNode {
 public:
  // A block is a (possibly empty) sequence of statements.
  using Block = std::vector<std::unique_ptr<const Statement>>;
};

// An assignment: id := ae.
class Assignment final : public Statement {
 public:
  Assignment(std::unique_ptr<const VariableExpr> lhs,
             std::unique_ptr<const RhsExpr> rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  const VariableExpr& lhs() const { return *lhs_; }
  const RhsExpr& rhs() const { return *rhs_; }

  void Visit(AstVisitor* visitor) const override;

 private:
  // The left-hand side and right-hand side of the assignment.
  const std::unique_ptr<const VariableExpr> lhs_;
  const std::unique_ptr<const RhsExpr> rhs_;
};

class Declaration final : public AstNode {
 public:
  using Block = std::vector<std::unique_ptr<const Declaration>>;

  Declaration(std::unique_ptr<const TypeExpr> type,
              std::unique_ptr<const VariableExpr> id)
      : type_(std::move(type)), id_(std::move(id)) {}

  const TypeExpr& type() const { return *type_; }
  const VariableExpr& id() const { return *id_; }

  void Visit(AstVisitor* visitor) const override;

 private:
  // The type and id of the declaration
  const std::unique_ptr<const TypeExpr> type_;
  const std::unique_ptr<const VariableExpr> id_;
};

// block node for both decls and stmts
class BlockExpr final : public AstNode {
 public:
  BlockExpr(std::vector<std::unique_ptr<const Declaration>> decls,
            std::vector<std::unique_ptr<const Statement>> stmts)
      : decls_(std::move(decls)), stmts_(std::move(stmts)) {}

  const std::vector<std::unique_ptr<const Declaration>>& decls() const {
    return decls_;
  }
  const std::vector<std::unique_ptr<const Statement>>& stmts() const {
    return stmts_;
  }

  void Visit(AstVisitor* visitor) const override;

 private:
  const std::vector<std::unique_ptr<const Declaration>> decls_;
  const std::vector<std::unique_ptr<const Statement>> stmts_;
};

// A conditional statement: if re block1 block2
class Conditional final : public Statement {
 public:
  Conditional(std::unique_ptr<const RelationalExpr> guard,
              std::unique_ptr<const BlockExpr> true_branch,
              std::unique_ptr<const BlockExpr> false_branch)
      : guard_(std::move(guard)),
        true_branch_(std::move(true_branch)),
        false_branch_(std::move(false_branch)) {}

  void Visit(AstVisitor* visitor) const override;

  const RelationalExpr& guard() const { return *guard_; }

  const BlockExpr& true_branch() const { return *true_branch_; }
  const BlockExpr& false_branch() const { return *false_branch_; }

 private:
  // The guard expression of the conditional.
  std::unique_ptr<const RelationalExpr> guard_;

  // The true and false branches of the conditional.
  std::unique_ptr<const BlockExpr> true_branch_;
  std::unique_ptr<const BlockExpr> false_branch_;
};

// A loop statement: while re block
class Loop final : public Statement {
 public:
  Loop(std::unique_ptr<const RelationalExpr> guard,
       std::unique_ptr<const BlockExpr> body)
      : guard_(std::move(guard)), body_(std::move(body)) {}

  const RelationalExpr& guard() const { return *guard_; }

  const BlockExpr& body() const { return *body_; }

  void Visit(AstVisitor* visitor) const override;

 private:
  // The guard expression of the loop.
  std::unique_ptr<const RelationalExpr> guard_;

  // The body of the loop.
  std::unique_ptr<const BlockExpr> body_;
};

// A function definition: def id(v...) type block ae. The 'v...' are the
// parameters of the function; the 'ae' at the end of the function body is the
// return value of the function. "type" is the return type
class FunctionDef final : public AstNode {
 public:
  // A block is a (possibly empty) sequence of function definitions.
  using Block = std::vector<std::unique_ptr<const FunctionDef>>;

  // A name is a function identifier; it is assumed to be unique.
  using Name = std::string;

  // originally third param was just std::vector<std::unique_ptr<const
  // VariableExpr>> parameters,
  // type expr and arithexpr had two ampersands after them...
  // e.g. std::unique_ptr<ArithmeticExpr>&& retval)
  FunctionDef(
      const Name& function_name, std::unique_ptr<const TypeExpr> type,
      // std::vector<std::unique_ptr<const Declaration>> parameters, // todo
      std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                            std::unique_ptr<const VariableExpr>>>
          parameters,
      std::unique_ptr<const BlockExpr> function_body,
      std::unique_ptr<const ArithmeticExpr> retval)
      : function_name_(function_name),
        parameters_(std::move(parameters)),
        type_(std::move(type)),
        function_body_(std::move(function_body)),
        retval_(std::move(retval)) {}

  const Name& function_name() const { return function_name_; }

  /* const std::vector<std::unique_ptr<const Declaration>>& parameters() const {
   */
  /*   return parameters_; */
  /* } */
  const std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                              std::unique_ptr<const VariableExpr>>>&
  parameters() const {
    return parameters_;
  }

  const TypeExpr& type() const { return *type_; }

  const BlockExpr& function_body() const { return *function_body_; }

  const ArithmeticExpr& retval() const { return *retval_; }

  void Visit(AstVisitor* visitor) const override;

 private:
  // The name of the function being defined.
  Name function_name_;

  // The parameters of the function being defined.
  std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                        std::unique_ptr<const VariableExpr>>>
      parameters_;

  // return type
  std::unique_ptr<const TypeExpr> type_;

  // The body of the function being defined.
  std::unique_ptr<const BlockExpr> function_body_;

  // The return value of the function being defined.
  std::unique_ptr<const ArithmeticExpr> retval_;
};

// A function call: id(ae...)
class FunctionCall final : public RhsExpr {
 public:
  FunctionCall(const FunctionDef::Name& callee_name,
               std::vector<std::unique_ptr<const ArithmeticExpr>> arguments)
      : callee_name_(callee_name), arguments_(std::move(arguments)) {}

  const FunctionDef::Name& callee_name() const { return callee_name_; }

  const std::vector<std::unique_ptr<const ArithmeticExpr>>& arguments() const {
    return arguments_;
  }

  void Visit(AstVisitor* visitor) const override;

 private:
  // The name of the function being called, "id"
  FunctionDef::Name callee_name_;

  // The arguments to the function being called.
  std::vector<std::unique_ptr<const ArithmeticExpr>> arguments_;
};

class Program final : public AstNode {
 public:
  Program(FunctionDef::Block function_defs,
          std::unique_ptr<const BlockExpr> statements,
          std::unique_ptr<const ArithmeticExpr> arithmetic_exp)
      : function_defs_(std::move(function_defs)),
        statements_(std::move(statements)),
        arithmetic_exp_(std::move(arithmetic_exp)) {}

  const FunctionDef::Block& function_defs() const { return function_defs_; }

  const BlockExpr& statements() const { return *statements_; }

  const ArithmeticExpr& arithmetic_exp() const { return *arithmetic_exp_; }

  void Visit(AstVisitor* visitor) const override;

 private:
  FunctionDef::Block function_defs_;
  std::unique_ptr<const BlockExpr> statements_;
  std::unique_ptr<const ArithmeticExpr> arithmetic_exp_;
};

inline std::ostream& operator<<(std::ostream& out, const AstNode& node) {
  return out << node.toString();
}

// just a bunch of aliases
using ProgramExprP = std::unique_ptr<const Program>;
using FunctionDefP = std::unique_ptr<const FunctionDef>;
using FunctionCallP = std::unique_ptr<const FunctionCall>;
using StatementP = std::unique_ptr<const Statement>;
using ArithmeticExprP = std::unique_ptr<const ArithmeticExpr>;
using ArithmeticBinaryOpExprP =
    std::unique_ptr<const ArithmeticBinaryOperatorExpr>;
using RelationalExprP = std::unique_ptr<const RelationalExpr>;
using RelationalBinaryOpExprP = std::unique_ptr<const RelationalBinaryOperator>;
using LogicalBinaryOpExprP = std::unique_ptr<const LogicalBinaryOperator>;
using VariableExprP = std::unique_ptr<const VariableExpr>;
using IntegerExprP = std::unique_ptr<const IntegerExpr>;
using AddExprP = std::unique_ptr<const AddExpr>;
using MultiplyExprP = std::unique_ptr<const MultiplyExpr>;
using SubtractExprP = std::unique_ptr<const SubtractExpr>;
using LessThanExprP = std::unique_ptr<const LessThanExpr>;
using LessThanEqualToP = std::unique_ptr<const LessThanEqualToExpr>;
using EqualToExprP = std::unique_ptr<const EqualToExpr>;
using LogicalAndExprP = std::unique_ptr<const LogicalAndExpr>;
using LogicalOrExprP = std::unique_ptr<const LogicalOrExpr>;
using LogicalNotExprP = std::unique_ptr<const LogicalNotExpr>;
using AssignmentExprP = std::unique_ptr<const Assignment>;
using ConditionalExprP = std::unique_ptr<const Conditional>;
using LoopExprP = std::unique_ptr<const Loop>;
using DeclarationExprP = std::unique_ptr<const Declaration>;
using BlockExprP = std::unique_ptr<const BlockExpr>;

}  // namespace cs160::frontend
