/**
 * myfind.cpp
 * Talib Pierson
 * December 2020
 * A simplified version of the UNIX command find.
 */

#include <boost/program_options.hpp>
#include <chrono>
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

/// report a failure
void run_err(const string &what, const string &msg) {
    cerr << prog << ": ‘" << what << "’: " << msg << "\n";
    exit(EXIT_FAILURE);
}

/// parse arguments
void parse_args(int argc, char *argv[]) {
    // set program name
    prog = argv[0];

    // parse options
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
                        if (arg_type != 'b' && arg_type != 'c' &&
                            arg_type != 'd' && arg_type != 'p' &&
                            arg_type != 'f' && arg_type != 'l' &&
                            arg_type != 's')
                            cerr << prog
                                 << ": Unknown argument to -type: " << arg_type
                                 << '\n';
                    } else
                        arg_err("missing argument to", "-type");
                    break;
                case 'e':  // -exec
                    if (++i < argc) {
                        exec = true;
                        for (; i < argc && !strcmp(argv[i], ";"); ++i) {
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
}

bool test_type(const filesystem::path &p) {  // TODO: test this
    switch (arg_type) {
        case 'b':  // block (buffered) special
            return fs::is_block_file(p);
        case 'c':  // character (unbuffered) special
            return fs::is_character_file(p);
        case 'd':  // directory TODO: FAILURE!
            return fs::is_directory(p);
        case 'p':  // named pipe (FIFO)
            return fs::is_fifo(p);
        case 'f':  // regular file
            return fs::is_regular_file(p);
        case 'l':  // symbolic link
            return fs::is_symlink(p);
        case 's':  // socket
            return fs::is_socket(p);
        default:
            perror("test_type");
            exit(EXIT_FAILURE);
    }
}

bool test_mtime(const filesystem::path &p) {  // TODO: mtime wrong
    /*
     * POSIX:
     * The primary shall evaluate as true
     * if the file modification time subtracted from the initialization time,
     * divided by 86400 (with any remainder discarded), is n.
     * LINUX:
     * File was last modified less than, more than or exactly n*24 hours ago.
     */

    time_t modt = chrono::system_clock::to_time_t(
            chrono::file_clock::to_sys(fs::last_write_time(p)));
    time_t curr = chrono::system_clock::to_time_t(chrono::system_clock::now());
    return (curr - modt) / 86400 == arg_mtime;
}

bool test_name(const filesystem::path &p) {  // TODO: wildcard
    return p.filename().string() == arg_name;
}

bool test(const filesystem::path &p) {
    bool ret = true;
    if (name) ret = test_name(p);
    if (mtime) ret = ret && test_mtime(p);
    if (type) ret = ret && test_type(p);
    return ret;
}

bool execute(const fs::path &path) {  // TODO: unimplemented
    bool ret = true;
    return ret;
}

void do_actions(const fs::path &path) {
    if (exec) {  // print only what is executed and returns SUCCESS
        if (execute(path) && print) cout << path.string() << '\n';
    } else {  // plain print; default action
        cout << path.string() << '\n';
    }
}

void find(fs::path &path) {
    if (test(path)) {
        do_actions(path);
    }
    for (auto &item : fs::recursive_directory_iterator(
            path,                           // iterate over path
            fs::directory_options(links)))  // follow symbolic links iff links
    {
        if (test(item)) do_actions(item.path());
    }
}

int main(int argc, char *argv[]) {
    // first parse the arguments
    parse_args(argc, argv);
    // then run find on every path in paths
    for (auto &p : paths) {
        if (fs::exists(p)) {
            if (links && fs::is_symlink(p)) p = fs::read_symlink(p);
            find(p);
        } else {
            run_err(p.string(), "No such file or directory");
        }
    }
}
