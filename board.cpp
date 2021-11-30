#include "board.h"

position::position() {}
position::position(uchar nx, uchar ny): x(nx), y(ny) {}
	
bool position::valid(uchar board_size)
{
	return (x < board_size && y < board_size);
}

position::operator const string()
{
	return (char)(x + 1 + 0x61) + to_string(y + 1);
}

game_board::game_board(uchar b): board_size(b), move_count(0)
{
	if (b < 5 || b > BOARDSIZEMAX) throw invalid_board_size_exception();
	
	for (uchar i = 0; i < board_size; i++)
		for (uchar j = 0; j < board_size; j++)
			positions[i][j] = None;
}
	
bool game_board::domove(position mv)
{
	if (! mv.valid(board_size)) return false;
	if (move_count >= board_size*board_size) return false;
	if (positions[mv.x][mv.y] != None) return false;
	
	move_count++;
	moves[move_count - 1] = mv;
	positions[mv.x][mv.y] = (move_count % 2 != 0)? Black : White;
	
	return true;
}	

bool game_board::undo(ushort count)
{
	if (count > move_count || count < 0) return false;
	
	for (ushort i = move_count - 1; i > move_count - 1 - count; i--)
		positions[moves[i].x][moves[i].y] = None;
	
	move_count -= count;
	return true;
}

bool game_board::goback(ushort num)
{
	if (num < 0 || num > move_count) return false;
	
	for (ushort i = move_count - 1; i > num - 1; i--)
		positions[moves[i].x][moves[i].y] = None;
	
	move_count = num;
	return true;
}

ushort game_board::record_count()
{
	return move_count;
}

bool game_board::record_input(string &str)
{
	if (str.length() == 0) return false;
	
	uchar c; uchar x = 0; uchar y = 0;
	for(ushort i = 0; i <= str.length() - 1; i++) {
		c = str[i];
		
		if (0x30 <= c && c < 0x30 + 10) { //numeric. see ASCII table for reference
			if (x > 0) { //if x is 0, c should be sequence(round) number
				y *= 10;
				y += (c - 0x30);
			}
		} else {
			if (x != 0 && y != 0)
				if (! domove(position(x - 1, y - 1))) return false;
			x = 0; y = 0;
			if (0x61 <= c && c < 0x61 + BOARDSIZEMAX) //lower case letter
				x = c - 0x61 + 1;
			else if (0x41 <= c && c < 0x41 + BOARDSIZEMAX) //upper case letter
				x = c - 0x41 + 1;
		}
	}
			
	if (x != 0 && y != 0)
		if (! domove(position(x - 1, y - 1))) return false;
	
	return true;	
}

void game_board::record_output(string &str, bool show_round_num)
{
	if (move_count < 1) return;
	
	bool b = false;
	for (ushort i = 0; i < move_count; i++)
	{
		b = !b;
		if (show_round_num) {
			if (b) str += to_string((i + 1)/2 + 1) + ". ";
			else if (i > 0) str += ' ';
		} else {
			if (i > 0)
				str += b? "  ":" ";
		}
		str += (string) moves[i];
	}
}

void game_board::print(ostream &pout)
{
	for(char v = board_size - 1; v >= 0; v--) {
		if (v + 1 < 10) pout << " ";
		pout << (short) (v + 1);
		for(char h = 0; h < board_size; h++) {
			switch (positions[h][v]) {
				case None:
					if (v == 0 && h == 0)
						pout << "└";
					else if (v == 0 && h == board_size - 1)
						pout << "┘";
					else if (v == board_size - 1 && h == 0)
						pout << "┌";
					else if (v == board_size - 1 && h == board_size - 1)
						pout << "┐";
					else if (v == 0)
						pout << "┴";
					else if (v == board_size - 1)
						pout << "┬";
					else if (h == 0)
						pout << "├";
					else if (h == board_size - 1)
						pout << "┤";
					else
						pout << "┼";
						break;
				case Black:
					if (moves[move_count - 1].x == h && moves[move_count - 1].y == v)
					 	pout << "◆";
					else
					 	pout << "●";
					break;
				case White:
					if (moves[move_count - 1].x == h && moves[move_count - 1].y == v)
						pout << "⊙";
					else
						pout << "○";
					break;
				default:
					pout <<"！"; break;
			}
		}
		
		pout << endl;
	}
		
	pout << "  ";
	for (char h = 0; h < board_size; h++)
		pout << (char) (h + 0x61);
	pout << endl;
}

