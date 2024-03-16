#include "assemble.h"

string Assembler::assemble() {
    _out.clear();
	_out << "extern printf\n"
        << "global main\n\n"
        << "section .text\n"
        << "main:\n";

    for (int i=0; i<_root.statements.size(); i++) {
        Node& stmt = _root.statements[i];
        if (stmt.node_type == NodeType::stmt_empty)
            continue;

        _interpret(stmt, &_root.statements);

        // if statements followed by elifs and else are interpreted as one block,
        // so skip all following elifs and else
        if (stmt.node_type == NodeType::if_stmt && stmt.expr == "if") {
            while (i+1<_root.statements.size() &&
                (_root.statements[i+1].expr == "elif" || _root.statements[i+1].expr == "else"))
                i++;
        }
    }

    _out << "\n    mov rax, 60\n"
        << "    mov rdi, 0\n"
        << "    syscall\n\n"
        << "section .data\n"
        //<< "    fmt_str: db \"To accommodate the longest float value possible\", 10, 0\n"
        << "    fmt_int: db \"%d\", 10, 0\n"
        << "    fmt_flt: db \"%f\", 10, 0\n";
        //<< "    msg: db \"Hello, World\", 10, 0\n";

	return _out.str();
}

// The idea with each func/node is to have it do all the assembly
// in such a way that its result is on top of the stack.
// E.g., addition will push the args on the stack, do the calculation
// and push/have the result on top of the stack.
// At a high level, we're doing a post-order traversal of the
// parse tree, represented by a list and any sublists, e.g.
// 2 + 3 * 4 -> [2, +, [3, *, 4]]
// however, it's not a binary tree - there can be more than
// two operands in a list with operations of the same type (e.g. +/- or *//)
void Assembler::_interpret(Node& node, vector<Node>* parent_list) {
    if (node.node_type == NodeType::stmt_empty)
        return;
    // else if (node.nodes.size() == 1)
    //     return _interpret(node.nodes[0]);
    else if (node.node_type == NodeType::declaration) {
        string varname = node.expr;
        Node& expr = node.nodes[0];

        // variable assignment; expression can itself be a variable
        if (expr.node_type == NodeType::expr_num) {
            _out << "    xor rax, rax\n"
                << "    mov rax, " << expr.expr << "\n";
            _push("rax", varname);
        }
        else if (expr.node_type == NodeType::expr_var) {
            _unary_op(""s, expr);
        }
        else if (expr.node_type == NodeType::expr_binop) {
            _interpret(expr, &node.nodes);
            // after this, the result of the expression will be
            // on top of stack. we no longer know what the value
            // is as it's been computed in assembly,
            // so save the position of the value on the stack instead
            _stack.insert_or_assign(varname, _stack_height-1);
        }
    }
    else if (node.node_type == NodeType::print_) {
        // a print node has only one subnode in the list - the expression,
        // however, the expression itself can contain sub-items
        _unary_op(""s, node.nodes[0]);

        // the result to be returned will always be on the top of the stack
        // due to previous operations;
        // print it out
        _print_result();
    }
    else if (node.node_type == NodeType::if_stmt) {
        // include if, elif and else;
        // if_stmt goes over if, elifs and else, unlike all other interpreation funcs
        _if_stmt(node, parent_list);
    }
    else if (node.node_type == NodeType::loop_stmt) {
        // in a while_stmt node, first node in list is condition and second is statements
        _loop_stmt(node);
    }
    else if (node.node_type == NodeType::expr_bool) {
        // e.g. "1 and 2" or "1 and 2 or 3" ([1, and, 2, or, 3])
        int index = 0;
        // left node
        _interpret(node.nodes[index], &node.nodes);
        while (index+2 < node.nodes.size()) {
            // right node
            _interpret(node.nodes[index+2], &node.nodes);
            // operator itself
            _binary_op(node.nodes[index+1]);
            index += 2;
        }
    }
    else if (node.node_type == NodeType::expr_compare) {
        // e.g. "1 < 2"; ignore if more than 2 operands
        // left node
        _interpret(node.nodes[0], &node.nodes);
        // right node
        _interpret(node.nodes[2], &node.nodes);
        // operator itself
        _binary_op(node.nodes[1]);
        if (node.nodes.size() > 3)
            cout << "In Assembler.assemble: WARN: more than 2 operands specified in a comparison operation. "
                << "Ignoring " << (node.nodes.size()-3) << " elements.\n";
    }
    else if (node.node_type == NodeType::expr_num) {
        _out << "    xor rax, rax\n"
            << "    mov rax, " << node.expr << "\n";
        _push("rax");
    }
    else if (node.node_type == NodeType::expr_var) {
        _unary_op(""s, node);
    }
    else if (node.node_type == NodeType::expr_list) {
        // just a list of expressions; evaluate one by one
        for (Node& n : node.nodes)
            _interpret(n, &node.nodes);
    }
    else if (node.node_type == NodeType::expr_unop) {
        // unary op (not x; -x, +x)
        if (node.nodes.size() != 2)
            throw runtime_error{"In Assembler.assemble: Unexpected token '"s +
                node.str() +
                "'. Expected: a unary expression, e.g. '-10', 'not x' etc."};

        _unary_op(node.nodes[0].expr, node.nodes[1]);
        return;
    }
    else if (node.node_type == NodeType::expr_binop) {
        // bool is_unary_or_binary = accumulate(node.nodes.begin(), node.nodes.end(), false,
        //     [](bool prev, Node& curr){return prev? prev : curr.node_type==NodeType::operator_;});

        // binary-operator expressions; could contain more than 2 operands in the list,
        // e.g. "1 + 2" or "1 + 2 - 3" ([1, +, 2, -, 3])
        // or "1 + 2 * 3 - 7" ([1, +, [2, *, 3], -, 7])
        // OR - could just be multiple independent expressions
        int index = 0;
        // left node
        _interpret(node.nodes[index], &node.nodes);
        while (index+2 < node.nodes.size()) {
            // right node
            _interpret(node.nodes[index+2], &node.nodes);
            // operator itself
            _binary_op(node.nodes[index+1]);
            index += 2;
        }
    }
    else
        throw runtime_error{"In Assembler.assemble: Unexpected token '"s +
            node.str() +
            "'. Expected: 'set' with a variable declaration or 'print' with expression."};
}

void Assembler::_if_stmt(Node& node, vector<Node>* parent_list) {
    // in an if_stmt node, first node in list is condition and second is statements;
    // the "else" portion will not have conditions and thus will only have one node (stmts).
    // parent_list is provided so we can walk from "if" to last "else" in the list.
    // if this is the "if" node, add it and the rest of elifs/else to a separate list
    vector<Node*> if_else_list;
    if (parent_list != nullptr && node.expr == "if") {
        for (int i=0; i<parent_list->size(); i++) {
            if (&node == &(*parent_list)[i]) {
                if_else_list.push_back(&(*parent_list)[i]);
                while (++i < parent_list->size() &&
                    ((*parent_list)[i].expr == "elif" || (*parent_list)[i].expr == "else"))
                    if_else_list.push_back(&(*parent_list)[i]);
                break;
            }
        }
    }

    if (if_else_list.size() == 0)
        if_else_list.push_back(&node);

    /*
    Have to manage labels carefully here. Example:
    if (1 > 2) {    // jz if2_N

    }               //   jmp if_endN
    elif (2 > 4) {  // if2_N: jz if3

    }               //   jmp if_endN
    else {          // if3_N:

    }               //   jmp if_endN [opt]
    ...other stmts; // if_end4:
    */
    string label_end = "if_end"s + to_string(_label_suf_if);

    for (int i=0; i<if_else_list.size(); i++) {
        Node* pnode = if_else_list[i];
        Node stmt;
        string label_this = "if"s + to_string(i+1) + "_"s + to_string(_label_suf_if);
        string label_next = "if"s + to_string(i+2) + "_"s + to_string(_label_suf_if);
        string label_jump = (i+1 < if_else_list.size())? label_next : label_end;

        _out << "\n" << label_this << ":\n";
        if (pnode->nodes.size() == 2) {
            // if/elif
            Node condition = pnode->nodes[0];
            _interpret(condition, &pnode->nodes);

            _pop("rax");
            _out << "    test rax, rax\n"
                << "    jz " << label_jump << "\n\n";

            stmt = pnode->nodes[1];
        }
        else {
            // 'else' - the only node in the list is statements
            stmt = pnode->nodes[0];
        }

        _interpret(stmt, &pnode->nodes);
        _out << "    jmp " << label_end << "\n";
    }

    _label_suf_if++;
    _out << "\n" << label_end << ":\n";
}

void Assembler::_loop_stmt(Node& node) {
    throw runtime_error{"In Assembler.assemble: 'while' statements are not implemented."};

    Node condition = node.nodes[0];
    Node stmt = node.nodes[1];

    string label_start = "loop_start"s + to_string(_label_suf_loop);
    string label_end = "loop_end"s + to_string(_label_suf_loop++);

    // TODO: this is only a one-time evaluation; need to be able to reevaluate it every time
    // for it to be a proper loop
    _interpret(condition, &node.nodes);
    _pop("rax");

    _out << "\n" << label_start << ":\n"
        << "    test rax, rax\n"
        << "    jz " << label_end << "\n";

    _interpret(stmt, &node.nodes);
    
    _out << "    jmp " << label_start << "\n\n";
    _out << label_end << ":\n";
}

// +, -, not
void Assembler::_unary_op(const string& op, Node& node) {
    string varname;
    if (node.node_type == NodeType::expr_var) {
        // find the value of the variable on the stack and re-push it on top
        varname = node.expr;
        size_t stack_offset = _stack_height - 1 - _stack.at(varname);
        _out //<< "\n"
            << "    push QWORD [rsp + " << (stack_offset*8) << "]\n";
        _stack_height++;
    }
    else if (node.node_type == NodeType::expr_num) {
        _out << "    xor rax, rax\n"
            << "    mov rax, " << node.expr << "\n";
        _push("rax");
    }
    else if (node.node_type == NodeType::expr_unop ||
            node.node_type == NodeType::expr_binop ||
            node.node_type == NodeType::expr_list) {
        // evaluate the list of nodes recursively
        _interpret(node);
    }
    else
        throw runtime_error{"In Assembler.assemble: Unexpected expression '"s +
            node.expr + "'."};

    if (op == "not") {
        string label_z = "zero"s + to_string(_label_suf);
        string label_nz = "non_zero"s + to_string(_label_suf);
        string label_done = "done"s + to_string(_label_suf);
        _label_suf++;
        _out << endl;
        _pop("rax");
        _out << "    cmp rax, 0\n"
            << "    je " << label_z << "\n"
            << label_nz << ":\n"
            << "    mov rax, 0\n"
            << "    push rax\n"
            << "    jmp " << label_done << "\n"
            << label_z << ":\n"
            << "    mov rax, 1\n"
            << "    push rax\n"
            << label_done << ":\n";
        _stack_height++; // only increment once since only one branch will execute
    }
    else if (op == "-") {
        _pop("rax");
        _out << "    mov rbx, -1\n"
            << "    mul rbx\n";
        _push("rax");
    }
}

// the two operands should already be on stack: right on top, left under it
void Assembler::_binary_op(Node& op) {
    if (op.expr == "+") {
        _pop("rbx");
        _pop("rax");
        _out << "    add rax, rbx\n";
        _push("rax");
        // _out << "    fld rax\n"
        //     << "    fld rbx\n"
        //     << "    faddp st1, st0\n"; // value is on the stack
        //     << "    fstp res\n";
        _out << "\n";
    }
    else if (op.expr == "-") {
        _pop("rbx");
        _pop("rax");
        _out << "    sub rax, rbx\n";
        _push("rax");
        _out << "\n";
    }
    else if (op.expr == "*") {
        _pop("rbx");
        _pop("rax");
        _out << "    mul rbx\n";
        _push("rax");
        _out << "\n";
    }
    else if (op.expr == "/") {
        _pop("rbx");
        _pop("rax");
        _out << "    div rbx\n";
        _push("rax");
        _out << "\n";
    }
    else if (_is_comp_op(op.expr)) {
        // example: for "1 == 2" the assembly should look like this (1 in rax, 2 in rbx):
        //    cmp rax, rbx
        //    je comp_true1
        // comp_false1:
        //    mov rax, 0
        //    push rax
        //    jmp comp_end1
        // comp_true1:
        //    mov rax, 1
        //    push rax
        // comp_end1:
        string label_true = "comp_true"s + to_string(_label_suf_comp);
        string label_false = "comp_false"s + to_string(_label_suf_comp);
        string label_end = "comp_end"s + to_string(_label_suf_comp++);
        _pop("rbx");
        _pop("rax");
        if (op.expr != "and" && op.expr != "or")
            _out << "    cmp rax, rbx\n";
        
        if (op.expr == "==")
            _out << "    je " << label_true << "\n";
        else if (op.expr == ">")
            _out << "    jg " << label_true << "\n";
        else if (op.expr == ">=")
            _out << "    jge " << label_true << "\n";
        else if (op.expr == "<")
            _out << "    jb " << label_true << "\n";
        else if (op.expr == "<=")
            _out << "    jbe " << label_true << "\n";
        else if (op.expr == "and")
            _out << "    test rax, rbx\n"
                << "    jnz " << label_true << "\n";
        else if (op.expr == "or")
            _out << "    test rax, rax\n"
                << "    jnz " << label_true << "\n"
                << "    test rbx, rbx\n"
                << "    jnz " << label_true << "\n";

        _out << label_false << ":\n"
            << "    mov rax, 0\n"
            << "    push rax\n"
            << "    jmp " << label_end << "\n"
            << label_true << ":\n"
            << "    mov rax, 1\n"
            << "    push rax\n"
            << label_end << ":\n";

        // only one condition will be true, so stack will increment by 1
        _stack_height++;
        _out << "\n";

    }
}

void Assembler::_push(const string& reg, const string& varname) {
    if (varname.length() > 0) // only keep track of variable locations on stack
        _stack.insert_or_assign(varname, _stack_height);
    _stack_height++;
    _out << "    push "s + reg + "\n";
}

void Assembler::_pop(const string& reg) {
    _stack_height--;
    _out << "    pop "s + reg + "\n";
}

void Assembler::_print_result() {
    _stack_height--;
    _out << "    pop rsi\n" // value to be printed
        << "    push rbp\n"
        //<< "    mov	rdi, fmt_int\n" // this triggers ld warnings
        << "    lea rdi, [rel fmt_int]\n"
        << "    mov	rax, 0\n"
        << "    call printf WRT ..plt\n"
        << "    pop	rbp\n";
}

/*
string Assembler::_getvar(const string& varname) {
    if (_vars.contains(varname))
        return _vars.at(varname);
    else
        throw runtime_error{"In Assembler.assemble: Usage of undeclared variable '"s +
            varname + "'."};
}
*/

/*
string assemble(const vector<Token>& tokens) {
	stringstream output;
	output << "global _start\n_start:\n";
	
	for (int i=0; i<tokens.size(); i++) {
		const Token& token = tokens[i];
		if (token.type == TokenType::print) {
			if (i+1 < tokens.size() && tokens[i+1].type == Token::TokenType::int_lit &&
				i+2 < tokens.size() && tokens[i+2].type == Token::TokenType::semi) {
				output  << "    mov rax, 60\n"
						<< "    mov rdi, " << tokens[i+1].value << "\n"
						<< "    syscall\n";
				i += 2;
			}
		}
	}
	
	return output.str();
}
*/
