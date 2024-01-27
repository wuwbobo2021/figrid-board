<div align="center">
  <img src="https://github.com/wuwbobo2021/figrid-board/blob/main/logo.png?raw=true">
</div>

# Figrid
Version 0.20 (2024-1-27)

A recording software for the Five-in-a-Row game which can run on Linux shell and is compatible with Renlib.

(Original Renlib software: <http://www.renju.se/renlib>, <https://www.github.com/gomoku/Renlib>, by Frank Arkbo)

Bugs fixed in v0.20 (2023-10-25):
1. Comment of the last node in the tree is not read (if it exists) when loading the Renlib file, thus it will be lost in the Renlib file modified by this progam;
2. Bug in search function: write at least two recordings h8 h9 (h9 must be leaf) then h8 i9, then switch to h8 h9 and enter `search`, then the i9 branch will be included in the result;
3. a few minor bugs.

Bug fixed in v0.20 (2024-1-27): Auto-rotation feature is lost after the 5th move.

If you have found bugs in this program, or you have any suggestion (especially suggestions about adding comments), please create an issue, or contact me.

Thanks to [nguyencongminh090](https://github.com/nguyencongminh090) for reporting bugs and making suggestions, he might become the owner of this repository in the future.

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

## Program Structure
A few description is provided here to explain things that might not be quickly understood by viewing the code, hopefully it is better than nothing.

Note that the current structure is far from being perfect (see "Lacking Features" at the bottom). It should be reconstructed to support multi-thread programming, and it might be rewritten in Rust. But I might not to do all these things in a long time, except someone is really interested in this program and asks me to improve it.

### Move
Represents a single move in a game record. It is meaningless outside `Recording`.

Contains unsigned `x`, `y`, and a boolean flag `swap`. When `x` and `y` are not the maximum value 31, the move represents a stone placed on coordinate (x, y), given the coordinate of the bottom-left corner (0, 0). When `x` and `y` are both 31, the move represents a null (pass) move, and the swap flag might make sense.

Enum `Position_State` is defined to represent the state of a position on the board: empty, black stone placed, or white stone placed. But it is not used by `Move` itself.

Enum `Position_Rotation` is defined to cover all 8 kinds of rotation/reflection (including no rotation) calculations of the position, which is implemented by `Move::rotate()` which requires the board size as an argument. Two such enums can be summed by `conbine_rotation()`, but it is not commutative; such enum can be reversed by `reverse_rotation()`.

To support the auto merging feature, there is a so called `Move::standardize()` function which puts the move into a triangular area by rotation and reflection, vertexes of the triangle are: the center of the board, the top-right corner, and the middle of the right border.

### Recording
Stores a game record by an array `moves`; maintaines the corresponding board matrix `positions` which should not be modified directly from outside. Used by `Tree` to maintain the board status for the route from root to the current cursor; used by `Rule` for rule checking; used by `Session` to store the current status.

The board is always a square in current implementation; the maximum board size `Board_Size_Max` is set to 26 for the record string format followed by `Recording::input()` and `Recording::output()`: each move (not null) has a notation of a lower-case latin letter for x combined with a integer string for y, for example, a `Move` with coordination (0, 0) has a notation `a1`.

Size of array `moves` is determined by the square of given board size plus 32 (for possible null moves) when `Recording` is constructed; once constructed, the board size cannot be changed anymore. the stone color for each `Move` is not stored in the `Move` itself, instead it is calculated by the parity of the index, without caring for existion of null moves.

`cnt` stores the current count of moves in the current record "as is"; `cnt_null_pos` stores the count of null moves, which is also counted by `cnt`; `cnt_all` maintaines a historical maximum count just for the redoing feature, it is made equal to `cnt` when a new stone is placed.

The board matrix `positions` of type `Position_State` is also created under the construction of `Recording`. Here the concept of board is "confused" with the recording; member functions with prefix `board_` are all operations related to the board; all member functions make sure the board is synchronized with the recording.

Currently it isn't well-designed for Swap2 opening record. Swap openings can be entered in such format: "xx xx xx black/white" (of the second mover's choice in 4th item), or "xx xx xx xx xx black/white" (of the first mover's choice in 6th item). "-" can always be entered to add a null move without swapping. Unfortunately, the swap flag cannot be saved in a Renlib file.

### Tree
Stores lots of move sequences (game records) by a tree structure. Keeps a cursor for the current node to be observed. Used by `Session` to keep the loaded/unsaved Renlib library in RAM.

Each node has the type `Node`, which containes a `pos` of type `Move`, a mark flag `marked`, a "start" position mark flag `marked_start`, boolean flag `has_comment`, and unsigned integer `tag_comment` which stores the index in vector `comments` of the `Tree`; it has two `Node*` pointers, `down` for its left descendent, `right` for its right sibling. A pointer is set to `NULL` if the left descendent or right sibling doesn't exist.

In class `Tree`, there is a `Node*` pointer `root`, which has a null move, corresponding with the root node in a "non-standard" Renlib library; another `Node*` pointer `cur` stores the cursor's current position;

An array of `Node*` pointer named `seq` stores the route from root to current node, seq[0] always points to the root node; a short integer `cur_depth` stores the current depth, which becomes 0 when the cursor goes back to the root; `Recording rec` keeps the board status for the current route.

Comments are all stored in `vector<string> comments`. Note that encoding issues are not solved (see "Lacking Features" at the bottom).

The `Tree` supports querying of a `Move` under the cursor node, as well as querying of a `Recording` starting from the root node. In the second case rotations are tried to match as deep as possible; if the query is successful, `flag_rotate` will store how the board of the `Recording` that has been queried should be rotated to match into the existing route (move sequence) in the `Tree`; then functions like `Tree::current_move()`, `Tree::get_current_recording()` will rotate it back according to the original query by default.

`Tree::write_recording()` writes a given recording starting from the root node: it will do a querying at first, then it will rotate the given recording according to `flag_rotate` before writing (by default). This function is based on `Tree::write_move()` which doesn't do rotation.

#### Renlib
The board size in Renlib is always 15x15, so is the current Figrid executable program (for compatibility). Convention of little-endian bit order is followed here in the description.

The first 20 bytes of data in the Renlib file compose the file header, which keeps the same in every Renlib file created by the latest version of Renlib software. This header sequence is kept by Figrid, defined as `Renlib_Header`.

After the 20 bytes header, each node consists of 2 bytes (plus the length of comment, if exist), defined as `RenlibNode` here. In the first byte, bits 3:0 is for x, while bits 7:4 is for y; if they are both 0, it represents a null move; otherwise the `Figrid::Move` format of coordination (x, y) becomes (x + 1, 14 - y) in this byte.

In the second byte, all bits are boolean flags: the highest bit 7 is `has_sibling`, which is true if it has a right sibling node; bit 6 is `is_leaf`, which is false if it has one or more child node(s). The bit 4 is the mark flag; bit 2 is the "start" position mark flag; bit 3 is the comment flag.

If a node's comment flag is true, then an ANSI comment string is at the right of the second byte of the node. If the comment is multiline, then there would be a '\b' at the end of the first line, the others would be "\r\n". The end of a comment string must be marked with one or more '\0'.

Nodes in the Renlib file are stored in preorder sequence. If the first node in the file contains a null move, it is considered "non-standard", otherwise the first node contains a move at the board center h8. The tree structure can be rebuilt in memory according to the 2 marks `is_leaf` and `has_sibling` in the second byte of each node:

After a `Figrid::Tree` object is initialized, The root `Node` of the `Tree` is set to a null position;

the Renlib file header is skipped; the local cursor inside function `Tree::load_renlib()` is placed on the root if the Renlib is "non-standard", or else it's placed on the left descendant of the root, so that the root of `Figrid::Tree` can always be null;

then, at the beginning of each loop, 2 bytes of data is read as a `RenlibNode`, which is converted to a `Node` at the cursor;

if this node has a left descendant, the cursor's `down` pointer will point to the next node to be read, and the cursor itself will be pushed into the node stack if it has a right sibling;

if this node is a leaf node but has a right sibling node, the cursor's `right` pointer will point to the next node to be read;

if this node is a leaf node and doesn't have a right sibling node, it means the descendants of the top of the node stack were read, thus it will be popped out, its `right` pointer will be found and will point to the next node to be read; the read progress is finished once the node stack is found empty here.

the cursor is set to the next node to be read at the end of each loop.

With above description, function `Tree::save_renlib()` should be easy to understand, so tautology is avoided here. Renlib-related functions requires the board size to be exactly 15.

### Rule
Holds a pointer to a `Recording`, determines the game state by checking every entered move; the whole `Recording` can also be checked at once. Used by `Session` to make sure the entered record doesn't break the game rule.

Stores the pointer `crec` pointing to the `Recording`, game state flag `cstatus` of enum type `Game_Status`, unsigned short integer `invalid_count`, they are all protected members that can be used by rule implementations inherited from this class.

In enum type `Game_Status`, items are actually bit flags in the byte. It stores the color of the next stone by `Game_Status_Black` and `Game_Status_White` (the only two items that shouldn't be ORed); it stores permissions for each player to move by `Game_Status_First_Mover` and `Game_Status_Second_Mover`, they also tell the winner when the game is ended; permission for the current player to swap is stored by `Game_Status_Choice`; `Game_Status_Ended` tells whether the game is ended; `Game_Status_Foul` tells whether the rule is broken.

`crec` is initialized by `Rule::set_recording()`, which checks the record at once while being called. `Rule::check_recording()` creates a copy of the current record, then the current record is cleared, moves are read from the copy and are checked one by one; If the rule is broke by a move, then `cstatus` is set to `Game_Status_Foul`, `invalid_count` is set to the count of remaining moves starting from this move, then the bad recording is recovered from the copy; these moves can be deleted by `Rule::undo_invalid_moves()`.

`Rule::domove()` checks the validity of every entered move; it is the only function that needs to be implemented by a class inherited from this class.

#### RuleOriginal
Currently it is the only implemented rule. Under this rule, the black stone is always placed by the first mover, the white stone is always placed by the second player; the swap flag of the move is unconcerned here. Exactly 5 stones of the same color are required to end the game.

Checks each move (not null) by scanning the horizontal, vertical and two diagonal lines passing through the coordination of the move, from one end point to another. If 5 stones of the same color are found without spacing, then the game is considered ended, and the winner is determined by the color of this line of stones.

### Session
This is a manager that stayes behind `FigridUI`, keeping the status of the executable program. Currently the information string for the current node in the `Tree` is also generated here, not in `FigridTUI`.

The board size and a pointer to a `Rule` are required to construct a session, this pointer is stored as `p_rule`; then the game record `rec` and the `tree` are created.

`mode` of enum type `Session_Mode` can be recording mode (`Session_Mode_None`), library reading mode, or library writing mode. Library-related operations are disabled under recording mode, and operations that modify the tree are disabled under library reading mode.

### FigridUI
This is merely an interface class, but it has a protected member: a pointer of the `Session` object.

Function `FigridUI::run()` in the interface represents the loop context of the executable program, it should return an integer (usually 0) for `int main()`. `FigridUI::refresh()` should update the user interface by the status of `Session`.

#### FigridTUI
Currently it is the only implemented user interface, which works in a terminal (or console).

In `FigridTUI::run()` loop, a command is parsed and executed each time a line is entered, then `FigridTUI::refresh()` is called to clear the terminal screen and print the new board figure, record string and node information (if a library is opened). Some commands were considered to be "safe" that they can be reexecuted when another Enter is pressed. Enter "help", "h", or "?" for help.

## Lacking Features
- Rewrite this program in Rust with multi-thread support and (possibly) provide an interface for Cython.
- Calculate future move(s) by calling external engines like [Rapfi](https://github.com/dhbloo/rapfi) and [AlphaGomoku](https://github.com/MaciejKozarzewski/AlphaGomoku).
- Deal with the encoding (code page) setting in the terminal for comments in Renlib. Original Renlib on Windows stores comments in ANSI local page which is determined by Windows language version; this program does the same on Windows, but UTF-8 is used by default on Linux except you switch it manually.
- Support more library formats like RenjuNet Database.
- A graphical user interfafce for this program. The current version might be uncomfortable to use because clicking on the printed board figure makes no sense, and the board printing feature depends on appropriate fonts (like SimSun, or Ubuntu Mono with Noto Mono fallback) and ambiguous-width characters setting of the terminal (which is not available in terminals based on Qt). Run this program with `-x` to enable ASCII XO board (which is forced on Windows).
- Big-endian mode CPUs are not supported because they were rarely used (see `RenlibNode` in `tree.cpp`).
