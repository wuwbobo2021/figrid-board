// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_RECORDING_H
#define FIGRID_RECORDING_H

#include <iostream>

using namespace std;

namespace Figrid
{
const unsigned char Board_Size_Max = 26;
const unsigned char Board_Pos_Null = 0x1f; //2^5 - 1

enum Position_State: unsigned char
{
	Position_Empty = 0, Position_Black, Position_White
};

enum Position_Rotation //flip before rotate. 1 bit for flipping, 2 lower bits for rotation
{
	Rotate_None = 0,              //0 00, don't rotate
	Rotate_Clockwise = 1,         //0 01, rotate 90 degrees
	Rotate_Central_Symmetric = 2, //0 10, equals rotate 180 degrees clockwise
	Rotate_Counterclockwise = 3,  //0 11, equals rotate 270 degrees clockwise
	
	Reflect_Horizontal = 4,       //1 00
	Reflect_Vertical = 6,         //1 10, equals Reflect_Horizontal | Rotate_Central_Symmetric
	Reflect_Left_Diagonal = 5,    //1 01, equals Reflect_Horizontal | Rotate_Clockwise
	Reflect_Right_Diagonal = 7    //1 11, equals Reflect_Horizontal | Rotate_Counterclockwise
};

Position_Rotation conbine_rotation(Position_Rotation r1, Position_Rotation r2);
Position_Rotation reverse_rotation(Position_Rotation r);

struct Move
{
	unsigned char x: 5;
	unsigned char y: 5;
	bool swap: 1; //if the current mover chooses the color of the next move, it's false; otherwise it's true
	
	Move(); //create a null move
	Move(unsigned char nx, unsigned char ny);
	Move(bool sw);
	
	bool operator==(const Move&) const;
	bool operator!=(const Move&) const;
	operator string() const;

	// make sure board_size is valid
	bool position_valid(unsigned char board_size) const;
	bool pos_is_null() const;
	void rotate(unsigned char board_size, Position_Rotation rotation);
	Position_Rotation standardize(unsigned char board_size); //for storing into the library
};

Move read_single_move(const string& str); //Note: not optimized

class InvalidBoardSizeException: public exception {};

enum Board_Row_Direction
{
	Board_Row_Horizontal = 0,
	Board_Row_Vertical = 1,
	Board_Row_Left_Diagonal = 2,
	Board_Row_Right_Diagonal = 3
};

class Recording
{
	Move* moves; //array of moves
	unsigned short cnt = 0, cnt_all = 0;
	unsigned char cnt_null_pos = 0;

	Position_State** positions; //grid array
	
	void refresh_positions();
	void copy_from(const Recording& from);
	
public:
	const unsigned char board_size;
	const unsigned short pos_count_max, moves_count_max;
	
	Recording(unsigned char sz);
	Recording(const Recording&); //copy
	Recording& operator=(const Recording&); //copy
	~Recording();
	
	unsigned short count() const;
	unsigned short count_all() const; //including redoable count
	Move operator[](unsigned short i) const; //the returned value is not a reference
	Move last_move() const;
	Position_State color_next() const;
	
	bool operator==(const Recording&) const;
	bool operator!=(const Recording&) const;
	
	bool domove(Move mv);
	bool append(const Recording* record);
	bool undo(unsigned short steps = 1);
	bool redo(unsigned short steps = 1);
	bool goto_num(unsigned short num);
	bool goto_move(Move mv, bool back = false);
	void clear();

	bool input(istream& ist, bool multiline = true);
	void output(ostream& ost, bool show_round_num = false) const;
	
	// meaning of `index` (counts from 0) in function `board_get_row`:
	//     Horizontal: {a1, b1...o1} , {a2, b2...o2} ...{a15, b15...o15}
	//       Vertical: {a1, a2...a15}, {b1, b2...b15}...{o1,  o2... o15}
	//  Left Diagonal: {a15}, {a14, b15}, {a13, b14, c15}...{n1 , o2} , {o1}
	// Right Diagonal: {a1} , {a2 , b1} , {a3,  b2,  c1} ...{n15, o14}, {o15}
	bool board_is_filled() const;
	unsigned char board_diagonals_count() const; //returns the amount of diagonals on the board, including those length 1 corners
	unsigned char board_get_row(Board_Row_Direction direction, unsigned char index, Position_State* po) const;
	unsigned char board_get_row(Board_Row_Direction direction, Move pos, Position_State* po) const;
	void board_rotate(Position_Rotation rotation);
	void board_print(ostream &ost, bool use_ascii = false, Move* pdots = NULL, unsigned short dots_count = 0) const;

	const Move* recording_ptr() const;
	const Position_State** board_ptr() const;
};

}
#endif
