#pragma once

#include<iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
using namespace std;

enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq
};

struct Token {
    TokenType type;
    optional<string> value;
};

class Tokenizer {
public:
    explicit inline Tokenizer(const string &src)
        : m_src(move(src)) {
    }

    inline vector<Token> tokenize() {
        vector<Token> tokens;

        string buff;

        while (peek().has_value()) {
            if (isalpha(peek().value())) {
                buff.push_back(consume());

                while (peek().has_value() && isalnum(peek().value())) {
                    buff.push_back(consume());
                }

                if (buff == "exit") {
                    tokens.push_back({.type = TokenType::exit});
                    buff.clear();
                    continue;
                } else if (buff == "let") {
                    tokens.push_back({.type = TokenType::let, .value = buff});
                    buff.clear();
                    continue;
                } else {
                    tokens.push_back({.type = TokenType::ident, .value = buff});
                    buff.clear();
                    continue;
                }
            } else if (isdigit(peek().value())) {
                buff.push_back(consume());
                while (peek().has_value() && isdigit(peek().value())) {
                    buff.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_lit, .value = buff});
                buff.clear();
                continue;
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::open_paren});
                continue;
            } else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::close_paren});
                continue;
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semi});
                continue;
            } else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::eq});
                continue;
            } else if (isspace(peek().value())) {
                consume();
                continue;
            } else {
                cerr << "You messed up!" << endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    [[nodiscard]] inline optional<char> peek(int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index + offset);
        }
    }

    inline char consume() {
        return m_src.at(m_index++);
    }

    const string m_src;
    int m_index = 0;
};
