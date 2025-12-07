#pragma once
#include "scheme.hpp"
#include "AST_sql_parser.hpp"
#include "table.hpp"
#include "Structures/UnMap.hpp"

using namespace std;

class Executor {
public:
   

    Executor(Schema& Schema) : db(Schema) {}

    void execute(const SQLStmt& stmt) {
        switch (stmt.type) {
            case SQLStmt::SELECT:
                execute_select(*stmt.select);
                break;

            case SQLStmt::INSERT:
                execute_insert(*stmt.insert);
                break;

            case SQLStmt::DELETE:
                execute_delete(*stmt.del);
                break;
            default:
                throw runtime_error("Неизвестное выражение");
        }
    }

private:
    Schema& db;

    void execute_insert(const InsertStmt& ins) {
        Table table{db, ins.table};
        Row rw{ins.values};
        if (!db.is_table_exist(ins.table)){
            throw runtime_error("Такой таблицы в схеме нет");
        }

        table.insert_row(rw); 
    }


    void execute_delete(const DeleteStmt& del) {
        Table table{db, del.table};
        table.delete_where(del.where -> left -> colref.column, del.where -> right -> value);
    }


    void execute_select(const SelectStmt& sel) {
        cout << select(sel) << endl;

    }

    string eval_operand(shared_ptr<Expr> e,
                        const UnMap<string,string>& rowmap) const {
        if (e -> kind == Expr::VALUE) {
            return e -> value;
        }
        if (e -> kind == Expr::COLUMN_REF) {
            string key = e -> colref.table + "." + e -> colref.column;
            return rowmap[key];
        }
        throw runtime_error("Неизвестный операнд");
    }

    bool eval_expr(shared_ptr<Expr> e,
                   const UnMap<string,string>& rowmap) const {
        if (e == nullptr){
            return false;
        }
        switch (e -> kind) {
            case Expr::AND:
                return eval_expr(e -> left, rowmap) && eval_expr(e -> right, rowmap);

            case Expr::OR:
                return eval_expr(e -> left, rowmap) || eval_expr(e -> right, rowmap);
                
            case Expr::PARENS:
                return eval_expr(e -> left, rowmap);

            case Expr::EQUAL:
                return eval_operand(e -> left, rowmap) == eval_operand(e -> right, rowmap);

            default:
                throw runtime_error("Bad expression type");
        }
    }

    MyArray<MyArray<string>> select(const SelectStmt& sel) const {

        MyArray<MyArray<string>> result;

        int n = sel.tables.msize();

        MyArray<MyArray<string>> currentRows;
       
        MyArray<MyArray<string>> headers;

        for (int i = 0; i < n; i++) {
            ifstream tb(db.name + "/" + sel.tables[i] + "/1.csv", ios::in);
            if (!tb.is_open()) {
                throw runtime_error("Не получилось открыть таблицы");
            }
            string header;
            getline(tb, header);
            tb.close();
            headers.MPUSH_back(split_csv_line(header));
        }        

        function<void(int)> dfs = [&](int depth) {
            if (depth == n) {
                UnMap<string,string> rowmap;

                for (int ti = 0; ti < n; ti++) {
                    string tname = sel.tables[ti];

                    for (int ci = 0; ci < headers[ti].msize(); ci++) {
                        string col = headers[ti][ci];
                        rowmap.insert(tname + "." + col, currentRows[ti][ci]);
                    }
                }

                
                if (sel.where && !eval_expr(sel.where, rowmap)) {
                    return;
                }


                MyArray<string> row;
                for (int c = 0; c < sel.columns.msize(); c++) {

                    string tname = sel.columns[c].table;
                    string cname = sel.columns[c].column;

                    int ti = -1;
                    for (int i = 0; i < n; i++) {
                        if (sel.tables[i] == tname) {
                            ti = i;
                        }
                    }

                    if (ti == -1) {
                        throw runtime_error("Неизвестная таблица: " + tname);
                    }

                    int ci = -1;
                    for (int j = 0; j < headers[ti].msize(); j++){
                        if (headers[ti][j] == cname){
                            ci = j;
                        }
                    }
                    if (ci == -1){
                        throw runtime_error("Неизвестная колонка: " + cname);
                    }

                    row.MPUSH_back(currentRows[ti][ci]);
                }

                result.MPUSH_back(row);
                return;
            }

            string tname = sel.tables[depth];    
            int fileIndex = 1;

            while (fs::exists(db.name + "/" + tname + "/" + to_string(fileIndex) + ".csv")) {
                ifstream tb(db.name + "/" + tname + "/" + to_string(fileIndex) + ".csv");
                string line;

                getline(tb, line); // пропускаем header

                while (getline(tb, line)) {
                    if (line.empty()) {continue;}
                    currentRows.MPUSH_back(split_csv_line(line));
                    dfs(depth + 1);  // преходим на след 
                    currentRows.MPOP_back();
                }
                tb.close();
                fileIndex++;
            }
            
        };

        dfs(0);
        return result;
    }
};
