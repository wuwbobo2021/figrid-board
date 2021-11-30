#include <iostream>

using namespace std;

typedef unsigned short ushort;
typedef unsigned char uchar;

enum position_status: uchar
{
	None = 0, Black = 1, White = 2
};

struct position
{
	uchar x;
	uchar y;
	
	position();
	position(uchar nx, uchar ny);
	
	bool valid(uchar board_size);
	operator const string();
};

const ushort BOARDSIZEMAX = 19;

class invalid_board_size_exception: exception {};

class game_board
{
	position_status positions[BOARDSIZEMAX][BOARDSIZEMAX];
	position moves[BOARDSIZEMAX*BOARDSIZEMAX];
	
	uchar board_size;
	ushort move_count;

public:
	game_board(uchar b);
	
	bool domove(position mv);
	bool undo(ushort count = 1);
	bool goback(ushort num);
	
	ushort record_count();
	bool record_input(string &str);
	void record_output(string &str, bool show_round_num = false);
	
	void print(ostream &pout);
};
