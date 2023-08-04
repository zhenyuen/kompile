#include <fstream>
#include <unordered_map>

#include "./ast.cpp"
#include "./lexer.cpp"
#include "./logging/logging.cpp"


namespace parser {
// class BinopPrecedenceMap;

class Parser;
}

// class parser::BinopPrecedenceMap {
//     private:
//         std::unordered_map<int, int> binop_precedence_;
    
//     public:
//         void set_binop_precedence(int binop_token, int precedence);
//         int binop_precedence(int token);
// };

class parser::Parser {
   private:
    /* data */
    std::unordered_map<int, int> binop_precedence_;
    std::ifstream fin_;
    Logging logger_;
    lexer::Lexer lexer_;

   public:
    Parser(std::string fname) : fin_(std::ifstream(fname)){};
    ~Parser() = default;

    bool set_binop_precedence(int binop_token, int precedence);
    int binop_precedence(int token);

    int GetNextToken();

    std::unique_ptr<ast::ExprAST> ParseExpression();
    std::unique_ptr<ast::ExprAST> ParseNumberExpr();
    std::unique_ptr<ast::ExprAST> ParseParenExpr();
    std::unique_ptr<ast::ExprAST> ParseIdentifierExpr();
    std::unique_ptr<ast::ExprAST> ParsePrimary();
    std::unique_ptr<ast::ExprAST> ParseBinOpRHS(
        int ExprPrec, std::unique_ptr<ast::ExprAST> LHS);
    std::unique_ptr<ast::PrototypeAST> ParsePrototype();
    std::unique_ptr<ast::FunctionAST> ParseDefinition();
    std::unique_ptr<ast::FunctionAST> ParseTopLevelExpr();
    std::unique_ptr<ast::PrototypeAST> ParseExtern();
};

bool parser::Parser::set_binop_precedence(int binop_token, int precedence) {
    if (!isascii(binop_token)) {
        logger_.error("Op must be ascii value.");
        return false;
    }
    // Make sure it has not be set previously
    if (binop_precedence_.find(binop_token) != binop_precedence_.end()) {
        logger_.error("Op precedence set previously.");
        return false;
    }
    binop_precedence_[binop_token] = precedence;
    return true;
}

int parser::Parser::GetNextToken() {
    return lexer_.GetTok(fin_);
};

int parser::Parser::binop_precedence(int token) {
    if (!isascii(token))
        return -1;

    // Make sure it's a declared binop.
    if (binop_precedence_.find(token) == binop_precedence_.end()) return -1;
    return binop_precedence_[token];
}



std::unique_ptr<ast::ExprAST> parser::Parser::ParseNumberExpr() {
    auto result = std::make_unique<ast::NumberExprAST>(lexer_.num_val());
    GetNextToken();
    return std::move(result);
}

/// parenexpr ::= '(' expression ')'
std::unique_ptr<ast::ExprAST> parser::Parser::ParseParenExpr() {
    GetNextToken();  // eat (.
    auto V = ParseExpression();
    if (!V) return nullptr;

    if (lexer_.curr_tok() != ')') {
        logger_.error("expected ')'");
        return nullptr;
    }

    GetNextToken();  // eat ).
    return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
std::unique_ptr<ast::ExprAST> parser::Parser::ParseIdentifierExpr() {
    std::string id_name = lexer_.id_str();

    GetNextToken();  // eat identifier.

    if (lexer_.curr_tok() != '(')  // Simple variable ref.
        return std::make_unique<ast::VariableExprAST>(id_name);

    // Call.
    GetNextToken();  // eat (
    std::vector<std::unique_ptr<ast::ExprAST>> args;
    if (lexer_.curr_tok() != ')') {
        while (true) {
            if (auto arg = ParseExpression())
                args.push_back(std::move(arg));
            else
                return nullptr;

            if (lexer_.curr_tok() == ')') break;

            if (lexer_.curr_tok() != ',') {
                logger_.error("Expected ')' or ',' in argument list");
                return nullptr;
            }
            
            GetNextToken();
        }
    }
    // Eat the ')'.
    GetNextToken();
    return std::make_unique<ast::CallExprAST>(id_name, std::move(args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
std::unique_ptr<ast::ExprAST> parser::Parser::ParsePrimary() {
    switch (lexer_.curr_tok()) {
        case lexer::kTokIdentifier:
            return ParseIdentifierExpr();
        case lexer::kTokNumber:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
        default:
            logger_.error("uUnknown token when expecting an expression");
            return nullptr;
    }
}

std::unique_ptr<ast::ExprAST> parser::Parser::ParseBinOpRHS(
    int expr_prec, std::unique_ptr<ast::ExprAST> lhs) {
    // If this is a binop, find its precedence.
    while (true) {
        int tok_prec = binop_precedence(lexer_.curr_tok());

        // If this is a binop that binds at least as tightly as the current
        // binop, consume it, otherwise we are done.
        if (tok_prec < expr_prec) return lhs;

        // Okay, we know this is a binop.
        int bin_op = lexer_.curr_tok();
        GetNextToken();  // eat binop

        // Parse the primary expression after the binary operator.
        auto rhs = ParsePrimary();
        if (rhs) return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int next_prec = binop_precedence(lexer_.curr_tok());
        if (tok_prec < next_prec) {
            rhs = ParseBinOpRHS(tok_prec + 1, std::move(rhs));
            if (!rhs) return nullptr;
        }

        // Merge LHS/RHS.
        lhs = std::make_unique<ast::BinaryExprAST>(bin_op, std::move(lhs),
                                              std::move(rhs));
    }
}

/// expression
///   ::= primary binoprhs
///
std::unique_ptr<ast::ExprAST> parser::Parser::ParseExpression() {
    auto lhs = ParsePrimary();
    if (!lhs) return nullptr;

    return ParseBinOpRHS(0, std::move(lhs));
}

/// prototype
///   ::= id '(' id* ')'
std::unique_ptr<ast::PrototypeAST> parser::Parser::ParsePrototype() {
    if (lexer_.curr_tok() != lexer::Token::kTokIdentifier) {
        logger_.error("Expected function name in prototype");
        return nullptr;
    }

    std::string fn_name = lexer_.id_str();
    GetNextToken();

    if (lexer_.curr_tok() != '(') {
        logger_.error("Expected '(' in prototype");
        return nullptr;
    }

    std::vector<std::string> arg_names;
    while (GetNextToken() == lexer::Token::kTokIdentifier) arg_names.push_back(lexer_.id_str());
    if (lexer_.curr_tok() != ')') {
        logger_.error("Expected ')' in prototype");
        return nullptr;
    }

    // success.
    GetNextToken();  // eat ')'.

    return std::make_unique<ast::PrototypeAST>(fn_name, std::move(arg_names));
}

/// definition ::= 'def' prototype expression
std::unique_ptr<ast::FunctionAST> parser::Parser::ParseDefinition() {
    GetNextToken();  // eat def.
    auto proto = ParsePrototype();
    if (!proto) return nullptr;

    if (auto e = ParseExpression())
        return std::make_unique<ast::FunctionAST>(std::move(proto), std::move(e));
    return nullptr;
}

/// toplevelexpr ::= expression
std::unique_ptr<ast::FunctionAST> parser::Parser::ParseTopLevelExpr() {
    if (auto e = ParseExpression()) {
        // Make an anonymous proto.
        auto proto = std::make_unique<ast::PrototypeAST>("__anon_expr",
                                                    std::vector<std::string>());
        return std::make_unique<ast::FunctionAST>(std::move(proto), std::move(e));
    }
    return nullptr;
}

/// external ::= 'extern' prototype
std::unique_ptr<ast::PrototypeAST> parser::Parser::ParseExtern() {
    GetNextToken();  // eat extern.
    return ParsePrototype();
}
