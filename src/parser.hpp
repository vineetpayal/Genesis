#pragma once
#include "tokenization.hpp"
#include<variant>

#include "arena.hpp"
using namespace std;


struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr;

struct BinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct BinExprMulti {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct BinExpr {
    variant<BinExprAdd *, BinExprMulti *> var;
};

struct NodeExpr {
    variant<NodeExprIntLit *, NodeExprIdent *, BinExpr *> var;
};


struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt {
    variant<NodeStmtExit *, NodeStmtLet *> var;
};

struct NodeProg {
    vector<NodeStmt> stmts;
};


class Parser {
public:
    inline explicit Parser(vector<Token> tokens)
        : m_tokens(move(tokens)),
          m_allocator(1024 * 1024 * 4) {
    }

    optional<NodeExpr*> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            auto expr_int_lit = m_allocator.alloc<NodeExprIntLit>();
            expr_int_lit->int_lit = consume();
            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = expr_int_lit;
            return expr;
        } else if (peek().has_value() && peek().value().type == TokenType::ident) {
            return NodeExpr{.var = NodeExprIdent{.ident = consume()}};
        } else {
            return {};
        }
    }

    optional<NodeStmt> parse_stmt() {
        if (peek().value().type == TokenType::exit && peek(1).has_value() && peek(1).value().type ==
            TokenType::open_paren) {
            consume();
            consume();

            NodeStmtExit stmt_exit;
            if (auto node_expr = parse_expr()) {
                stmt_exit = {.expr = node_expr.value()};
            } else {
                cerr << "Invalid Expression" << endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                consume();
            } else {
                cerr << "Expected `)`" << endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                cerr << "Expected `;`" << endl;
                exit(EXIT_FAILURE);
            }
            return NodeStmt{.var = stmt_exit};
        } else if (peek().has_value() && peek().value().type == TokenType::let
                   && peek(1).has_value() && peek(1).value().type == TokenType::ident
                   && peek(2).has_value() && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt_let = NodeStmtLet{.ident = consume()};
            consume();
            if (auto expr = parse_expr()) {
                stmt_let.expr = expr.value();
            } else {
                cerr << "Invalid Expression" << endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                cerr << "Expected `;`" << endl;
                exit(EXIT_FAILURE);
            }

            return NodeStmt{.var = stmt_let};
        } else {
            return {};
        }
    }

    optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                cerr << "Invalid Statement" << endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    [[nodiscard]] inline optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token consume() {
        return m_tokens.at(m_index++);
    }

    const vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;
};
