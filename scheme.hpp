#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "Structures/MySet.hpp"
#include "AST_sql_parser.hpp"
#include "Structures/UnMap.hpp"

using namespace std;

class Schema {
public:
    string name;
    int tuples_limit;
    MySet<string> tablenames;

    Schema(const string& filename) {
        loadFromJSON(filename);
        initializeDirectories();
    }

    bool is_table_exist(const string& name){
        if (tablenames.SET_AT(name)){
            return true;
        } else {
            return false;
        }

    }

    MyArray<MyArray<string>> select(const SelectStmt& sel){

        MyArray<MyArray<string>> result;

        int n = sel.tables.msize();

        MyArray<MyArray<string>> currentRows;
       
        MyArray<MyArray<string>> headers;

        for (int i = 0; i < n; i++) {
            ifstream tb(name + "/" + sel.tables[i] + "/1.csv", ios::in);
            if (!tb.is_open()) throw runtime_error("Не получилось открыть таблицы");
            string header;
            getline(tb, header);
            tb.close();
            headers[i] = split_csv_line(header);
        }


        function<string(const Expr&, const UnMap<string,string>&)> eval_operand =
        [&](const Expr& e, const UnMap<string,string>& rowmap) -> string {
            if (e.kind == Expr::VALUE)
                return e.value;

            if (e.kind == Expr::COLUMN_REF) {
                string key = e.colref.table + "." + e.colref.column;
                return rowmap[key];
            }

            throw runtime_error("Bad operand");
        };


        function<bool(const Expr&, const UnMap<string,string>&)> eval_expr =
        [&](const Expr& e, const UnMap<string,string>& rowmap) -> bool {

            switch (e.kind) {

                case Expr::AND:
                    return eval_expr(*e.left, rowmap) &&
                        eval_expr(*e.right, rowmap);

                case Expr::OR:
                    return eval_expr(*e.left, rowmap) ||
                        eval_expr(*e.right, rowmap);

                case Expr::PARENS:
                    return eval_expr(*e.left, rowmap);

                case Expr::EQUAL: {
                    string L = eval_operand(*e.left, rowmap);
                    string R = eval_operand(*e.right, rowmap);
                    return L == R;
                }

                default:
                    throw runtime_error("Unexpected expression");
            }
        };

        function<void(int)> dfs = [&](int depth)
        {
            if (depth == n) {
                UnMap<string,string> rowmap;

                for (int ti = 0; ti < n; ti++) {
                    string tname = sel.tables[ti];

                    for (int ci = 0; ci < headers[ti].msize(); ci++) {
                        string col = headers[ti][ci];
                        rowmap.insert(tname + "." + col, currentRows[ti][ci]);
                    }
                }

                if (sel.where) {
                    if (!eval_expr(*sel.where, rowmap)) return;
                }


                MyArray<string> row;
                for (int c = 0; c < sel.columns.msize(); c++) {

                    string tname = sel.columns[c].table;
                    string cname = sel.columns[c].column;

                    int ti = -1;
                    for (int i = 0; i < n; i++)
                        if (sel.tables[i] == tname) ti = i;

                    if (ti == -1)
                        throw runtime_error("Unknown table: " + tname);

                    int ci = -1;
                    for (int j = 0; j < headers[ti].msize(); j++)
                        if (headers[ti][j] == cname) ci = j;

                    if (ci == -1)
                        throw runtime_error("Unknown column: " + cname);

                    row.MPUSH_back(currentRows[ti][ci]);
                }

                result.MPUSH_back(row);
                return;
            }

            string tname = sel.tables[depth];    
            int fileIndex = 1;

            while (fs::exists(name + "/" + tname + "/" + to_string(fileIndex) + ".csv")) {
                ifstream tb(name + "/" + tname + "/" + to_string(fileIndex) + ".csv");
                string line;

                getline(tb, line); // пропускаем header

                while (getline(tb, line)) {
                    if (line.empty()) continue;

                    currentRows.MPUSH_back(split_csv_line(line));
                    dfs(depth + 1);  // преходим на след таблицу
                }

                tb.close();
                fileIndex++;
            }
        };

        dfs(0);
        return result;
    }


private:
    nlohmann::json j; // храним JSON внутри

    // чтение JSON напрямую
    void loadFromJSON(const std::string& filename) {
        ifstream file(filename);
        if (!file.is_open())
            throw runtime_error("Cannot open schema.json");

        file >> j;

        name = j["name"].get<string>();
        tuples_limit = j["tuples_limit"].get<int>();
    }

    // создание директорий схемы и таблиц
    void initializeDirectories() {
        namespace fs = filesystem;

        if (!fs::exists(name))
            fs::create_directory(name);

        for (auto& [tableName, columns] : j["structure"].items()) {
            tablenames.SETADD(tableName);

            fs::path tableDir = fs::path(name) / tableName;

            if (!fs::exists(tableDir))
                fs::create_directory(tableDir);

            fs::path csv = tableDir / "1.csv";
            fs::path cnt = tableDir / "1_count";
            if (!fs::exists(csv)) {
                ofstream f(csv.string(), ios::app);
                for (string it : columns){
                    if (it != columns.back()){
                        f << it << ';';
                    } else {
                        f << it << ';';
                        f << tableName + "_pk" << '\n';
                    }
                }
                
            }
            if (!fs::exists(cnt)){
                ofstream f_cnt(cnt.string(), ios::out);
                f_cnt << 0;
                f_cnt.close();
            }

            fs::path pk = tableDir / (tableName + "_pk_sequence");
            if (!fs::exists(pk)) {
                ofstream f(pk.string());
                f << 1;
            }

            fs::path lock = tableDir / (tableName + "_lock");
            if (!fs::exists(lock)) {
                ofstream f(lock.string());
                f << 0;
            }
        }
    }
};