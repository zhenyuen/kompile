#include <fstream>
#include <queue>
#include <sstream>
#include <string>

namespace lexer {
enum Token {
    kTokEof = -1,

    // commands
    kTokDef = -2,
    kTokExtern = -3,

    // primary
    kTokIdentifier = -4,
    kTokNumber = -5
};

class Lexer;
}  // namespace lexer

class lexer::Lexer {
   private:
   int curr_tok_;
    std::string id_str_;
    double num_val_;

   public:
    /// gettok - Return the next token from standard input.
    int GetTok(std::ifstream &fin);

    std::string id_str();
    double num_val();
    int curr_tok();
};

std::string lexer::Lexer::id_str() { return id_str_; }

double lexer::Lexer::num_val() { return num_val_; }

int lexer::Lexer::curr_tok() { return curr_tok_; }

int lexer::Lexer::GetTok(std::ifstream &fin) {
    int last_char = ' ';

    while (isspace(last_char)) last_char = fin.get();

    // Check for end of file.
    if (last_char == EOF) return curr_tok_ = kTokEof;

    if (std::isalpha(last_char)) {  // identifier: [a-zA-Z][a-zA-Z0-9]*
        // Using stringstream faster than concantenating strings explicitly
        std::stringstream id_ss;
        do {
            id_ss << char(last_char);
            last_char = fin.get();
        } while (std::isalnum(last_char));

        id_str_ = id_ss.str();

        if (id_str_ == "def") return curr_tok_ = kTokDef;
        if (id_str_ == "extern") return curr_tok_ = kTokExtern;
        return curr_tok_ = kTokIdentifier;
    }

    if (std::isdigit(last_char) || last_char == '.') {  // Number: [0-9.]+
        std::stringstream num_ss;
        do {
            num_ss << last_char;
            last_char = fin.get();
        } while (std::isdigit(last_char) || last_char == '.');

        double num;
        num_ss >> num_val_;
        return curr_tok_ = kTokNumber;
    }

    if (last_char == '#') {
        // Comment until end of line.
        do last_char = fin.get();
        while (last_char != '\n' && last_char != '\r');

        if (last_char != EOF) return curr_tok_ = GetTok(fin);
    }

    // Otherwise, just return the character as its ascii value.
    return curr_tok_ = last_char;
}