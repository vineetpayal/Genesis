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
    semi
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

        while (peak().has_value()) {
            if (isalpha(peak().value())) {
                buff.push_back(consume());

                while (peak().has_value() && isalnum(peak().value())) {
                    buff.push_back(consume());
                }

                if (buff == "exit") {
                    tokens.push_back({.type = TokenType::exit});
                    buff.clear();
                    continue;
                } else {
                    cerr << "You messed up!" << endl;
                    exit(EXIT_FAILURE);
                }
            } else if (isdigit(peak().value())) {
                buff.push_back(consume());
                while (peak().has_value() && isdigit(peak().value())) {
                    buff.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_lit, .value = buff});
                buff.clear();
                continue;
            } else if (peak().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semi});
                continue;
            } else if (isspace(peak().value())) {
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
    [[nodiscard]] inline  optional<char> peak(int ahead = 1) const {
        if (m_index + ahead > m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index);
        }
    }

    inline char consume() {
        return m_src.at(m_index++);
    }

    const string m_src;
    int m_index = 0;
};
