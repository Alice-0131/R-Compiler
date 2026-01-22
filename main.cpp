#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "include/Lexer/lexer.hpp"
#include "include/Parser/parser.hpp"
#include "include/Semantic/SymbolChecker.hpp"
#include "include/CodeGen/CodeGen.hpp"

int main() {
  try {
    std::string src;
    std::string line;
    while (true) {
      if (!getline(std::cin, line)) {
        break;
      }
      src += line;
      src += '\n';
    }
    
    Lexer lexer(src);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto crate = parser.parse();
    //std::cout << "Parsing succeeded." << std::endl;

    SymTable Syms;
    Checker Checker(crate, Syms);
    Checker.check();
    //std::cout << "Checker succeeded." << std::endl;

    llvm::LLVMContext context;
    llvm::Module module("R-Module", context);
    CodeGen codegen(*crate, Syms, context, module);
    bool success = codegen.emit();
    if (success) {
      module.print(llvm::outs(), nullptr);
      //std::cout << "Code generation succeeded." << std::endl;
    } else {
      throw std::runtime_error("Code generation failed.");
    }
  } catch (std::runtime_error err) {
    std::cerr << "Error: " << err.what() << std::endl;
    return 1;
  }
  
  return 0;
}
