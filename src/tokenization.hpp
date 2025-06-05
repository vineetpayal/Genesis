#pragma once

#include <string>
#include <vector>

enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    plus,
    star,
    minus,
    fslash,
    open_curly,
    close_curly,
    if_,
    elif,
    else_,
    // New relational operators:
    eq_eq, // "=="
    not_e, // "!="
    less, // "<"
    less_eq, // "<="
    greater, // ">"
    greater_eq, // ">="
    //loops
    while_,
    // New tokens for input/output:
    print,
    input,
};

inline std::string to_string(const TokenType type) {
    switch (type) {
        case TokenType::exit:
            return "`exit`";
        case TokenType::int_lit:
            return "int literal";
        case TokenType::semi:
            return "`;`";
        case TokenType::open_paren:
            return "`(`";
        case TokenType::close_paren:
            return "`)`";
        case TokenType::ident:
            return "identifier";
        case TokenType::let:
            return "`let`";
        case TokenType::eq:
            return "`=`";
        case TokenType::plus:
            return "`+`";
        case TokenType::star:
            return "`*`";
        case TokenType::minus:
            return "`-`";
        case TokenType::fslash:
            return "`/`";
        case TokenType::open_curly:
            return "`{`";
        case TokenType::close_curly:
            return "`}`";
        case TokenType::if_:
            return "`if`";
        case TokenType::elif:
            return "`elif`";
        case TokenType::else_:
            return "`else`";
        case TokenType::eq_eq:
            return "==";
        case TokenType::not_e:
            return "!=";
        case TokenType::less:
            return "<";
        case TokenType::less_eq:
            return "<=";
        case TokenType::greater:
            return ">";
        case TokenType::greater_eq:
            return ">=";
        case TokenType::while_:
            return "`while`";
        case TokenType::print:
            return "`print`";
        case TokenType::input:
            return "`input`";
    }
    assert(false);
}

inline std::optional<int> bin_prec(const TokenType type) {
    switch (type) {
        case TokenType::star:
        case TokenType::fslash:
            return 3; // highest among our operators
        case TokenType::plus:
        case TokenType::minus:
            return 2;
        case TokenType::less:
        case TokenType::less_eq:
        case TokenType::greater:
        case TokenType::greater_eq:
        case TokenType::eq_eq:
        case TokenType::not_e:
            return 1; // relational operators evaluated after arithmetic
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    int line;
    std::optional<std::string> value{};
};

class Tokenizer {
public:
    explicit Tokenizer(std::string src)
        : m_src(std::move(src)) {
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        std::string buf;
        int line_count = 1;
        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({TokenType::exit, line_count});
                    buf.clear();
                } else if (buf == "let") {
                    tokens.push_back({TokenType::let, line_count});
                    buf.clear();
                } else if (buf == "if") {
                    tokens.push_back({TokenType::if_, line_count});
                    buf.clear();
                } else if (buf == "elif") {
                    tokens.push_back({TokenType::elif, line_count});
                    buf.clear();
                } else if (buf == "else") {
                    tokens.push_back({TokenType::else_, line_count});
                    buf.clear();
                } else if (buf == "while") {
                    // New check for while
                    tokens.push_back({TokenType::while_, line_count});
                    buf.clear();
                } else if (buf == "print") {
                    tokens.push_back({TokenType::print, line_count});
                    buf.clear();
                } else if (buf == "input") {
                    tokens.push_back({TokenType::input, line_count});
                    buf.clear();
                } else {
                    tokens.push_back({TokenType::ident, line_count, buf});
                    buf.clear();
                }
            } else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({TokenType::int_lit, line_count, buf});
                buf.clear();
            } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
                consume();
                consume();
                while (peek().has_value() && peek().value() != '\n') {
                    consume();
                }
            } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
                consume();
                consume();
                while (peek().has_value()) {
                    if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
                        break;
                    }
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({TokenType::open_paren, line_count});
            } else if (peek().value() == ')') {
                consume();
                tokens.push_back({TokenType::close_paren, line_count});
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({TokenType::semi, line_count});
            } else if (peek().value() == '=') {
                if (peek(1).has_value() && peek(1).value() == '=') {
                    consume(); // first '='
                    consume(); // second '='
                    tokens.push_back({TokenType::eq_eq, line_count});
                } else {
                    consume();
                    tokens.push_back({TokenType::eq, line_count});
                }
            } else if (peek().value() == '<') {
                consume();
                if (peek().has_value() && peek().value() == '=') {
                    consume();
                    tokens.push_back({TokenType::less_eq, line_count});
                } else {
                    tokens.push_back({TokenType::less, line_count});
                }
            } else if (peek().value() == '>') {
                consume();
                if (peek().has_value() && peek().value() == '=') {
                    consume();
                    tokens.push_back({TokenType::greater_eq, line_count});
                } else {
                    tokens.push_back({TokenType::greater, line_count});
                }
            } else if (peek().value() == '!') {
                consume();
                if (peek().has_value() && peek().value() == '=') {
                    consume();
                    tokens.push_back({TokenType::not_e, line_count});
                } else {
                    std::cerr << "Unexpected character '!' without '='\n";
                    exit(EXIT_FAILURE);
                }
            } else if (peek().value() == '+') {
                consume();
                tokens.push_back({TokenType::plus, line_count});
            } else if (peek().value() == '*') {
                consume();
                tokens.push_back({TokenType::star, line_count});
            } else if (peek().value() == '-') {
                consume();
                tokens.push_back({TokenType::minus, line_count});
            } else if (peek().value() == '/') {
                consume();
                tokens.push_back({TokenType::fslash, line_count});
            } else if (peek().value() == '{') {
                consume();
                tokens.push_back({TokenType::open_curly, line_count});
            } else if (peek().value() == '}') {
                consume();
                tokens.push_back({TokenType::close_curly, line_count});
            } else if (peek().value() == '\n') {
                consume();
                line_count++;
            } else if (std::isspace(peek().value())) {
                consume();
            } else {
                std::cerr << "Invalid token" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    [[nodiscard]] std::optional<char> peek(const size_t offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        }
        return m_src.at(m_index + offset);
    }

    char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    size_t m_index = 0;
};
