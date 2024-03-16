#pragma once

#include <sstream>
#include <map>
#include <numeric>
using namespace std;

#include "parser.h"
#include "util.h"

//string assemble(const vector<Token>& tokens);

class Assembler {
public:
    inline Assembler(ProgNode& root)
        : _root(root) {
    }
    
    string assemble();

private:
    ProgNode& _root;
    stringstream _out;
    size_t _stack_height = 0;
    map<string, size_t> _stack; // keeps track of vars' offset on the stack
    int _label_suf = 0;
    int _label_suf_if = 0;
    int _label_suf_comp = 0;
    int _label_suf_loop = 0;

    void _interpret(Node& node, vector<Node>* parent_list = nullptr);
    void _if_stmt(Node& node, vector<Node>* parent_list = nullptr);
    void _loop_stmt(Node& node);
    void _unary_op(const string& op, Node& node);
    void _binary_op(Node& op); //, Node& left, Node& right);
    void _push(const string& reg, const string& varname = ""s);
    void _pop(const string& reg);
    void _print_result();

	inline bool _is_comp_op(string& s) {
		static const char* const words[] = {">", "<", ">=", "<=", "==", "and", "or"};
		for (const char* it : words)
			if (s == it) return true;
		return false;
	}
};
