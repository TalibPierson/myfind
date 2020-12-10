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

/// provide operator<< for ostream, vector<std::string>
template<typename T>
ostream& operator<<(ostream &out, const vector<T> &paths) {
    if (paths.empty()) {
        out << '[';
        for (auto &path : paths) out << path << ", ";
        out << "\b\b]";
    }
    return out;
}

/// report a failure
void fail(const string &what) {
    perror(what.c_str());
    exit(EXIT_FAILURE);
}

/// open a directory stream
DIR *xopendir(const string &dirname) {
    DIR *dp = opendir(dirname.c_str());
    if (dp == nullptr) fail("opendir");
    return dp;
}

/// List subdirectories recursively
void ls_dir_r(const char *name) { DIR *dir_stream = opendir(name); }

pair<string, string> exec_parser(const string& s)
{
    if (s.find("-exec") == 0) {
        return pair<string, string>("exec", s.substr(0, s.find(';')));
    }
    return pair<string, string>();
}

int main(int argc, char *argv[]) {
    // Declare the supported desc.
    po::positional_options_description pos;
    pos.add("path", -1);
    boost::program_options::options_description desc("desc");
    desc.add_options()(
            "help", "Print the usage of myfind and exit.")(
            "version", "Print the myfind version number and exit.")(
            "path", po::value<vector<string>>()->default_value({"."}), "Path names.")(
            "L", "Follow symbolic links.");
    boost::program_options::options_description tests("tests");
    tests.add_options()(
            "name", po::value<string>(), "Base of file name matches shell pattern arg.")(
            "mtime", po::value<ssize_t>(), "File's data was last modified arg*24 hours ago.")(
            "type", po::value<char>(), "File is of type arg.");
    boost::program_options::options_description actions("actions");
    actions.add_options()(
            "exec", po::value<string>(), "Execute arg; true if 0 status is returned.")(
            "print","Print the full file name on the standard output.");
    desc.add(tests).add(actions);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
                      .options(desc)
                      .style(po::command_line_style::unix_style |
                             po::command_line_style::allow_long_disguise)
                      .positional(pos)
                      .extra_parser(exec_parser)
                      .run(),
              vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << "Usage: myfind [-L] [path...] [expression]" << "\n\n";
        cout << desc << '\n';
        return EXIT_SUCCESS;
    }
    if (vm.count("version")) {
        cout << "myfind (Talib Pierson) December 2020" << '\n';
        return EXIT_SUCCESS;
    }
    if (vm.count("path")) {
        cout << "path = {";
        for (auto &s : vm["path"].as<vector<string>>()) {
            cout << "    " << s << "\n";
        }
        cout << "}\n";
        return EXIT_SUCCESS;
    }
    if (vm.count("exec")) {
        cout << vm["path"].as<vector<string>>() << "\n";
        return EXIT_SUCCESS;
    }
}