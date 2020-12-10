/**
 * myfind.cpp
 * Talib Pierson
 * December 2020
 * A simplified version of the UNIX command find.
 */
#include <dirent.h>

#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

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
ostream& operator<<(ostream &out, const vector<T> &paths) {
    out << '[';
    if (!paths.empty()) {
        for (auto &path : paths) out << path << ", ";
        out << "\b\b";
    }
    out << ']';
    return out;
}

/// List subdirectories recursively
void ls_dir_r(const char *name) { DIR *dir_stream = opendir(name); }

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
                    if (i + 1 < argc) {
                        name = true; arg_name = argv[i + 1];
                    }
                    else arg_err("missing argument to", "-name");
                    break;
                case 'm': // -mtime
                    if (i + 1 < argc){
                        mtime = true; arg_mtime = stol(argv[i + 1]);
                    }
                    else arg_err("missing argument to", "-mtime");
                    break;
                case 't': // -type
                    if (i + 1 < argc) {
                        type = true; arg_type = argv[i + 1][0];
                    }
                    else arg_err("missing argument to", "-type");
                    break;
                case 'e': // -exec
                    if (i + 1 < argc) {
                        exec = true;
                        for (i = i + 1; i < argc && argv[i][0] != ';'; ++i) {
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