## Project 4: myfind
Talib Pierson
December 2020

https://shapeshed.com/unix-find/
https://en.cppreference.com/w/cpp/filesystem

Support the following command line options:

*`-name` pattern (including wildcards)  
*`-mtime` n (to simplify, I will only test with n=0, so don't bother with negatives or plus sign).  
*`-type` t  
*`-exec` command (only the ; variant).  
*`-print`  
*`-L` (follow symbolic links)  

## Notes and hints:

    Your output should be 100% identical to that of find on Linux for the supported options, including error messages and edge cases. I will use a testing script to verify this.
    If you're using C++-17, you may use the filesystem library. And ff you're using g++ version 8 you'll have to add -lstdc++fs to your link instruction, and even then support for the library appears a little iffy (the path class appears to be non-copiable, which you can work around).
    find can take zero or more starting points. If zero are passed, . is assumed.
    If no -print OR -exec operations are passed as argument then -print is assumed (to test, simply try ./myfind .).
    The OS shell normally expands wildcards and special characters like * and ; on its own. To pass them explicitly to myfind, you'll need to use either quotation marks or backslash.
    The man page for find offers hints on how to do wildcard matching correctly and easily.
    Remember that a file matches only if all the concurrent options to find agree (are true), including a zero output from any -exec options. But you don't need to support any of find's logical operators such as -not.
    It is important that you test all these options (and their combinations), as well as different file/directory scenarios. Compare your output to find's. That's how I'll test your code, buy diffing its output against many find outputs of varied scenarios.
    You may share ideas for test cases among yourselves, but no source code.
    You only need to support the n parameter for -mtime, not +n and not -n. See here for more details.
    Also note that std::filesystem::last_write_time does not have an option to not follow symlinks. This leads to differing behavior when passing -mtime without -L. To allow you to use std::filesystem, I will always add -L when testing the -mtime predicate.

## Testing

For testing, I'll use the following script. It assumes a certain directory tree (under tstdir/ with its own subdirectories) and files that I created and is unknown to you. You can create your own normal files and wild edge cases. I've only included the first five examples I'll tests. In my tests there'll be about two dozen more cases that are also unknown to you. Exercise your imagination!

#!/usr/bin/env bash

g++-8 -O3 -Werror -Wall -Wextra -pedantic  -std=c++17 myfind.cc -o test_find -lstdc++fs || exit

for bin in find ./test_find
do
  echo Running tests with ${bin}
  mkdir ${bin}.out &> /dev/null
  rm ${bin}.out/*

  (cd tstdir; ${bin}; cd ..) &> ${bin}.out/no-args.out
  ${bin} tstdir &> ${bin}.out/one-arg.out
  ${bin} tstdir tstdir &> ${bin}.out/one-arg-dup.out
  ${bin} tstdir tstdir/dir1 &> ${bin}.out/two-args.out
  ${bin} nodir &> ${bin}.out/nodir.out
done

###############

echo "Comparing test outputs"

for out in find.out/*.out
do
  echo TESTING: $out ...
  diff $out ${bin}.out
done

