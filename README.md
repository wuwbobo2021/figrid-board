# Figrid
Version 0.15 "Plymouth Straw"

A recording software for the Five-in-a-Row game which can run on Linux shell and is compatible with Renlib.

(Original Renlib software: <http://www.renju.se/renlib>, <https://www.github.com/gomoku/Renlib>, by Frank Arkbo)

If you have found bugs in this program, or you have any suggestion (especially suggestions about adding comments), please pull an issue, or contact me.

## Main Features
1. A text based user interface showing the game board, supporting undo, goback and rotation;
2. Reading game records from input and from PGN files;
3. Creating, opening and saving Renlib library file, and mark/comment/position searching feature;
4. Auto rotation for querying: you don't have to rotate the board manually, but the library will rotate it's recordings for you;
5. A pipe interface for external programs which can be opened with parameter `-p`. Under this mode, the board figure is not printed, and current recording string can be shown only by the command `output`.

## Build
```
git clone https://github.com/wuwbobo2021/figrid-board
cd figrid-board
g++ -o <executable path> recording.cpp tree.cpp rule.cpp rule_original.cpp tui.cpp figrid.cpp main.cpp
```
If you have failed to build it on Windows MinGW, please report the problem.

## TODO
This list is for those who are willing to improbe this program. I will not do these things in a long time.

1. Communication with external engines (like AlphaGomoku: <https://github.com/MaciejKozarzewski/AlphaGomoku>).
2. Implementation of Renju and Swap2 rule for this program;
3. Support of RenjuNet Database; and implementation of a new library format for this program to support various board sizes.
4. Make a graphical user interfafce for this program.

