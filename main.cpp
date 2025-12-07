#include "scheme.hpp"
#include "executor.hpp"
#include "AST_sql_parser.hpp" 

using namespace std;

int main() {
    try {
        Schema schema("schema.json");
        Executor exec(schema);
      
        string line, buffer;
        while (cout << "sql> ", getline(cin, line)) {
            buffer += line + " ";
            size_t pos;
            while ((pos = buffer.find(';')) != string::npos) {
                string stmtText = buffer.substr(0, pos+1);
                buffer = buffer.substr(pos+1);

                try {
                    Parser p(stmtText);
                    SQLStmt st = p.parse();
                    exec.execute(st);
                } catch (exception &e) {
                    cerr << "Parse/Execution error: " << e.what() << endl;
                }
            }
        }

    } catch (exception &e) {
        cerr << "Fatal: " << e.what() << endl;
        return 1;
    }
    return 0;
}
