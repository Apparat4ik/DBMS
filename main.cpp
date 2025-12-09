#include "scheme.hpp"
#include "executor.hpp"
#include "AST_sql_parser.hpp" 

using namespace std;

int main(int argc, char* argv[]) {
    try {
        if (argc < 3) {
            cerr << "Usage: " << argv[0] << " --schema <schema.json>\n";
            return 1;
        }

        string schemaPath;

        for (int i = 1; i < argc; i++) {
            string arg = argv[i];
            if (arg == "--schema" && i + 1 < argc) {
                schemaPath = argv[i++];
            } else if (arg.rfind("--schema=", 0) == 0) {
                schemaPath = arg.substr(9); // после "--schema="
            }
        }

        if (schemaPath.empty()) {
            cerr << "Ошибка: не найден файл схемы.\n";
            return 1;
        }
        Schema schema("schema.json");
        Executor exec(schema);
      
        string line, buffer;
        while (getline(cin, line)) {
            cout << "sql> ";
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
