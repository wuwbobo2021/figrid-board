// Figrid v0.11
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_RECORDING_H
#define FIGRID_RECORDING_H

#include <iostream>

using namespace std;

namespace Namespace_Figrid
{
const unsigned char Board_Size_Max = 0x1f; //2^5 - 1

enum Position_State: unsigned char
{
	Position_Empty = 0, Position_Black, Position_White
};

enum Position_Rotation: unsigned char //5 higher bits are reserved
{
	Rotate_None = 0,              //00000000, don't rotate
	Rotate_Clockwise = 1,         //00000001, rotate 90 degrees
	Rotate_Counterclockwise = 3,  //00000011, equals rotate 270 degrees clockwise
	Rotate_Central_Symmetric = 2, //00000010, equals rotate 180 degrees clockwise
	
	Reflect_Horizontal = 4,       //00000100
	Reflect_Vertical = 6          //00000110, equals Rotate_Central_Symmetric | Reflect_Horizontal
};

struct Move
{
	unsigned char x: 5;
	unsigned char y: 5;
	bool swap: 1; //if the current mover chooses the color of the next move, it's false; otherwise it's true
	
	Move();
	Move(unsigned char nx, unsigned char ny);
	Move(bool sw);
	
	bool operator==(const Move&) const;
	bool operator!=(const Move&) const;
	operator string() const;

	bool position_valid(unsigned char board_size) const;
	bool position_null() const;
	void rotate(unsigned char board_size, Position_Rotation rotation, bool rotate_back = false);
};
const Move Null_Position_Move = Move(Board_Size_Max, Board_Size_Max);

class Invalid_Board_Size_Exception: exception {};

enum Board_Row_Direction
{
	Board_Row_Horizontal = 0,
	Board_Row_Vertical = 1,
	Board_Row_Left_Diagonal = 2,
	Board_Row_Right_Diagonal = 3
};

class Recording
{
	Position_State** positions; //two-dimension array
	Move* moves; //array of moves
	unsigned short count = 0;
	unsigned char null_pos_count = 0;
	
	void refresh_positions(); //low-level function, useless outside
	void copy(const Recording& from);
	
public:
	const unsigned char board_size;
	const unsigned short moves_count_max;
	
	Recording(unsigned char sz);
	Recording(const Recording&);
	Recording& operator=(const Recording&);
	~Recording();
	
	bool operator==(const Recording&) const;
	bool operator!=(const Recording&) const;
	
	bool domove(Move mv);
	bool append(const Recording* record);
	bool undo(unsigned short steps = 1);
	bool goback(unsigned short num);
	void clear();
	
	Move last_move() const;
	Position_State color_next() const;
	unsigned short moves_count() const;
	
	const Move* recording_ptr() const;
	bool input(istream& ist);
	void output(ostream& ost, bool show_round_num = false) const;
	
	// meaning of `index` (counts from 0) in function `board_get_row`):
	//     Horizontal: {a1, b1...o1} , {a2, b2...o2} ...{a15, b15...o15}
	//       Vertical: {a1, a2...a15}, {b1, b2...b15}...{o1,  o2... o15}
	//  Left Diagonal: {a15}, {a14, b15}, {a13, b14, c15}...{n1 , o2} , {o1}
	// Right Diagonal: {a1} , {a2 , b1} , {a3,  b2,  c1} ...{n15, o14}, {o15}
	const Position_State** board_ptr() const;
	unsigned char board_diagonals_count() const; //returns the amount of diagonals on the board, including those length 1 corners
	unsigned char board_get_row(Board_Row_Direction direction, unsigned char index, Position_State* po) const;
	unsigned char board_get_row(Board_Row_Direction direction, Move pos, Position_State* po) const;
	void board_rotate(Position_Rotation rotation, bool rotate_back = false);
	void board_print(ostream &ost, unsigned short dots_count = 0, Move* pdots = NULL) const;
};

}
#endif
