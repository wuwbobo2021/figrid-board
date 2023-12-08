<div align="center">
  <img src="https://github.com/wuwbobo2021/figrid-board/blob/master/logo.png?raw=true">
</div>

# Figrid
Version 0.20 (2023-10-27)

A recording software for the Five-in-a-Row game which can run on Linux shell and is compatible with Renlib.

(Original Renlib software: <http://www.renju.se/renlib>, <https://www.github.com/gomoku/Renlib>, by Frank Arkbo)

Bugs fixed in v0.20:
1. Comment of the last node in the tree is not read (if it exists) when loading the Renlib file, thus it will be lost in the Renlib file modified by this progam;
2. Bug in search function: write at least two recordings h8 h9 (h9 must be leaf) then h8 i9, then switch to h8 h9 and enter `search`, then the i9 branch will be included in the result;
3. a few minor bugs.

If you have found bugs in this program, or you have any suggestion (especially suggestions about adding comments), please create an issue, or contact me.

## Main Features
1. A text based user interface showing the game board, supporting undo, goto and rotation;
2. Reading/Writing game records from/to commandline interface, PGN file, or record list file in which each single line represents a recording;
3. Creating, opening and saving Renlib library file, and mark/comment/position searching feature;
4. The library will rotate its recordings for you, and you can merge all variants existing in the library that can be rotated to get the same recording being queried, or even do automatic mergings under the root node (experimental);
5. A pipe interface for external programs, which can be opened with parameter `-p`. In this mode the board figure is not printed, and current recording string can be shown only by the command `output`.

## Building
```
git clone https://github.com/wuwbobo2021/figrid-board
cd figrid-board
make; ./figrid
```

## Lacking Features
- Calculate future move(s) by calling external engines like [Rapfi](https://github.com/dhbloo/rapfi) and [AlphaGomoku](https://github.com/MaciejKozarzewski/AlphaGomoku).
- Deal with the encoding (code page) setting in the terminal for comments in Renlib. Original Renlib on Windows stores comments in ANSI local page which is determined by Windows language version; this program does the same on Windows, but UTF-8 is used by default on Linux except you switch it manually.
- Support of more library formats like RenjuNet Database; rewrite this program in Rust with multi-thread support and provide a C library interface for Cython.
- A graphical user interfafce for this program. The current version might be uncomfortable to use because clicking on the printed board figure makes no sense, and the board printing feature depends on appropriate fonts (like SimSun, or Ubuntu Mono with Noto Mono fallback) and ambiguous-width characters setting of the terminal (which is not available in terminals based on Qt). Run this program with `-x` to enable ASCII XO board (which is forced on Windows).
- Big-endian mode CPUs are not supported because they were rarely used (see `RenlibNode` in `tree.cpp`).
