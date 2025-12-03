#pragma once
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "Structures/MyArray.hpp"

using namespace std;
namespace fs = std::filesystem;

inline string trim(const string &s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a])) a++;
    while (b > a && isspace((unsigned char)s[b-1])) b--;
    return s.substr(a, b - a);
}

inline MyArray<string> split_csv_line(const string& line) {
    MyArray<string> out;
    string cur;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == ';') {
            out.MPUSH_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.MPUSH_back(cur);
    for (int i = 0; i < out.msize(); i++){
        out[i] = trim(out[i]);     // trim используется на разделенные данные, чтобы убрать лишние пробелы
    } 
    return out;
}

inline string join_csv_row(const MyArray<string>& row) {
    stringstream oss;
    for (size_t i = 0; i < row.msize(); i++) {
        if (i) oss << ";";
        oss << row[i];
    }
    return oss.str();
}

// функция для удаления кавычек при SQL запросах
inline string unquote_sql_literal(const string &s) {
    if (s.size() >= 2 && s.front() == '\'' && s.back() == '\'') {
        return s.substr(1, s.size()-2);
    }
    return s;
}


inline string lower(string s) {
    for (char& c : s) {
        c = tolower(c);
    }
    return s;
}

