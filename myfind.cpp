/**
 * myfind.cpp
 * Talib Pierson
 * December 2020
 * A simplified version of the UNIX command find.
 */

#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>

using namespace std;
namespace fs = filesystem;
namespace po = boost::program_options;

std::string prog;
vector<string> paths;
bool name = false; string arg_name;
bool mtime = false; ssize_t arg_mtime = 0;
bool type = false; char arg_type = '\0';
bool exec = false; vector<string> arg_exec;
bool print = false;
bool links = false;

/// provide operator<< for ostream, vector<std::string>
template<typename T>
ostream& operator<<(ostream &out, const vector<T> &vec) {
    out << '[';
    if (!vec.empty()) {
        for (auto &path : vec) out << path << ", ";
        out << "\b\b";
    }
    out << ']';
    return out;
}

/// report a failure
void arg_err(const string &msg, const string &what) {
    cerr << prog << ": " << msg << " `" << what << "'\n";
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    // declare arguments
    prog = argv[0];

    // parse arguments
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
                case 'n': // -name
                    if (++i < argc) {
                        name = true; arg_name = argv[i];
                    }
                    else arg_err("missing argument to", "-name");
                    break;
                case 'm': // -mtime
                    if (++i < argc){
                        mtime = true; arg_mtime = stol(argv[i]);
                    }
                    else arg_err("missing argument to", "-mtime");
                    break;
                case 't': // -type
                    if (++i < argc) {
                        type = true; arg_type = argv[i][0];
                    }
                    else arg_err("missing argument to", "-type");
                    break;
                case 'e': // -exec
                    if (++i < argc) {
                        exec = true;
                        for (; i < argc && argv[i][0] != ';'; ++i) {
                            arg_exec.emplace_back(argv[i]);
                        }
                    }
                    else arg_err("missing argument to", "-exec");
                    break;
                case 'p': // -print
                    print = true;
                    break;
                case 'L': // -L
                    links = true;
                    break;
                default: // -???
                    arg_err("unknown predicate", arg);
                    break;
            }
        } else {
            paths.emplace_back(arg);
        }
    }
    if (!(print || exec)) print = true;

    cout << "prog: " << prog << '\n';
    cout << "paths: " << paths << '\n';
    cout << "name: " << name << ' ' << arg_name << '\n';
    cout << "mtime: " << mtime << ' ' << arg_mtime << '\n';
    cout << "type: " << type << ' ' << arg_type << '\n';
    cout << "exec: " << exec << ' ' << arg_exec << '\n';
    cout << "print: " << print << '\n';
    cout << "links: " << links << '\n';
}
