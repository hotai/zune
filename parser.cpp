
#include "parser.h"


ProgNode Parser::parse() {
    ProgNode retval;

    optional<Token> curr_tok = _peek();
    while (curr_tok.has_value() &&
            curr_tok.value().type != TokenType::_null) {
        Node sn = _parse_stmt();
        retval.statements.push_back(sn);
        curr_tok = _peek();
    }

    _index = 0; // reset in case we want to parse again
    return retval;
}

// statement can be either a return or a var declaration
Node Parser::_parse_stmt() {
    Node node{NodeType::stmt_empty};
    Token curr_tok = _eat();
    while (curr_tok.type != TokenType::_null) {
        if (curr_tok.type == TokenType::semi)
            // empty statement
            return node;
        else if (curr_tok.type == TokenType::print) {
            node.node_type = NodeType::print_;
            node.nodes.push_back(_parse_expr());
        }
        else if (curr_tok.type == TokenType::_set) {
            auto next_tok = _eat();
            if (next_tok.type == TokenType::varname) {
                node.node_type = NodeType::declaration;
                node.expr = next_tok.value; // var name
                _expect(_eat(), "=", _index-1);
                node.nodes.push_back(_parse_expr());
            }
            else
                throw runtime_error{"In Parser.parse_stmt: Unexpected token '"s +
                    next_tok.str() + "' in token list at index "s + to_string(_index-1) + 
                    ". Expected: variable name."};
        }
        else if (curr_tok.type == TokenType::flow) {
            return _parse_if_loop();
            // do not fall through - there's no ; at the end of if/while
        }
        else
            throw runtime_error{"In Parser.parse_stmt: Unexpected token '"s +
                curr_tok.str() + "' in token list at index "s + to_string(_index-1) + 
                ". Expected: 'set' with a variable declaration or 'return' with expression."};

        // all statements must terminate with ;
        _expect(_eat(), ";", _index-1);
        return node;
        //curr_tok = _eat();
    }

    // shouldn't get here
    return {};
}

Node Parser::_parse_if_loop() {
    Node node;
    Token curr_tok = _peek(-1).value(); // current token, already eaten by caller

    // in an if_stmt or while_stmt node, first node in list is condition and second is statements;
    // the "else" portion will not have conditions and thus will only have one node (stmts)
    if (curr_tok.value == "if" || curr_tok.value == "elif" || curr_tok.value == "else")
        node.node_type = NodeType::if_stmt;
    else // while
        node.node_type = NodeType::loop_stmt;
    node.expr = curr_tok.value;
    
    if (curr_tok.value != "else") {
        _expect(_eat(), "(", _index-1);
        node.nodes.push_back(_parse_bool());
        _expect(_eat(), ")", _index-1);
    }

    _expect(_eat(), "{", _index-1);

    Node stmt{NodeType::expr_list};
    optional<Token> next_tok = _peek();
    while (next_tok.has_value() &&
            next_tok.value().type != TokenType::_null &&
            next_tok.value().value != "}") {
        stmt.nodes.push_back(_parse_stmt());
        next_tok = _peek();
    }
    node.nodes.push_back(stmt);

    _expect(_eat(), "}", _index-1);

    return node;
}

Node Parser::_parse_bool() {
    Node node{NodeType::expr_bool};
    node.nodes.push_back(_parse_compare());

    optional<Token> tok = _peek();
    while (tok.has_value() && tok.value().type == TokenType::_bool) {
        Token ct = _eat();
        Node op{NodeType::operator_};
        op.expr = ct.value;
        node.nodes.push_back(op);
        node.nodes.push_back(_parse_compare());
        tok = _peek();
    }
    return node;
}

Node Parser::_parse_compare() {
    Node left = _parse_expr();
    optional<Token> tok = _peek();
    if (!(tok.has_value() && tok.value().type == TokenType::compare)) {
        return left;
    }

    Node node{NodeType::expr_compare};
    node.nodes.push_back(left);

    tok = _peek();
    while (tok.has_value() && tok.value().type == TokenType::compare) {
        Token ct = _eat();
        Node op{NodeType::operator_};
        op.expr = ct.value;
        node.nodes.push_back(op);
        node.nodes.push_back(_parse_compare());
        tok = _peek();
    }
    return node;
}

Node Parser::_parse_expr() {
    Node left = _parse_term();
    optional<Token> tok = _peek();
    if (!(tok.has_value() && (tok.value().value == "+" || tok.value().value == "-"))) {
        return left;
    }

    Node node{NodeType::expr_binop};
    node.nodes.push_back(left);

    while (tok.has_value() && (tok.value().value == "+" || tok.value().value == "-")) {
        Token ct = _eat();
        Node op{NodeType::operator_};
        op.expr = ct.value;
        Node right = _parse_term();
        node.nodes.push_back(op);
        node.nodes.push_back(right);
        tok = _peek();
    }

    return node;
}

Node Parser::_parse_term() {
    Node left = _parse_factor();
    optional<Token> tok = _peek();
    if (!(tok.has_value() && (tok.value().value == "*" || tok.value().value == "/"))) {
        return left;
    }

    Node node{NodeType::expr_binop};
    node.nodes.push_back(left);

    while (tok.has_value() && (tok.value().value == "*" || tok.value().value == "/")) {
        Token ct = _eat();
        Node op{NodeType::operator_};
        op.expr = ct.value;
        Node right = _parse_factor();
        node.nodes.push_back(op);
        node.nodes.push_back(right);
        tok = _peek();
    }

    return node;
}

Node Parser::_parse_factor() {
    Node node;
    Token tok = _eat();
    if (tok.type == TokenType::number) {
        node.node_type = NodeType::expr_num;
        node.expr = tok.value;
        return node;
    }
    else if (tok.type == TokenType::varname) {
        node.node_type = NodeType::expr_var;
        node.expr = tok.value;
        return node;
    }
    else if (tok.value == "(") {
        Node node = _parse_expr();
        _expect(_eat(), ")", _index-1);
        return node;
    }
    else if (tok.value == "-" || tok.value == "+" || tok.value == "not") {
        node.node_type = NodeType::expr_unop;
        Node op{NodeType::operator_};
        op.expr = tok.value;
        Node operand = _parse_factor();
        node.nodes.push_back(op);
        node.nodes.push_back(operand);
        return node;
    }

    throw runtime_error{"In Parser.factor: Unexpected token '"s + tok.str() +
        "' in token list at index "s + to_string(_index-1) + 
        ". Expected: numeric expression or a variable."};

}

Token Parser::_eat() {
    if (_index < 0 || _index >= _tokens.size())
        return Token{TokenType::_null, ""s};
    return _tokens[_index++];
}

[[nodiscard]] const optional<Token> Parser::_peek(int offset) const {
    if (_index+offset >= _tokens.size() || _index+offset < 0)
        return {};
    return _tokens[_index+offset];
}

void Parser::_expect(const Token& tok, const string& s, int location) const {
    if (tok.value != s)
        throw runtime_error{"While parsing: Unexpected token '"s + tok.str() +
            "' in token list at index "s + to_string(location) + 
            ". Expected: '" + s + "'."};
}
