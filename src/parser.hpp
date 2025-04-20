#pragma once
#include "tokenization.hpp";
using namespace std;

namespace node {
    struct NodeExpr {
        Token int_lit;
    };

    struct NodeExit {
        NodeExpr expr;
    };
}

class Parser {
public:
    inline explicit Parser(vector<Token> tokens) : m_tokens(move(tokens)) {
    }

    optional<node::NodeExpr> parse_expr() {
        if (peak().has_value() && peak().value().type == TokenType::int_lit) {
            return node::NodeExpr{.int_lit = consume()};
        } else {
            return {};
        }
    }

    optional<node::NodeExit> parse() {
        optional<node::NodeExit> exit_node;
        while (peak().has_value()) {
            if (peak().value().type == TokenType::exit) {
                consume();
                if (auto node_expr = parse_expr()) {
                    exit_node = node::NodeExit{.expr = node_expr.value()};
                } else {
                    cerr << "Invalid Expression" << endl;
                    exit(EXIT_FAILURE);
                }
                if (peak().has_value() && peak().value().type == TokenType::semi) {
                    consume();
                } else {
                    cerr << "Invalid Expression" << endl;
                    exit(EXIT_FAILURE);
                }
            }
        }

        m_index = 0;
        return exit_node;
    }

private:
    [[nodiscard]] inline optional<Token> peak(int ahead = 1) const {
        if (m_index + ahead > m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index);
        }
    }

    inline Token consume() {
        return m_tokens.at(m_index++);
    }

    const vector<Token> m_tokens;
    size_t m_index = 0;
};
