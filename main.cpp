#include <fstream>
#include <iostream>
#include <sstream>
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "midend/ir.h"

using namespace cs160::frontend;
using namespace cs160::midend;

void usage(char const* programName) {
  std::cerr
      << "Usage: " << programName << " program.l1 "
      << "This program runs a GCSE optimization pass over an L1 program. ";
}

int main(int argc, char* argv[]) {
  std::string outputFileName;

  if (argc == 3) {
    outputFileName = argv[2];
  } else {
    usage(argv[0]);
    return 1;
  }

  std::ifstream programFile{argv[1]};
  if (!programFile.is_open()) {
    std::cerr << "'" << argv[1]
              << "' does not exist or is not a regular file.\n\n";
    usage(argv[0]);

  }  // Read the file
  std::string programText{std::istreambuf_iterator<char>(programFile),
                          std::istreambuf_iterator<char>()};

  // Run the lexer
  std::cout << "Lexing the input program '" << argv[1] << "'" << std::endl;
  Lexer lexer;
  auto tokens = lexer.tokenize(programText);

  // Run the parser
  std::cout << "Parsing the token stream" << std::endl;
  Parser parser(tokens);
  auto ast = parser.parse();

  if (!ast) {
    std::cerr << "Parse error: the parser produced an empty unique_ptr"
              << std::endl;
    return 1;
  }

  // Run the IR
  std::cout << "Generating IR" << std::endl;

  // Write out the IR (unoptimized)
  std::ofstream irFile{outputFileName};

  if (!irFile.is_open()) {
    std::cerr << "Failed to open the outfile '" << outputFileName << "'"
              << std::endl;
    return 1;
  }

  IR ir;
  auto insns = ir.generateCFG(*ast);
  auto m = ir.ProgramBlocks();

  for (auto p = m.cbegin(); p != m.cend(); ++p) {
    // function definitions
    irFile << "function: " << p->first << std::endl;

    CFG cfg(p->second);
    cfg.computeAvailExprs();  // populates availableExpressions
    auto genkill_sets = cfg.getAllGenKill();
    auto optimized_function = cfg.computeGCSE(genkill_sets);

    // just write out in/out sets to top of file
    irFile << "\t\tFIXED POINT SOLUTION" << std::endl;
    auto availExprs = cfg.getAvailableExpressions();
    for (auto iter = availExprs.cbegin(); iter != availExprs.cend(); ++iter) {
      auto idx = std::distance(availExprs.cbegin(), iter);
      irFile << "block: " << std::to_string(idx) << std::endl;
      irFile << "in: ";
      for (auto i = availExprs[idx].first.cbegin();
           i != availExprs[idx].first.cend(); ++i) {
        irFile << *i << " ";
      }
      irFile << std::endl;
      irFile << "out: ";
      for (auto i = availExprs[idx].second.cbegin();
           i != availExprs[idx].second.cend(); ++i) {
        irFile << *i << "  ";
      }
      irFile << std::endl << std::endl;
    }

    irFile << std::endl;

    // write out optimized function
    irFile << "\t\tOPTIMIZED PROGRAM" << std::endl;
    for (auto v = optimized_function.cbegin(); v != optimized_function.cend();
         ++v) {
      auto idx = std::distance(optimized_function.cbegin(), v);
      irFile << " block " << std::to_string(idx) << ":" << std::endl;

      // blocks of instructions
      auto instr = v->instructions();
      for (auto b = instr.cbegin(); b != instr.cend(); ++b) {
        irFile << "    " << b->toString() << std::endl;
      }
    }
    irFile << std::endl;
  }

  return 0;
}
