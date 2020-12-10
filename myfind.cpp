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

/* ==[ INITIALIZE GLOBAL ARGUMENTS ]== */
/// name of program
std::string prog;
/// paths to search
vector<fs::path> paths;

// tests
bool name = false;
string arg_name;
bool mtime = false;
ssize_t arg_mtime = 0;
bool type = false;
char arg_type = '\0';

// actions
bool exec = false;
vector<string> arg_exec;
bool print = false;

// short argument
bool links = false;

/// provide operator<< for ostream, vector
template<typename T>
ostream &operator<<(ostream &out, const vector<T> &vec) {
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

/// parse arguments
void parse_args(int argc, char *argv[]) {
    prog = argv[0];
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
                case 'n':  // -name
                    if (++i < argc) {
                        name = true;
                        arg_name = argv[i];
                    } else
                        arg_err("missing argument to", "-name");
                    break;
                case 'm':  // -mtime
                    if (++i < argc) {
                        try {
                            mtime = true;
                            arg_mtime = stol(argv[i]);
                        } catch (const std::invalid_argument &_) {
                            arg_err(
                                    "invalid argument `" + string(argv[i]) + "' to",
                                    "-mtime");
                        }
                    } else
                        arg_err("missing argument to", "-mtime");
                    break;
                case 't':  // -type
                    if (++i < argc) {
                        type = true;
                        arg_type = argv[i][0];
                    } else
                        arg_err("missing argument to", "-type");
                    break;
                case 'e':  // -exec
                    if (++i < argc) {
                        exec = true;
                        for (; i < argc && argv[i][0] != ';'; ++i) {
                            arg_exec.emplace_back(argv[i]);
                        }
                    } else
                        arg_err("missing argument to", "-exec");
                    break;
                case 'p':  // -print
                    print = true;
                    break;
                case 'L':  // -L
                    links = true;
                    break;
                default:  // -???
                    arg_err("unknown predicate", arg);
                    break;
            }
        } else {
            paths.emplace_back(arg);
        }
    }
    if (paths.empty()) paths = {"."};    // default path is .
    if (!(print || exec)) print = true;  // default action is print

    // debug print
    cout << "prog: " << prog << '\n';
    cout << "paths: " << paths << '\n';
    cout << "name: " << name << ' ' << arg_name << '\n';
    cout << "mtime: " << mtime << ' ' << arg_mtime << '\n';
    cout << "type: " << type << ' ' << arg_type << '\n';
    cout << "exec: " << exec << ' ' << arg_exec << '\n';
    cout << "print: " << print << '\n';
    cout << "links: " << links << '\n';
}

bool test(const filesystem::directory_entry &entry) {
    bool ret = true;
    return ret;
}

void run(fs::path &path) {
    cout << path << '\n';
    if (links) {
        for (auto &item : fs::recursive_directory_iterator(
                path, fs::directory_options(links))) {
            if (test(item)) std::cout << item.path().string() << '\n';
        }
    }
}

int main(int argc, char *argv[]) {
    // first parse the arguments
    parse_args(argc, argv);
    // then run find on every path in paths
    for (auto &path : paths) {
        run(path);
    }
}
