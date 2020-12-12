/**
 * myfind.cpp
 * Talib Pierson
 * December 2020
 * A simplified version of the UNIX command find in C++20.
 */
#include <fnmatch.h>
#include <unistd.h>
#include <wait.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::invalid_argument;  // for stol
using std::ostream;
using std::stol;
using std::string;
using std::vector;
namespace fs = std::filesystem;
namespace c = std::chrono;  // for -mtime

/* ==[ INIT GLOBAL ARGS ]== */
using global_t = struct myfind_data {
public:
    /// name of program
    string prog;
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
};

/// provide operator<< for ostream, vector
ostream &operator<<(ostream &out, const vector<string> &vec) {
    out << '[';
    if (!vec.empty()) {
        for (auto &path : vec) out << path << ", ";
        out << "\b\b";
    }
    out << ']';
    return out;
}

/// report a failure
void arg_err(const string &msg, const string &what, const global_t &data) {
    cerr << data.prog << ": " << msg << " `" << what << "'\n";
    exit(EXIT_FAILURE);
}

/// report a failure
void run_err(const string &what, const string &msg, const global_t &data) {
    cerr << data.prog << ": ‘" << what << "’: " << msg << "\n";
    exit(EXIT_FAILURE);
}

/// parse arguments
void parse_args(int argc, char *argv[], global_t data) {
    // set program name
    data.prog = argv[0];

    bool expressionflag = false;
    bool predicateflag = false;

    // parse options
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg[0] == '-') {
            expressionflag = true;
            switch (arg[1]) {
                case 'n':  // -name
                    if (++i < argc) {
                        data.name = true;
                        data.arg_name = argv[i];
                    } else {
                        arg_err("missing argument to", "-name", data);
                    }
                    predicateflag = true;
                    break;
                case 'm':  // -mtime
                    if (++i < argc) {
                        try {
                            data.mtime = true;
                            data.arg_mtime = stol(argv[i]);
                        } catch (const invalid_argument &_) {
                            arg_err(
                                    "invalid argument `" + string(argv[i]) + "' to",
                                    "-mtime", data);
                        }
                    } else {
                        arg_err("missing argument to", "-mtime", data);
                    }
                    predicateflag = true;
                    break;
                case 't':  // -type
                    if (++i < argc) {
                        data.type = true;
                        data.arg_type = argv[i][0];
                        if (data.arg_type != 'b' && data.arg_type != 'c' &&
                            data.arg_type != 'd' && data.arg_type != 'p' &&
                            data.arg_type != 'f' && data.arg_type != 'l' &&
                            data.arg_type != 's') {
                            cerr << data.prog << ": Unknown argument to -type: "
                                 << data.arg_type << '\n';
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        arg_err("missing argument to", "-type", data);
                    }
                    predicateflag = true;
                    break;
                case 'e':  // -exec
                    if (++i < argc) {
                        data.exec = true;
                        for (; i < argc && strcmp(argv[i], ";") != 0; ++i) {
                            data.arg_exec.emplace_back(argv[i]);
                        }
                        if (i >= argc) {
                            arg_err("missing argument to", "-exec", data);
                        }
                        if (data.arg_exec.empty()) {
                            arg_err("invalid argument `;' to", "-exec", data);
                        }
                    } else {
                        arg_err("missing argument to", "-exec", data);
                    }
                    predicateflag = true;
                    break;
                case 'p':  // -print
                    data.print = true;
                    predicateflag = true;
                    break;
                case 'L':  // -L
                    if (predicateflag) arg_err("unknown predicate", arg, data);
                    data.links = true;
                    break;
                default:  // -???
                    arg_err("unknown predicate", arg, data);
                    break;
            }
        } else {
            if (expressionflag)
                arg_err("paths must precede expression:", arg, data);
            data.paths.emplace_back(arg);
        }
    }
    if (data.paths.empty()) data.paths = {"."};  // default path is .
    if (!(data.print || data.exec))
        data.print = true;  // default action is print
}

bool test_type(const fs::path &p, const global_t &data) {
    // TODO: test this
    // TODO: bad when combined with -L
    // TODO: cannot find symlinks
    switch (data.arg_type) {
        case 'b':  // block (buffered) special
            return fs::is_block_file(p) && !fs::is_symlink(p);
        case 'c':  // character (unbuffered) special
            return fs::is_character_file(p) && !fs::is_symlink(p);
        case 'd':  // directory TODO: FAILURE!
            return fs::is_directory(p) && !fs::is_symlink(p);
        case 'p':  // named pipe (FIFO)
            return fs::is_fifo(p) && !fs::is_symlink(p);
        case 'f':  // regular file
            return fs::is_regular_file(p) && !fs::is_symlink(p);
        case 'l':  // symbolic link
            return fs::is_symlink(p);
        case 's':  // socket
            return fs::is_socket(p) && !fs::is_symlink(p);
        default:
            perror("test_type");
            exit(EXIT_FAILURE);
    }
}

bool test_mtime(const fs::path &p, const global_t &data) {
    /*
     * POSIX:
     * The primary shall evaluate as true
     * if the file modification time subtracted from the initialization time,
     * divided by 86400 (with any remainder discarded), is n.
     * LINUX:
     * File was last modified less than, more than or exactly n*24 hours ago.
     */

    time_t modt = c::system_clock::to_time_t(
            c::file_clock::to_sys(fs::last_write_time(p)));
    time_t curr = c::system_clock::to_time_t(c::system_clock::now());
    return (curr - modt) / 86400 == data.arg_mtime;
}

bool test_name(const fs::path &p,
               const global_t &data) {
    // check the easiest thing first
    if (p.filename().string() == data.arg_name) return true;

    /* Match NAME: p.filename().c_str()
     * against the filename pattern PATTERN: arg_name.c_str(),
     * returning zero if it matches, FNM_NOMATCH if not.  */
    int match = fnmatch(data.arg_name.c_str(), p.filename().c_str(), 0);
    if (match == 0) return true;             // Zero if string matches pattern
    if (match == FNM_NOMATCH) return false;  // FNM_NOMATCH if there is no match

    // we should not get this far
    perror("fnmatch");
    exit(EXIT_FAILURE);
}

bool test(const fs::path &p, const global_t &data) {
    bool ret = true;
    if (data.name) ret = test_name(p, data);
    if (data.mtime) ret = ret && test_mtime(p, data);
    if (data.type) ret = ret && test_type(p, data);
    return ret;
}

int execute(const fs::path &path, const global_t &data) {
    // The string "{}" is replaced by the current file name being processed
    size_t len = data.arg_exec.size();
    char **args = new char *[len + 1];
    for (size_t i = 0; i < len; ++i) {
        string arg = data.arg_exec[i];
        size_t where = arg.find("{}");
        if (where != string::npos)
            arg.replace(where, arg.length(), path.string());
        // TODO: the following line is BAD!
        args[i] = new char[arg.length() + 1];
        snprintf(args[i], arg.length() + 1, "%s", arg.c_str());
    }
    args[len] = nullptr;

    pid_t pid = fork();
    if (pid == -1) {  // failure to fork
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {  // parent
        int ret;
        waitpid(pid, &ret, 0);
        return ret;
    } else {  // child
        execvpe(args[0], args, environ);
        exit(EXIT_FAILURE);  // we shouldn't get here
    }
}

void do_actions(const fs::path &path, const global_t &data) {
    if (data.exec) {  // print only what is executed and returns SUCCESS
        if (execute(path, data) && data.print) cout << path.string() << '\n';
    } else {  // plain print; default action
        cout << path.string() << '\n';
    }
}

/// TODO: cannot increment recursive directory iterator: Permission denied
void find(const fs::path &path, const global_t &data) {
    if (test(path, data)) {
        do_actions(path, global_t());
    }
    for (auto &item : fs::recursive_directory_iterator(
            path,  // iterate over path
            fs::directory_options(
                    data.links))) {  // follow symbolic links iff links
        if (test(item, data)) do_actions(item.path(), data);
    }
}

int main(int argc, char *argv[]) {
    global_t data;
    // first parse the arguments
    parse_args(argc, argv, data);
    // then run find on every path in paths
    for (auto &p : data.paths) {
        if (!fs::exists(p))
            run_err(p.string(), "No such file or directory", data);
        uint8_t i = 0;
        for (; data.links && i < UINT8_MAX && fs::is_symlink(p); ++i) {
            p = fs::read_symlink(p);
        }
        find(p, data);
    }
}
