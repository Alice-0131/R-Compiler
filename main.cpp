#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "include/Lexer/lexer.hpp"
#include "include/Parser/parser.hpp"
#include "include/Semantic/SymbolChecker.hpp"

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

    SymTable Syms;
    Checker Checker(crate, Syms);
    Checker.check();
    std::cout << "Checker succeeded." << std::endl;

  } catch (std::runtime_error err) {
    std::cerr << "Error: " << err.what() << std::endl;
    return -1;
  }
  
  return 0;
}
