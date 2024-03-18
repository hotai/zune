#include "tokenizer.h"


// init statics
// map<int, string> Token::_typeMap;
// Token::StaticConstructor Token::_staticConstructor;

// Tokenizer impl
vector<Token> Tokenizer::tokenize() {
	vector<Token> tokens;
	string buf;
	char c;

    while (c=_eat(), c != 0) {
		buf.clear();
		if (c == '#') {
			// comment to end of line
			optional<char> p = _peek();
			while (p.has_value() && p.value() != '\n') {
				_eat();
				p = _peek();
			}
			continue;
		}
		else if (iswspace(c)) {
			// doesn't need to be called out explicitly, but nice to be thorough
			continue;
		}
		else if (c == ';') {
			tokens.push_back({TokenType::semi, ";"s});
			continue;
		}
		else if (isalpha(c)) {
			_index--;
			buf = _read_word();
		
			if (buf == "print")
				tokens.push_back({TokenType::print, buf});
			else if (buf == "set")
				tokens.push_back({TokenType::_set, buf});
			else if (_is_bool_op(buf)) // and, or, not
				tokens.push_back({TokenType::_bool, buf});
			else if (_is_flow_op(buf)) // if, elif, else, repeat
				tokens.push_back({TokenType::flow, buf});
			else // var name
				tokens.push_back({TokenType::varname, buf});
                //throw runtime_error{"In Tokenizer.tokenize: Unexpected token '"s +
				//	buf + "' at location " + to_string(_index-buf.length())};
		}
		else if (isdigit(c)) {
			_index--;
			buf = _read_num();
			tokens.push_back({TokenType::number, buf});
		}
		else if (_is_op(c) && !(c == '=' && _peek() == '=')) {
			tokens.push_back({TokenType::op, string{c}});
			continue;
		}
		else if (_is_spec_char(c)) {
			// start of a comparison operator
			string comp_op;
			while (_is_spec_char(c)) {
				comp_op += c;
				c = _eat();
			}
			_index--;
			tokens.push_back({TokenType::compare, comp_op});
		}
    }
	
    _index = 0; // reset in case caller wants to tokenize again
	return tokens;
}

[[nodiscard]] optional<char> Tokenizer::_peek(int offset) const {
    if (_index+offset >= _str.length() || _index+offset < 0)
        return {};
    return _str[_index+offset];
}

char Tokenizer::_eat() {
    if (_index < 0 || _index >= _str.length())
        return 0;
    return _str[_index++];
}

string Tokenizer::_read_num() {
	string num;
	bool is_float = false;
	char c = _eat();
	while (isdigit(c) || c == '.') {
		if (c == '.') is_float = true;
		num += c;
		c = _eat();
	}
	_index--; // backtrack from last non-num char
	return num;
}

string Tokenizer::_read_word() {
	string word;
	char c = _eat();
	if (!isalpha(c)) // must start with letter
		throw runtime_error{"In Tokenizer.read_word: Unexpected character '"s +
			c + "' at location " + to_string(_index-1) +
			". Expected: a keyword or identifier starting with a letter."};
	while (isalnum(c)) {
		word += c;
		c = _eat();
	}
	_index--; // backtrack from last non-alnum char
	return word;
}
