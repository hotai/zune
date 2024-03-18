#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <optional>
using namespace std;

enum TokenType {
	_null,
	print,
	_set,
	semi,
	_bool,
	flow,
	compare,
	number,
	varname,
	op
};

struct Token {
private:
	// static map<int, string> _typeMap;
    // static struct StaticConstructor {
    //     StaticConstructor() {
    //         _typeMap.insert({(int)TokenType::print, to_string((int)TokenType::print)});
	//         _typeMap.insert({(int)TokenType::int_lit, to_string((int)TokenType::int_lit)});
	//         _typeMap.insert({(int)TokenType::semi, to_string((int)TokenType::semi)});
    //     }
    // } _staticConstructor;

public:
	TokenType type;
	string value;

	inline Token() {
		type = TokenType::_null;
	}

	inline Token(TokenType typ, string val)
		: type(typ), value(val) {
	}

	inline const string str() const {
		return ""s + enumToStr() + ": " + value;
	}

private:
	const char* enumToStr() const {
		static const char* const names[] =
			{ "null", "print", "set", "semi", "bool", "flow", "comparison",
			  "number", "varname", "operator" };
		if (type < sizeof(names) / sizeof(names[0]))
			return names[type];
		return nullptr;
	}
};

class Tokenizer {
public:
	inline Tokenizer(const string& src)
		: _str(src) {
		_index = 0;
	}

	vector<Token> tokenize();

protected:
	inline bool _is_op(char& c) {
		static const char words[] = {'=', '+', '-', '*', '/', '(', ')', '{', '}'};
		for (const char it : words)
			if (c == it) return true;
		return false;
	}

	inline bool _is_spec_char(char& c) {
		static const char words[] = {'>', '<', '='};
		for (const char it : words)
			if (c == it) return true;
		return false;
	}

	inline bool _is_comp_op(string& s) {
		static const char* const words[] = {">", "<", ">=", "<=", "=="};
		for (const char* it : words)
			if (s == it) return true;
		return false;
	}

	inline bool _is_bool_op(string& s) {
		static const char* const words[] = {"and", "or", "not"};
		for (const char* it : words)
			if (s == it) return true;
		return false;
	}

	inline bool _is_flow_op(string& s) {
		static const char* const words[] = {"if", "elif", "else", "repeat"};
		for (const char* it : words)
			if (s == it) return true;
		return false;
	}

private:
	const string _str;
	size_t _index;

	optional<char> _peek(int offset=0) const;
	char _eat();
	string _read_num();
	string _read_word();
};

