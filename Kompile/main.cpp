#include <fstream>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "./lexer.cpp"
#include "./logging/logging.cpp"

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
namespace Kaleidoscope {}

namespace {

int GetNextToken(std::ifstream& fin, std::queue<std::string> id_str_buffer,
                 std::queue<double> num_buffer) {
    return lexer::gettok(fin, id_str_buffer, num_buffer);
}

void MainLoop(std::ifstream& fin) {
    std::queue<std::string> id_str_buffer;  // Filled in if tok_identifier
    std::queue<double> num_buffer;          // Filled in if tok_number
    int curr_token = GetNextToken(fin, id_str_buffer, num_buffer);

    while (true) {
        std::cout << curr_token << std::endl;
        switch (curr_token) {
            // case ';':  // ignore top-level semicolons.
            //     curr_token = lexer::gettok(fin, id_str_buffer, num_buffer);
            //     break;
            case lexer::kTokDef:
                // HandleDefinition();
                std::cout << id_str_buffer.back() << std::endl;
                break;
            case lexer::kTokExtern:
                // HandleExtern();
                std::cout << id_str_buffer.back() << std::endl;
                break;
            case lexer::kTokEof:
                return;
            case lexer::kTokNumber:
                std::cout << num_buffer.back() << std::endl;
                return;
                // default:
                //     // HandleTopLevelExpression();
                //     break;
        }
        curr_token = lexer::gettok(fin, id_str_buffer, num_buffer);
    }
}
}  // namespace

int main(int argc, char* argv[]) {
    // Install standard binary operators.
    
    std::unordered_map<char, int> binop_precedence;
    // 1 is lowest precedence.
    binop_precedence['<'] = 10;
    binop_precedence['+'] = 20;
    binop_precedence['-'] = 20;
    binop_precedence['*'] = 40;  // highest.

    if (argc < 2) {
        logging::error("no file specified ");
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream fin(filename);

    // int last_char = fin.get();
    // char c;
    // while (last_char != EOF) {
    //     std::cout << last_char << std::endl;
    //     last_char = fin.get();
    // }
    MainLoop(fin);
    return 0;
}
