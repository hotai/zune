// The Parser creates basically a list of nodes,
// some of which can be nested lists of nodes themselves, e.g.:
// [1, *, [(, 3, +, 10, )]]
// There may be more than two nodes for expressions of same type, e.g.:
// [2, +, 10, -, 3, -, 1] -
// not ideal design (each node should only contain two child nodes
// in a proper binary tree), but we're simplifying a bit by not using
// strictly binary trees.
#pragma once

#include <vector>
#include <optional>
#include <variant>
using namespace std;

#include "tokenizer.h"

enum NodeType {
    null,
    program,
    stmt_empty,
    print_,
    declaration, // var declaration
    operator_, // operator (+ - * /)
    expr_num, // 17 | 200.34
    expr_var,  // var
    expr_unop, // -1, +1, not x
    expr_binop, // 1+2*4 (expression with one or more binary operators)
    expr_list, // just a list of independent expressions/statements
    expr_bool, // x > 5 and y > 10
    expr_compare, // x > 5
    //expr_paren // ((1+2)*4)
    if_stmt,
    loop_stmt
};

// Parse tree Nodes
// expression can be either:
// a) numeric literal; b) variable; c) simple arith expr; d) parenthesized expr
// statement can be either:
// a) return; b) var declaration; c) if statement; d) while statement
struct Node {
    NodeType node_type = NodeType::null;
    string expr;
    vector<Node> nodes;

    inline Node() {}
    inline Node(NodeType type): node_type(type) {}
    inline string str() { return enumToStr();}
    inline string detailed(int level=0) {
        stringstream ss;
        ss << str() << ":" << expr;
        if (nodes.size() == 0)
            return ss.str();
        ss << "[";
        for (int i = 0; i < nodes.size(); i++) {
            ss << nodes[i].detailed(level++);
            if (i<nodes.size()-1) ss << ", ";
        }
        ss << "]";
        return ss.str();
    }

private:
	const char* enumToStr() {
		static const char* const names[] =
			{ "null", "program", "stmt-empty", //"stmt-ret", "stmt-decl",
              "print", "declaration", "operator",
			  "expr_num", "expr_var", "expr_unop",
              "expr_binop", "expr_list", "expr_bool",
              "expr_compare", "if_stmt", "loop_stmt" };
		if (node_type < sizeof(names) / sizeof(names[0]))
			return names[node_type];
		return nullptr;
	}
};

struct ProgNode {
    vector<Node> statements;
    inline string str() { return "program"s;}
    inline string detailed(int level=0) {
        stringstream ss;
        ss << "[\n";
        for (int i = 0; i < statements.size(); i++) {
            ss << statements[i].detailed(level);
            if (i<statements.size()-1) ss << ",\n";
        }
        ss << "\n]\n";
        return ss.str();
    }
};

// Parser
class Parser {
public:
    inline Parser(const vector<Token>& tokens)
        : _tokens(tokens) {
        _index = 0;
    }

    ProgNode parse();

private:
    vector<Token> _tokens;
	size_t _index;

	Token _eat();
	const optional<Token> _peek(int offset=0) const;
    void _expect(const Token& tok, const string& s, int location) const;
    Node _parse_stmt();
    Node _parse_if_loop();
    Node _parse_bool();
    Node _parse_compare();
    Node _parse_expr();
    Node _parse_term();
    Node _parse_factor();
};
