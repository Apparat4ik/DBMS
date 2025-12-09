#pragma once
#include <string>
#include <iostream>
#include "utilities.hpp"
#include "Structures/MyArray.hpp"

using namespace std;

// токены для таблицы.колонки
struct ColumnRef {
    bool operator!=(const ColumnRef& other){
        return (table != other.table || column != other.column);
    }

    string table;
    string column;
};

// операторы для where
struct Expr {
    enum Kind { AND, OR, EQUAL, COLUMN_REF, VALUE, PARENS } kind;
    shared_ptr<Expr> left, right;
    ColumnRef colref;
    string value;

    Expr(Kind k) : kind(k) {}
};

// токен для операции выборки
struct SelectStmt {
    MyArray<ColumnRef> columns;
    MyArray<string> tables;
    shared_ptr<Expr> where;
};

// токени для вставки
struct InsertStmt {
    string table;
    MyArray<string> values;
};

// токен для удаления
struct DeleteStmt {
    string table;
    shared_ptr<Expr> where;
};


struct SQLStmt {
    enum Type { SELECT, INSERT, DELETE } type;
    shared_ptr<SelectStmt> select;
    shared_ptr<InsertStmt> insert;
    shared_ptr<DeleteStmt> del;
};


struct Token {
    string text;

    bool operator==(const Token& other){
        return text == other.text;
    }

    bool operator!=(const Token& other){
        return text != other.text;
    }
};

class Lexer {
public:
    Lexer(const string &src) : str(src), pos(0) { tokenize(); }

    const MyArray<Token>& tokens() const { return toks; }

private:
    string str;
    size_t pos;
    MyArray<Token> toks;

    void tokenize() {
        while (pos < str.size()) {
            char c = str[pos];

            if (isspace((unsigned char)c)) { pos++; continue; }

            if (c == ',' || c == '(' || c == ')' || c == ';' || c == '=') {
                toks.MPUSH_back({string(1, c)});
                ++pos;
                continue;
            }

            // identifier / keyword — разрешаем точку внутри токена:
            if (isalpha((unsigned char)c)) {
                std::string cur;
                while (pos < str.size()) {
                    char d = str[pos];
                    if (isalnum((unsigned char)d) || d == '_' || d == '.') {
                        cur.push_back(d);
                        pos++;
                    } else break;
                }
                toks.MPUSH_back({cur});
                continue;
            }
            
            if (c == '\'') {
                string lit;
                lit.push_back(c);
                pos++;
                while (pos < str.size()) {
                    char d = str[pos++];
                    lit.push_back(d);
                    if (d == '\''){break;}
                }
                toks.MPUSH_back({lit});
                continue;
            }

            // identifier / keyword
            if (isalnum((unsigned char)c) || c == '_') {
                string id;
                while (pos < str.size() && (isalnum((unsigned char)str[pos]) || str[pos] == '_' || str[pos]=='.')) {
                    id.push_back(str[pos++]);
                }
                toks.MPUSH_back({id});
                continue;
            }

            // отбрасываем неподходящие символы
            string other;
            while (pos < str.size() && !isspace((unsigned char)str[pos]) 
                   && string(",().;=").find(str[pos]) == string::npos) {
                other.push_back(str[pos++]);
            } 
                
            if (!other.empty()) {
                toks.MPUSH_back({other});
            }
        }

        // проверяем чтобы послдений токен был ;
        if (toks.empty() || toks.back().text != ";")
            toks.MPUSH_back({";"});
    }
};


class Parser {
public:
    explicit Parser(const string &sql)
        : lex(sql)
        , toks(lex.tokens())
        , pos(0) {}

    SQLStmt parse() {
        if (match_keyword("select"))  return parse_select_stmt();
        if (match_keyword("insert"))  return parse_insert_stmt();
        if (match_keyword("delete"))  return parse_delete_stmt();
        throw runtime_error("Неизвестное выражение");
    }

private:
    Lexer lex;
    const MyArray<Token>& toks;
    size_t pos;

    
    string peek() const {
        return pos < toks.msize() ? toks[pos].text : "";
    }
    string next() {
        return pos < toks.msize() ? toks[pos++].text : "";
    }
    bool match(const string& t) {
        if (peek() == t) {
             pos++; 
             return true;
        }
        return false;
    }
    bool match_keyword(const string& kw) {
        if (lower(peek()) == kw) { pos++; return true; }
        return false;
    }
    void expect(const string& t) {
        if (!match(t))
            throw runtime_error("Expected token: " + t);
    }


    SQLStmt parse_select_stmt() {
        shared_ptr<SelectStmt> stmt = make_shared<SelectStmt>();

   
        while (true) {
            stmt -> columns.MPUSH_back(parse_column_ref());
            if (!match(",")) break;
        }

        
        expect_keyword("from");
        while (true) {
            stmt->tables.MPUSH_back(next());
            if (!match(",")) break;
        }

        
        if (match_keyword("where")) {
            stmt->where = parse_expr();
        }

        expect(";");

        SQLStmt r;
        r.type = SQLStmt::SELECT;
        r.select = stmt;
        return r;
    }

    SQLStmt parse_insert_stmt() {
        expect_keyword("into");
        shared_ptr<InsertStmt> stmt = make_shared<InsertStmt>();
        stmt -> table = next();

        expect_keyword("values");
        expect("(");

        while (true) {
            stmt -> values.MPUSH_back(unquote_sql_literal(next()));
            if (!match(",")){break;}
        }

        expect(")");
        expect(";");

        SQLStmt r;
        r.type = SQLStmt::INSERT;
        r.insert = stmt;
        return r;
    }

    SQLStmt parse_delete_stmt() {
        expect_keyword("from");
        shared_ptr<DeleteStmt> stmt = make_shared<DeleteStmt>();
        stmt -> table = next();

        if (match_keyword("where"))
            stmt -> where = parse_expr();

        expect(";");

        SQLStmt r;
        r.type = SQLStmt::DELETE;
        r.del = stmt;
        return r;
    }


    shared_ptr<Expr> parse_expr() { return parse_or(); }

    shared_ptr<Expr> parse_or() {
        auto left = parse_and();
        while (match_keyword("or")) {
            shared_ptr<Expr> node = make_shared<Expr>(Expr::OR);
            node -> left = left;
            node -> right = parse_and();
            left = node;
        }
        return left;
    }

    shared_ptr<Expr> parse_and() {
        shared_ptr<Expr> left = parse_term();
        while (match_keyword("and")) {
            shared_ptr<Expr> node = make_shared<Expr>(Expr::AND);
            node -> left = left;
            node -> right = parse_term();
            left = node;
        }
        return left;
    }

    shared_ptr<Expr> parse_term() {
        if (match("(")) {
            auto inside = parse_expr();
            expect(")");
            auto n = make_shared<Expr>(Expr::PARENS);
            n -> left = inside;
            return n;
        }
        return parse_equality();
    }

    shared_ptr<Expr> parse_equality() {
        string a = next();
        expect("=");        
        string b = next();

        shared_ptr<Expr> eq = make_shared<Expr>(Expr::EQUAL);
        eq -> left = make_operand(a);
        eq -> right = make_operand(b);
        return eq;
    }

    shared_ptr<Expr> make_operand(const string &t) {
        if (t.size() >= 2 && t.front() == '\'' && t.back() == '\'') {
            shared_ptr<Expr> n = make_shared<Expr>(Expr::VALUE);
            n -> value = t.substr(1, t.size()-2);
            return n;
        }
        size_t dot = t.find('.');
        if (dot == string::npos){
            throw runtime_error("Expected value or table.column");
        }
        shared_ptr<Expr> n = make_shared<Expr>(Expr::COLUMN_REF);
        n -> colref.table = t.substr(0, dot);
        n -> colref.column = t.substr(dot+1);
        return n;
    }


    // впомогательные функции
    ColumnRef parse_column_ref() {
        string t = next();
        size_t dot = t.find('.');
        if (dot == string::npos)
            throw runtime_error("Expected table.column");

        return { t.substr(0,dot), t.substr(dot + 1) };
    }


    void expect_keyword(const string &kw) {
        if (!match_keyword(kw))
            throw runtime_error("Expected keyword: " + kw);
    }
};



