#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include "Structures/MyArray.hpp"
#include "scheme.hpp"
#include "utilities.hpp"


using namespace std;
namespace fs = std::filesystem;


struct Row {
    MyArray<string> values;

    Row() = default;
    Row(const MyArray<string>& row) : values(row) {}

    void get_line_file(const string& val){
        values = split_csv_line(val);
    }
};

class Table {
private:
    const Schema& schema;
    MyArray<string> colnames;
    string name;
    string dir;

    void lock() {
        string path = dir + "/" + name + "_lock";
        ifstream iflock(path, ios::app);
        bool checkLock = false;
        iflock >> checkLock;
        if (checkLock){throw runtime_error("Таблица занята другим пользователем");}
        iflock.close();
        ofstream out(path);
        out << 1;
    }

    void unlock() {
        string path = dir + "/" + name + "_lock";
        ofstream out(path);
        out << 0;
    }

    uint64_t next_pk() {
        ifstream fsq(dir + "/" + name + "_pk_sequence", ios::in);
        long cur = 0;
        if (fsq.is_open()) {
            fsq >> cur;
            fsq.close();
        }
        uint64_t nxt = cur + 1;
        fsq.close();
        ofstream ofs(dir + "/" + name + "_pk_sequence");
        ofs << nxt << "\n";
        return nxt;
    }

    string get_number_for_files(){
        int fileIndex = 1;

        while (fs::exists(dir + "/" + to_string(fileIndex) + ".csv")) {
            fileIndex++;
        }

        int last = fileIndex - 1;

        if (last == 0) {
            return "1";
        }

        string lastFile = to_string(last);
        int rowCount = 0;

        if (rowCount >= schema.tuples_limit) {
            string newFile = to_string(last + 1);
            return newFile;
        }

        return lastFile;
    }

    size_t get_index_for_column(string col){
        for (int i = 0; i < colnames.msize(); i++){
            if (colnames[i] == col){
                return i;
            }
        }
        throw invalid_argument("Такой колонки нет");
    }


public:
    Table(const Schema& scm, const string& nm) 
        : schema(scm)
        , name(nm) {
        dir = schema.name + "/" + name;
        ifstream f(dir + "/1.csv", ios::in);
        string line;
        getline(f, line);
        colnames = split_csv_line(line);
        f.close();
    }

    void insert_row(const Row& rw){
        try {
            lock();
            string fileNumber = get_number_for_files(); 

            int cnt = 0;

            ifstream rowCount_out((dir + "/" + fileNumber + "_count"), ios::in);
            rowCount_out >> cnt;
            rowCount_out.close();
            
            bool nextTable = false;
            if (cnt >= schema.tuples_limit){
                fileNumber = to_string(stoi(fileNumber) + 1);
                cnt = 0;
                nextTable = true;
            }
        
            string pathToTable = dir + "/" + fileNumber + ".csv";

            ofstream tblfile(pathToTable, ios::app);
            if (!tblfile){
                throw runtime_error("Не получилось открыть файл");
            }

            if (nextTable){
                tblfile <<  join_csv_row(colnames);
            }

            for (int i = 0; i < rw.values.msize(); i++){
                tblfile << rw.values[i];
                if (i != rw.values.msize() - 1){
                    tblfile << ';';
                }
            }
            cnt++;

            tblfile << ';' << next_pk() << ';';
            
            ofstream rowCount_in((dir + "/" + fileNumber + "_count"), ios::out);
            rowCount_in << cnt;
            rowCount_in.close();

            tblfile << '\n';
            tblfile.close();
            unlock();
        } catch (exception& err){
            unlock();
            cerr << err.what() << endl;
        }
    }

    void delete_where(string columnName, const string& value){
        try {
            lock();
            
            int fileIndex = 1;
            while (fs::exists(dir + "/" + to_string(fileIndex) + ".csv")){
                int cnt;
                ifstream countIn(dir + "/" + to_string(fileIndex) + "_count", ios::in);
                countIn >> cnt;
                countIn.close();

                fs::path part = dir + "/" + to_string(fileIndex) + ".csv";
                fs::path tmp = dir + "/" + to_string(fileIndex) + ".csv.tmp";
                ifstream curentFile(part);
                ofstream newFile(tmp, ios::app);

                string line;
                int column = get_index_for_column(columnName);
                while (getline(curentFile, line)){
                    MyArray<string> tmparray = split_csv_line(line);
                    if (tmparray[column - 1] == value){
                        cnt--;
                        continue;
                    }
                    newFile << line << '\n';
                }
                ofstream countOut(dir + "/" + to_string(fileIndex) + "_count", ios::out);
                countOut << cnt;

                countOut.close();
                curentFile.close(); newFile.close();
                fs::rename(tmp, part);
                fileIndex++;
            }
            unlock();
        } catch (exception& err){
            unlock();
            cerr << err.what();
        }
    }
};
/*
int main(){
    Schema schem("schema.json");
    MyArray<string> testrow = {"1", "2", "3", "4"};
    MyArray<string> testrow2 = {"5", "6", "7", "8"};
    Table tb{schem, "таблица1"};
    Row rw{testrow};
    Row rw2{testrow2};
    tb.insert_row(rw);
    tb.insert_row(rw2);
    tb.insert_row(rw2);
    tb.delete_where(1, "5");
    return 0;
}
    */