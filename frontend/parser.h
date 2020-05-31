#pragma once
#include <optional>
#include <stdexcept>
#include <vector>
#include "frontend/ast.h"
#include "frontend/token.h"

namespace cs160::frontend {

// This is meant as a general error
struct InvalidASTError : public std::runtime_error {
  InvalidASTError() : runtime_error("Invalid AST created") {}
};

// This is the parser class you need to implement. The function you need to
// implement is the parsify() method. You can define other class members such as
// fields or helper functions.
class Parser final {
 public:
  // The entry point of the parser you need to implement. It takes the output of
  // the lexer as the argument and produces an abstract syntax tree as the
  // result of parsing the tokens.
  Parser(const std::vector<Token> &lexer_tokens) : tokens(lexer_tokens) {}

  std::optional<Token> nextToken(int peek = 1);
  void matchToken(const TokenType &);

  VariableExprP parseVariableExpr();
  IntegerExprP parseIntegerExpr();

  ArithmeticExprP parseArithmeticExpr();
  std::pair<ArithmeticExprP, std::optional<Token>> parseAExpPrime();
  ArithmeticExprP parseATerm();
  ArithmeticExprP parseATermPrime();
  ArithmeticExprP parseAFactor();

  RelationalExprP parseRexp();
  RelationalExprP parseRexpPrime1();
  RelationalExprP parseRFactor();
  RelationalExprP parseCexp();
  std::pair<RelationalExprP, std::optional<Token>> parseRexpPrime2();

  DeclarationExprP parseDeclarationExprP();
  Declaration::Block parseDecls();

  StatementP parseStatementP();
  LoopExprP parseLoopExprP();
  ConditionalExprP parseCondExprP();
  AssignmentExprP parseAssignmentExprP();
  Statement::Block parseStmts();
  BlockExprP parseBlockExpr();

  std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                        std::unique_ptr<const VariableExpr>>>
  parseParams();
  std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                        std::unique_ptr<const VariableExpr>>>
  parseOptParams();

  std::vector<ArithmeticExprP> parseFunArgs();
  FunctionDefP parseFunDef();
  FunctionDef::Block parseFunDefs();

  FunctionCallP parseFunCall();

  ProgramExprP parse();

 private:
  const std::vector<Token> tokens;
  int head = -1;  // we start at one before the token list
};
};  // namespace cs160::frontend
