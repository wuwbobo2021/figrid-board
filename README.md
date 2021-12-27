# Figrid
##Version 0.10 "Innocent Coincidence"
A recording software for the Five-in-a-Row game which can run on Linux shell and is compatible with Renlib.
(Original Renlib software: <http://www.renju.se/renlib>, <https://www.github.com/gomoku/Renlib>, by Frank Arkbo, Sweden)
If you have found bugs in this program, or you have any suggestion (especially suggestions about adding comments), please pull an issue, or contact me.

##Main Features
1. A text based user interface showing the game board, supporting undo, goback and rotation;
2. Reading game records from input and from PGN files;
3. Creating, opening and saving Renlib library file;
4. Auto rotation for querying: you don't have to rotate the board manually, but the library will rotate it's recordings for you.

## Build
```
git clone https://github.com/wuwbobo2021/figrid-board
cd figrid-board
g++ -o <executable path> recording.cpp tree.cpp rule.cpp rule_gomoku_original.cpp tui.cpp platform_specific.cpp figrid.cpp main.cpp
```
If you have failed to build it on Windows MinGW, please report the problem.

## Known problems
This program cannot process Chinese comments in Renlib, and Chinese comments in Unicode created by this program cannot be read by the original Renlib program.

## TODO
1. This program should be able to communicate with external engines (like AlphaGomoku: <https://github.com/MaciejKozarzewski/AlphaGomoku>) through two pipes, one read and one write, probably this couldn't be done using C/C++ standard library functions;
2. Implementation of Renju and Swap2 rule for this program;
3. Support of RenjuNet Database; and implementation of a new library format for this program to support various board sizes.
4. Make a graphical user interfafce for this program.

