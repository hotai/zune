#pragma once

#include <string>
using namespace std;

inline string& s_replace_in_place(string& str, const string& to_find, const string& to_replace) {
    int n = str.find(to_find);
    if (n >= 0)
        str.replace(n, to_find.length(), to_replace);
    return str;
}

inline string s_replace(const string& str, const string& to_find, const string& to_replace) {
    string temp = str;
    int n = temp.find(to_find);
    if (n >= 0)
        temp.replace(n, to_find.length(), to_replace);
    return temp;
}

inline string num_to_str(const long& val) {
    stringstream ss;
    ss << val;
    return ss.str();
}

inline bool is_numeric(const string& str)
{
    char* p;
    strtod(str.c_str(), &p);
    return *p == 0;
}
