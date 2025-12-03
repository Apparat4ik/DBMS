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
        table.delete_where(del.where ->colref.column, del.where -> value);
    }


    void execute_select(const SelectStmt& sel) {
        db.select(sel);
    }


    bool eval_expr(const Expr& e,
                   const UnMap<string,string>& rowmap) {
        switch (e.kind) {
            case Expr::AND:
                return eval_expr(*e.left, rowmap) && eval_expr(*e.right, rowmap);

            case Expr::OR:
                return eval_expr(*e.left, rowmap) || eval_expr(*e.right, rowmap);
                
            case Expr::PARENS:
                return eval_expr(*e.left, rowmap);

            case Expr::EQUAL:
                return eval_operand(*e.left, rowmap) == eval_operand(*e.right, rowmap);

            default:
                throw runtime_error("Bad expression type");
        }
    }

    string eval_operand(const Expr& e,
                             const UnMap<string,string>& rowmap)
    {
        if (e.kind == Expr::VALUE) {
            return e.value;
        }
        if (e.kind == Expr::COLUMN_REF) {
            string key = e.colref.table + "." + e.colref.column;
            return rowmap[key];
        }
        throw runtime_error("Invalid operand");
    }
};
