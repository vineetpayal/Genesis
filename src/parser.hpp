#pragma once

#include <cassert>
#include <variant>

#include "arena.hpp"
#include "tokenization.hpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};


struct NodeExpr;

struct NodeBinExprEq {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprNotEq {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprLess {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprLessEq {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprGreater {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprGreaterEq {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeTermParen {
    NodeExpr *expr;
};

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMulti {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprSub {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprDiv {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    std::variant<
        NodeBinExprAdd *,
        NodeBinExprMulti *,
        NodeBinExprSub *,
        NodeBinExprDiv *,
        NodeBinExprEq *,
        NodeBinExprNotEq *,
        NodeBinExprLess *,
        NodeBinExprLessEq *,
        NodeBinExprGreater *,
        NodeBinExprGreaterEq *> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *, NodeTermParen *> var;
    // Explicit constructors:
    NodeTerm(NodeTermIntLit *p) : var(p) {
    }

    NodeTerm(NodeTermIdent *p) : var(p) {
    }

    NodeTerm(NodeTermParen *p) : var(p) {
    }
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr *expr{};
};

struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt *> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
    NodeExpr *expr{};
    NodeScope *scope{};
    std::optional<NodeIfPred *> pred;
};

struct NodeIfPredElse {
    NodeScope *scope;
};

struct NodeIfPred {
    std::variant<NodeIfPredElif *, NodeIfPredElse *> var;
};

struct NodeStmtIf {
    NodeExpr *expr{};
    NodeScope *scope{};
    std::optional<NodeIfPred *> pred;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr *expr{};
};

//while loop
struct NodeStmtWhile {
    NodeExpr *condition;
    NodeScope *scope;
};

//input and print
struct NodeStmtPrint {
    NodeExpr *expr;
};

struct NodeStmtInput {
    Token ident; // identifier that will receive the input
};


struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *, NodeScope *, NodeStmtIf *, NodeStmtAssign *,
        //addes for while loop
        NodeStmtWhile *,
        NodeStmtPrint *,
        NodeStmtInput *> var;
};

struct NodeProg {
    std::vector<NodeStmt *> stmts;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
          , m_allocator(1024 * 1024 * 4) // 4 mb
    {
    }

    void error_expected(const std::string &msg) const {
        std::cerr << "[Parse Error] Expected " << msg << " on line " << peek(-1).value().line << std::endl;
        exit(EXIT_FAILURE);
    }

    std::optional<NodeTerm *> parse_term() // NOLINT(*-no-recursion)
    {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.emplace<NodeTermIntLit>(int_lit.value());
            auto term = m_allocator.emplace<NodeTerm>(term_int_lit);
            return term;
        }
        if (auto ident = try_consume(TokenType::ident)) {
            auto expr_ident = m_allocator.emplace<NodeTermIdent>(ident.value());
            auto term = m_allocator.emplace<NodeTerm>(expr_ident);
            return term;
        }
        if (const auto open_paren = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);
            auto term_paren = m_allocator.emplace<NodeTermParen>(expr.value());
            auto term = m_allocator.emplace<NodeTerm>(term_paren);
            return term;
        }
        return {};
    }

    std::optional<NodeExpr *> parse_expr(const int min_prec = 0) // NOLINT(*-no-recursion)
    {
        std::optional<NodeTerm *> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }
        auto expr_lhs = m_allocator.emplace<NodeExpr>(term_lhs.value());

        while (true) {
            std::optional<Token> curr_tok = peek();
            std::optional<int> prec;
            if (curr_tok.has_value()) {
                prec = bin_prec(curr_tok->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            } else {
                break;
            }
            const auto [type, line, value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                error_expected("expression");
            }
            auto expr = m_allocator.emplace<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.emplace<NodeExpr>();
            expr_lhs2->var = expr_lhs->var;

            if (type == TokenType::plus) {
                expr_lhs2->var = expr_lhs->var;
                auto add = m_allocator.emplace<NodeBinExprAdd>(expr_lhs2, expr_rhs.value());
                expr->var = add;
            } else if (type == TokenType::star) {
                expr_lhs2->var = expr_lhs->var;
                auto multi = m_allocator.emplace<NodeBinExprMulti>(expr_lhs2, expr_rhs.value());
                expr->var = multi;
            } else if (type == TokenType::minus) {
                expr_lhs2->var = expr_lhs->var;
                auto sub = m_allocator.emplace<NodeBinExprSub>(expr_lhs2, expr_rhs.value());
                expr->var = sub;
            } else if (type == TokenType::fslash) {
                expr_lhs2->var = expr_lhs->var;
                auto div = m_allocator.emplace<NodeBinExprDiv>(expr_lhs2, expr_rhs.value());
                expr->var = div;
            }
            // New relational operator cases:
            else if (type == TokenType::eq_eq) {
                auto eq = m_allocator.emplace<NodeBinExprEq>(expr_lhs2, expr_rhs.value());
                expr->var = eq;
            } else if (type == TokenType::not_e) {
                auto not_e = m_allocator.emplace<NodeBinExprNotEq>(expr_lhs2, expr_rhs.value());
                expr->var = not_e;
            } else if (type == TokenType::less) {
                auto less = m_allocator.emplace<NodeBinExprLess>(expr_lhs2, expr_rhs.value());
                expr->var = less;
            } else if (type == TokenType::less_eq) {
                auto less_eq = m_allocator.emplace<NodeBinExprLessEq>(expr_lhs2, expr_rhs.value());
                expr->var = less_eq;
            } else if (type == TokenType::greater) {
                auto greater = m_allocator.emplace<NodeBinExprGreater>(expr_lhs2, expr_rhs.value());
                expr->var = greater;
            } else if (type == TokenType::greater_eq) {
                auto greater_eq = m_allocator.emplace<NodeBinExprGreaterEq>(expr_lhs2, expr_rhs.value());
                expr->var = greater_eq;
            } else {
                assert(false); // Unreachable;
            }
            expr_lhs->var = expr;
        }
        return expr_lhs;
    }

    std::optional<NodeScope *> parse_scope() // NOLINT(*-no-recursion)
    {
        if (!try_consume(TokenType::open_curly).has_value()) {
            return {};
        }
        auto scope = m_allocator.emplace<NodeScope>();
        while (auto stmt = parse_stmt()) {
            scope->stmts.push_back(stmt.value());
        }
        try_consume_err(TokenType::close_curly);
        return scope;
    }

    std::optional<NodeIfPred *> parse_if_pred() // NOLINT(*-no-recursion)
    {
        if (try_consume(TokenType::elif)) {
            try_consume_err(TokenType::open_paren);
            const auto elif = m_allocator.alloc<NodeIfPredElif>();
            if (const auto expr = parse_expr()) {
                elif->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);
            if (const auto scope = parse_scope()) {
                elif->scope = scope.value();
            } else {
                error_expected("scope");
            }
            elif->pred = parse_if_pred();
            auto pred = m_allocator.emplace<NodeIfPred>(elif);
            return pred;
        }
        if (try_consume(TokenType::else_)) {
            auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (const auto scope = parse_scope()) {
                else_->scope = scope.value();
            } else {
                error_expected("scope");
            }
            auto pred = m_allocator.emplace<NodeIfPred>(else_);
            return pred;
        }
        return {};
    }

    std::optional<NodeStmt *> parse_stmt() // NOLINT(*-no-recursion)
    {
        if (peek().has_value() && peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.emplace<NodeStmtExit>();
            if (const auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);
            try_consume_err(TokenType::semi);
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value()
            && peek(1).value().type == TokenType::ident && peek(2).has_value()
            && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt_let = m_allocator.emplace<NodeStmtLet>();
            stmt_let->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::semi);
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::ident && peek(1).has_value()
            && peek(1).value().type == TokenType::eq) {
            const auto assign = m_allocator.alloc<NodeStmtAssign>();
            assign->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                assign->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::semi);
            auto stmt = m_allocator.emplace<NodeStmt>(assign);
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::open_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.emplace<NodeStmt>(scope.value());
                return stmt;
            }
            error_expected("scope");
        }
        if (auto if_ = try_consume(TokenType::if_)) {
            try_consume_err(TokenType::open_paren);
            auto stmt_if = m_allocator.emplace<NodeStmtIf>();
            if (const auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);
            if (const auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                error_expected("scope");
            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.emplace<NodeStmt>(stmt_if);
            return stmt;
        }

        //To parse while loop
        if (try_consume(TokenType::while_)) {
            try_consume_err(TokenType::open_paren);
            auto condition = parse_expr();
            if (!condition.has_value()) {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);
            auto scope = parse_scope();
            if (!scope.has_value()) {
                error_expected("scope");
            }
            // Allocate the while statement node using the arena allocator:
            auto while_stmt = m_allocator.emplace<NodeStmtWhile>();
            while_stmt->condition = condition.value();
            while_stmt->scope = scope.value();

            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = while_stmt;
            return stmt;
        }

        //print statement
        if (try_consume(TokenType::print)) {
            try_consume_err(TokenType::open_paren);
            auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("expression");
            }
            try_consume_err(TokenType::close_paren);
            try_consume_err(TokenType::semi);
            auto print_stmt = m_allocator.emplace<NodeStmtPrint>();
            print_stmt->expr = expr.value();
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = print_stmt;
            return stmt;
        }

        //input statement
        if (try_consume(TokenType::input)) {
            try_consume_err(TokenType::open_paren);
            auto ident = try_consume(TokenType::ident);
            if (!ident.has_value()) {
                error_expected("identifier");
            }
            try_consume_err(TokenType::close_paren);
            try_consume_err(TokenType::semi);
            auto input_stmt = m_allocator.emplace<NodeStmtInput>();
            input_stmt->ident = ident.value();
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = input_stmt;
            return stmt;
        }


        return {};
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                error_expected("statement");
            }
        }
        return prog;
    }

private:
    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens.at(m_index + offset);
    }

    Token consume() {
        return m_tokens.at(m_index++);
    }

    Token try_consume_err(const TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        error_expected(to_string(type));
        return {};
    }

    std::optional<Token> try_consume(const TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        return {};
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;
};
