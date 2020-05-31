#include "frontend/parser.h"
#include <iostream>

namespace cs160::frontend {

void Parser::matchToken(const TokenType& tok) {
  if (nextToken() && nextToken().value().type() == tok) {
    head++;
  } else {
    // std::cout << "match token " << tokens[head++] << tokenTypeToString(tok)
    // << std::endl;
    throw InvalidASTError();
  }
}

std::optional<cs160::frontend::Token> Parser::nextToken(int peek) {
  // std::cout << "nextToken " << std::endl;
  if (head == tokens.size() - 1) {
    // std::cout << "end" << std::endl;
    return std::nullopt;
  } else {
    // std::cout << tokens[head + peek] << std::endl;
    return tokens[head + peek];
  }
}

IntegerExprP Parser::parseIntegerExpr() {
  matchToken(TokenType::Num);
  return std::make_unique<const IntegerExpr>(tokens[head].intValue());
}

VariableExprP Parser::parseVariableExpr() {
  matchToken(TokenType::Id);
  return std::make_unique<const VariableExpr>(tokens[head].stringValue());
}

ArithmeticExprP Parser::parseAFactor() {
  if (nextToken() && nextToken().value().type() == TokenType::LParen) {
    matchToken(TokenType::LParen);
    auto ae = parseArithmeticExpr();
    matchToken(TokenType::RParen);
    return ae;
  } else if (nextToken() && nextToken().value().type() == TokenType::Num) {
    return parseIntegerExpr();
  } else if (nextToken() && nextToken().value().type() == TokenType::Id) {
    return parseVariableExpr();
  }
  throw InvalidASTError();
}

ArithmeticExprP Parser::parseATermPrime() {
  if (nextToken() &&
      nextToken().value() == Token::makeArithOp(ArithOp::Times)) {
    matchToken(Token::makeArithOp(ArithOp::Times).type());

    auto l = parseAFactor();
    auto p = parseATermPrime();
    if (p) {
      return std::make_unique<const MultiplyExpr>(std::move(l), std::move(p));
    } else {
      return l;
    }
  } else {
    return nullptr;
  }
}

ArithmeticExprP Parser::parseATerm() {
  auto l = parseAFactor();
  auto p = parseATermPrime();
  if (p) {
    return std::make_unique<const MultiplyExpr>(std::move(l), std::move(p));
  } else {
    return l;
  }
}

std::pair<ArithmeticExprP, std::optional<Token>> Parser::parseAExpPrime() {
  if (nextToken() && nextToken().value() == Token::makeArithOp(ArithOp::Plus)) {
    matchToken(Token::makeArithOp(ArithOp::Plus).type());
    auto l = parseATerm();
    auto p = parseAExpPrime();
    if (p.first && p.second.value() == Token::makeArithOp(ArithOp::Plus)) {
      return std::make_pair(
          std::make_unique<const AddExpr>(std::move(l), std::move(p.first)),
          Token::makeArithOp(ArithOp::Plus));
    } else if (p.first &&
               p.second.value() == Token::makeArithOp(ArithOp::Minus)) {
      return std::make_pair(std::make_unique<const SubtractExpr>(
                                std::move(l), std::move(p.first)),
                            Token::makeArithOp(ArithOp::Plus));

    } else {
      return std::make_pair(std::move(l), Token::makeArithOp(ArithOp::Plus));
    }
  }

  else if (nextToken() &&
           nextToken().value() == Token::makeArithOp(ArithOp::Minus)) {
    matchToken(Token::makeArithOp(ArithOp::Minus).type());
    auto l = parseATerm();
    auto p = parseAExpPrime();
    if (p.first && p.second.value() == Token::makeArithOp(ArithOp::Plus)) {
      return std::make_pair(
          std::make_unique<const AddExpr>(std::move(l), std::move(p.first)),
          Token::makeArithOp(ArithOp::Minus));
    } else if (p.first &&
               p.second.value() == Token::makeArithOp(ArithOp::Minus)) {
      return std::make_pair(std::make_unique<const SubtractExpr>(
                                std::move(l), std::move(p.first)),
                            Token::makeArithOp(ArithOp::Minus));
    } else {
      return std::make_pair(std::move(l), Token::makeArithOp(ArithOp::Minus));
    }

  } else {
    return std::make_pair(nullptr, std::nullopt);
  }
}

ArithmeticExprP Parser::parseArithmeticExpr() {
  auto at = parseATerm();
  auto p = parseAExpPrime();
  if (p.second && p.second.value() == Token::makeArithOp(ArithOp::Plus)) {
    return std::make_unique<const AddExpr>(std::move(at), std::move(p.first));
  }

  else if (p.second && p.second.value() == Token::makeArithOp(ArithOp::Minus)) {
    return std::make_unique<const SubtractExpr>(std::move(at),
                                                std::move(p.first));
  }

  else {
    return at;
  }
}

RelationalExprP Parser::parseRexpPrime1() {
  if (nextToken() && nextToken().value() == Token::makeLNeg()) {
    matchToken(TokenType::LNeg);
    auto re = parseRexpPrime1();
    return std::make_unique<const LogicalNotExpr>(std::move(re));
  } else if (nextToken() && nextToken().value() == Token::makeLBracket()) {
    matchToken(TokenType::LBracket);
    auto re = parseRexp();
    matchToken(TokenType::RBracket);
    return re;
  } else {
    auto cexp = parseCexp();
    return cexp;
  }
  throw InvalidASTError();
}

RelationalExprP Parser::parseCexp() {
  auto ae1 = parseArithmeticExpr();
  if (nextToken() && nextToken().value() == Token::makeRelOp(RelOp::LessThan)) {
    matchToken(Token::makeRelOp(RelOp::LessThan).type());
    auto ae2 = parseArithmeticExpr();
    return std::make_unique<const LessThanExpr>(std::move(ae1), std::move(ae2));

  } else if (nextToken() &&
             nextToken().value() == Token::makeRelOp(RelOp::LessEq)) {
    matchToken(Token::makeRelOp(RelOp::LessEq).type());
    auto ae2 = parseArithmeticExpr();
    return std::make_unique<const LessThanEqualToExpr>(std::move(ae1),
                                                       std::move(ae2));

  } else if (nextToken() &&
             nextToken().value() == Token::makeRelOp(RelOp::Equal)) {
    matchToken(Token::makeRelOp(RelOp::Equal).type());
    auto ae2 = parseArithmeticExpr();
    return std::make_unique<const EqualToExpr>(std::move(ae1), std::move(ae2));
  }
  throw InvalidASTError();
}

std::pair<RelationalExprP, std::optional<Token>> Parser::parseRexpPrime2() {
  if (nextToken() && nextToken().value() == Token::makeLBinOp(LBinOp::And)) {
    matchToken(Token::makeLBinOp(LBinOp::And).type());
    auto l = parseRexpPrime1();
    auto r = parseRexpPrime2();
    if (r.first && r.second.value() == Token::makeLBinOp(LBinOp::And)) {
      return std::make_pair(std::make_unique<const LogicalAndExpr>(
                                std::move(l), std::move(r.first)),
                            Token::makeLBinOp(LBinOp::And));
    } else if (r.first && r.second.value() == Token::makeLBinOp(LBinOp::Or)) {
      return std::make_pair(std::make_unique<const LogicalOrExpr>(
                                std::move(l), std::move(r.first)),
                            Token::makeLBinOp(LBinOp::And));
    } else {
      return std::make_pair(std::move(l), Token::makeLBinOp(LBinOp::And));
    }
  }

  else if (nextToken() &&
           nextToken().value() == Token::makeLBinOp(LBinOp::Or)) {
    matchToken(Token::makeLBinOp(LBinOp::Or).type());
    auto l = parseRexpPrime1();
    auto r = parseRexpPrime2();
    if (r.first && r.second.value() == Token::makeLBinOp(LBinOp::And)) {
      return std::make_pair(std::make_unique<const LogicalAndExpr>(
                                std::move(l), std::move(r.first)),
                            Token::makeLBinOp(LBinOp::Or));
    } else if (r.first && r.second.value() == Token::makeLBinOp(LBinOp::Or)) {
      return std::make_pair(std::make_unique<const LogicalOrExpr>(
                                std::move(l), std::move(r.first)),
                            Token::makeLBinOp(LBinOp::Or));
    } else {
      return std::make_pair(std::move(l), Token::makeLBinOp(LBinOp::Or));
    }

  } else {
    return std::make_pair(nullptr, std::nullopt);
  }
}

RelationalExprP Parser::parseRexp() {
  auto l = parseRexpPrime1();
  auto r = parseRexpPrime2();
  if (r.second && r.second.value() == Token::makeLBinOp(LBinOp::And)) {
    return std::make_unique<const LogicalAndExpr>(std::move(l),
                                                  std::move(r.first));
  } else if (r.second && r.second.value() == Token::makeLBinOp(LBinOp::Or)) {
    return std::make_unique<const LogicalOrExpr>(std::move(l),
                                                 std::move(r.first));
  } else {
    return l;
  }
}

LoopExprP Parser::parseLoopExprP() {
  matchToken(TokenType::While);
  matchToken(TokenType::LParen);
  auto re = parseRexp();
  matchToken(TokenType::RParen);
  matchToken(TokenType::LBrace);
  auto blk = parseBlockExpr();
  matchToken(TokenType::RBrace);
  return std::make_unique<const Loop>(std::move(re), std::move(blk));
}

ConditionalExprP Parser::parseCondExprP() {
  matchToken(TokenType::If);
  matchToken(TokenType::LParen);

  auto re = parseRexp();

  matchToken(TokenType::RParen);
  matchToken(TokenType::LBrace);

  auto blk = parseBlockExpr();

  matchToken(TokenType::RBrace);

  if (nextToken() && nextToken().value().type() == TokenType::Else) {
    matchToken(TokenType::Else);
    matchToken(TokenType::LBrace);

    auto blk2 = parseBlockExpr();
    matchToken(TokenType::RBrace);

    return std::make_unique<const Conditional>(std::move(re), std::move(blk),
                                               std::move(blk2));

  } else {
    std::vector<DeclarationExprP> d;
    std::vector<StatementP> s;
    return std::make_unique<const Conditional>(
        std::move(re), std::move(blk),
        std::make_unique<const BlockExpr>(std::move(d), std::move(s)));
  }
}

AssignmentExprP Parser::parseAssignmentExprP() {
  auto id = parseVariableExpr();
  matchToken(TokenType::Assign);

  if (nextToken(1) && nextToken(2) &&
      nextToken(1).value().type() == TokenType::Id &&
      nextToken(2).value().type() == TokenType::LParen) {
    auto c = parseFunCall();
    matchToken(TokenType::Semicolon);
    return std::make_unique<const Assignment>(std::move(id), std::move(c));

  } else {
    auto ae = parseArithmeticExpr();
    matchToken(TokenType::Semicolon);
    return std::make_unique<const Assignment>(std::move(id), std::move(ae));
  }
}

DeclarationExprP Parser::parseDeclarationExprP() {
  matchToken(TokenType::Type);  // change if extra types are added
  auto t = std::make_unique<const IntType>();
  auto id = parseVariableExpr();
  matchToken(TokenType::Semicolon);
  return std::make_unique<const Declaration>(std::move(t), std::move(id));
}

Declaration::Block Parser::parseDecls() {
  // std::cout << "Parser::parseDecls" << std::endl;
  Declaration::Block ret;
  if (nextToken() && (nextToken().value().type() == TokenType::Type)) {
    auto s = parseDeclarationExprP();
    auto v = parseDecls();
    v.insert(v.begin(), std::move(s));
    ret.insert(std::end(ret), std::make_move_iterator(std::begin(v)),
               std::make_move_iterator(std::end(v)));
  }
  return ret;
}

StatementP Parser::parseStatementP() {
  if (nextToken() && nextToken().value().type() == TokenType::While) {
    return parseLoopExprP();
  } else if (nextToken() && nextToken().value().type() == TokenType::If) {
    return parseCondExprP();
  } else if (nextToken() && nextToken().value().type() == TokenType::Id) {
    return parseAssignmentExprP();
  }
  throw InvalidASTError();
}

Statement::Block Parser::parseStmts() {
  Statement::Block ret;
  if (nextToken() && (nextToken().value().type() == TokenType::Id ||
                      nextToken().value().type() == TokenType::While ||
                      nextToken().value().type() == TokenType::If)) {
    auto s = parseStatementP();
    auto v = parseStmts();
    v.insert(v.begin(), std::move(s));
    ret.insert(std::end(ret), std::make_move_iterator(std::begin(v)),
               std::make_move_iterator(std::end(v)));
  }
  return ret;
}

BlockExprP Parser::parseBlockExpr() {
  Declaration::Block d = parseDecls();
  Statement::Block s = parseStmts();
  return std::make_unique<const BlockExpr>(std::move(d), std::move(s));
}

std::vector<ArithmeticExprP> Parser::parseFunArgs() {
  std::vector<ArithmeticExprP> ret;

  while (nextToken() && (nextToken().value().type() == TokenType::Num ||
                         nextToken().value().type() == TokenType::Id ||
                         nextToken().value().type() == TokenType::ArithOp ||
                         nextToken().value().type() == TokenType::LParen)) {
    auto ae = parseArithmeticExpr();
    ret.push_back(std::move(ae));

    if (nextToken() && nextToken().value().type() == TokenType::Comma) {
      matchToken(TokenType::Comma);
    }
  }
  return ret;
}

FunctionCallP Parser::parseFunCall() {
  auto id = parseVariableExpr();
  matchToken(TokenType::LParen);
  auto args = parseFunArgs();
  matchToken(TokenType::RParen);
  return std::make_unique<const FunctionCall>(id->name(), std::move(args));
}

std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                      std::unique_ptr<const VariableExpr>>>
Parser::parseParams() {
  std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                        std::unique_ptr<const VariableExpr>>>
      ret;
  while (nextToken() && nextToken().value().type() == TokenType::Type) {
    matchToken(TokenType::Type);  // change if we get more types
    auto id = parseVariableExpr();
    auto pair =
        std::make_pair(std::make_unique<const IntType>(), std::move(id));
    ret.push_back(std::move(pair));

    if (nextToken() && nextToken().value().type() == TokenType::Comma) {
      matchToken(TokenType::Comma);
    }
  }
  return ret;
}

std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                      std::unique_ptr<const VariableExpr>>>
Parser::parseOptParams() {
  if (nextToken() && nextToken().value().type() == TokenType::Type) {
    return parseParams();
  } else {
    std::vector<std::pair<std::unique_ptr<const TypeExpr>,
                          std::unique_ptr<const VariableExpr>>>
        ret;
    return {};
  }
}

FunctionDefP Parser::parseFunDef() {
  matchToken(TokenType::Def);
  auto id = parseVariableExpr();

  matchToken(TokenType::LParen);
  auto optparams = parseOptParams();
  matchToken(TokenType::RParen);
  matchToken(TokenType::HasType);

  // auto t = parseTypeExpr(); // change if we add more types
  matchToken(TokenType::Type);
  matchToken(TokenType::LBrace);

  auto b = parseBlockExpr();
  matchToken(TokenType::Return);

  auto ae = parseArithmeticExpr();
  matchToken(TokenType::Semicolon);
  matchToken(TokenType::RBrace);
  return std::make_unique<const FunctionDef>(
      id->name(), std::make_unique<const IntType>(), std::move(optparams),
      std::move(b), std::move(ae));
}

FunctionDef::Block Parser::parseFunDefs() {
  FunctionDef::Block ret;
  while (nextToken() && (nextToken().value().type() == TokenType::Def)) {
    auto f = parseFunDef();
    ret.push_back(std::move(f));
  }
  return ret;
}

ProgramExprP Parser::parse() {
  FunctionDef::Block f = parseFunDefs();
  BlockExprP s = parseBlockExpr();

  matchToken(TokenType::Output);
  auto ae = parseArithmeticExpr();
  matchToken(TokenType::Semicolon);

  return std::make_unique<const Program>(std::move(f), std::move(s),
                                         std::move(ae));
}
}  // namespace cs160::frontend
