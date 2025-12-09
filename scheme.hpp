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
        load_from_JSON(filename);
        initialize_directories();
    }

    bool is_table_exist(const string& name){
        if (tablenames.SET_AT(name)){
            return true;
        } else {
            return false;
        }

    }

private:
    nlohmann::json j;

    // чтение JSON
    void load_from_JSON(const std::string& filename) {
        ifstream file(filename);
        if (!file.is_open())
            throw runtime_error("Cannot open schema.json");

        file >> j;

        name = j["name"].get<string>();
        tuples_limit = j["tuples_limit"].get<int>();
    }

    // создание директорий схемы и таблиц
    void initialize_directories() {
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
                f << tableName + "_pk" << ';';
                for (string it : columns){
                    if (it != columns.back()){
                        f << it << ';';
                    } else {
                        f << it << '\n';
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