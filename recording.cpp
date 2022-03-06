// Figrid v0.15
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#include <cstring>
#include <cctype>
#include "recording.h"

using namespace std;
using namespace Namespace_Figrid;

const unsigned short Null_Pos_Count_Max = 32;

Move::Move(): x(Board_Pos_Null), y(Board_Pos_Null), swap(false) {}
Move::Move(unsigned char nx, unsigned char ny): x(nx), y(ny), swap(false) {}
Move::Move(bool sw): x(Board_Pos_Null), y(Board_Pos_Null), swap(sw) {}

bool Move::operator==(const Move& p) const
{
	if (this->position_null() && p.position_null())
		return this->swap == p.swap;
	else
		return this->x == p.x && this->y == p.y;
}

bool Move::operator!=(const Move& p) const
{
	if (this->position_null() && p.position_null())
		return this->swap != p.swap;
	else
		return this->x != p.x || this->y != p.y;
}

Move::operator string() const
{
	if (this->position_null()) {
		if (! this->swap) return "-";
		else return "sw";
	}
	if (this->x > Board_Size_Max - 1 || this->y > Board_Size_Max - 1) return "??";
	return (char)(this->x + 0x61) + to_string(this->y + 1); //plus 0x61 to get the letter
}

bool Move::position_valid(unsigned char board_size) const
{
	return this->x < board_size && this->y < board_size;
}

bool Move::position_null() const
{
	return this->x == Board_Pos_Null && this->y == Board_Pos_Null;
}

void Move::rotate(unsigned char board_size, Position_Rotation rotation, bool rotate_back)
{
	if (this->position_null()) return;
	
	// Notice: the topmost x and rightmost y are both board_size - 1.
	if (!rotate_back && rotation >> 2) //Reflect_Horizontal bit is true
		this->x = (board_size - 1) - this->x;
	
	unsigned char ro = rotation & 3; //get two lower bits
	if (ro == 0) return;
	if (rotate_back) ro = 4 - ro; //rotate 90 degrees clockwise for 4 - ro times equals 360 degrees
	
	unsigned char xo = this->x, yo = this->y;
	switch (ro) {
		case Rotate_Clockwise:
			this->x = yo;                     this->y = (board_size - 1) - xo;  break;
		case Rotate_Counterclockwise:
			this->x = (board_size - 1) - yo;  this->y = xo;  break;
		case Rotate_Central_Symmetric:
			this->x = (board_size - 1) - xo;  this->y = (board_size - 1) - yo;  break;
	}
	
	if (rotate_back && rotation >> 2)
		this->x = (board_size - 1) - this->x;
}


void Recording::refresh_positions() //low level operation clearing array `positions`
{
	for (unsigned char i = 0; i < this->board_size; i++)
		for (unsigned char j = 0; j < this->board_size; j++)
			this->positions[i][j] = Position_Empty;
	
	for (unsigned short i = 0; i < this->count; i++) {
		if (this->moves[i].position_null()) continue;
		this->positions[moves[i].x][moves[i].y] = (i % 2 == 0)? Position_Black : Position_White;
	}
}

void Recording::copy(const Recording& from)
{
	if (from.board_size > this->board_size) throw Invalid_Board_Size_Exception();

	const Move* pf = from.recording_ptr();
	for (unsigned short i = 0; i < from.moves_count(); i++) {
		this->moves[i] = pf[i];
		if (this->moves[i].position_null()) this->null_pos_count++;
	}

	this->count = from.moves_count();
	this->refresh_positions();
}

Recording::Recording(unsigned char sz): board_size(sz), moves_count_max(sz * sz)
{
	if (sz < 5 || sz > Board_Size_Max) throw Invalid_Board_Size_Exception();
	
	this->moves = new Move[moves_count_max + Null_Pos_Count_Max];
	
	this->positions = new Position_State*[board_size];
	for (unsigned char i = 0; i < this->board_size; i++)
		this->positions[i] = new Position_State[board_size];
	
	this->refresh_positions();
}

Recording::Recording(const Recording& from): board_size(from.board_size), moves_count_max(from.moves_count_max)
{
	this->moves = new Move[moves_count_max + Null_Pos_Count_Max];
	this->count = 0;
	
	this->positions = new Position_State*[board_size];
	for (unsigned char i = 0; i < this->board_size; i++)
		this->positions[i] = new Position_State[board_size];

	this->copy(from);
}

Recording::~Recording()
{
	delete[] this->moves;
	
	for (unsigned char i = 0; i < this->board_size; i++)
		delete[] this->positions[i];
	delete[] this->positions;
}

Recording& Recording::operator=(const Recording& rec)
{
	this->copy(rec);
	return *this; //actually converted `this` from pointer to reference
}

bool Recording::operator==(const Recording& rec) const
{
	if (rec.board_size != this->board_size) return false;
	if (rec.moves_count() != this->count) return false;
	
	const Move* pr = rec.recording_ptr();
	for (unsigned short i = 0; i < this->count; i++)
		if (pr[i] != this->moves[i]) return false;
		
	return true;
}

bool Recording::operator!=(const Recording& rec) const
{
	return !(*this == rec);
}

bool Recording::domove(Move mv)
{
	if (mv.position_null()) {
		if (this->null_pos_count >= Null_Pos_Count_Max) return false;
		this->moves[this->count++] = mv;
		this->null_pos_count++;
		return true;
	}
	
	if (! mv.position_valid(this->board_size)) return false;
	if (this->positions[mv.x][mv.y] != Position_Empty) return false;
	
	this->positions[mv.x][mv.y] = this->color_next();
	this->moves[this->count++] = mv;
	
	return true;
}	

bool Recording::append(const Recording* record)
{
	bool suc = true;
	const Move* prec = record->recording_ptr();
	for (unsigned short i = 0; i < record->moves_count(); i++)
		if (! this->domove(prec[i])) suc = false;
	
	return suc;
}

bool Recording::undo(unsigned short steps)
{
	if (steps > this->count) return false;
	
	for (short i = this->count - 1; i > this->count - 1 - steps; i--) {
		if (! moves[i].position_null())
			this->positions[moves[i].x][moves[i].y] = Position_Empty;
		else
			this->null_pos_count--;
	}
	
	this->count -= steps;
	return true;
}

bool Recording::goback(unsigned short num)
{
	return this->undo(this->count - num);
}

void Recording::clear()
{
	this->count = 0; this->null_pos_count = 0;
	this->refresh_positions();
}

Move Recording::last_move() const
{
	if (this->count == 0) return Move(); //null move
	return this->moves[this->count - 1];
}

Position_State Recording::color_next() const
{
	return (this->count % 2 == 0)? Position_Black : Position_White;
}

unsigned short Recording::moves_count() const
{
	return this->count;
}

const Move* Recording::recording_ptr() const
{
	return this->moves;
}

bool Recording::input(istream& ist)
{
	unsigned int llmax = this->board_size * 13 + 10; 
	string str = ""; char buf[llmax]; char cch;
	while (ist && (! ist.eof())) {
		ist.getline(buf, llmax);
		if (strlen(buf) == 0) continue; //blank line
		if (buf[0] == '[') continue; //TODO: improve the class `Recording` to load more info from PGN texts
		break;
	}
	while (true) {
		str += (string) buf; str += " ";
		
		if (!ist || ist.eof()) break;
		ist >> cch;
		if (cch == '[') {ist.putback(cch); break;} // that should be the beginning of the next PGN recording
		ist.putback(cch);
		
		ist.getline(buf, llmax);
		if (strlen(buf) == 0) break; //blank line
	}
	
	unsigned char c; unsigned char x = 0, y = 0;
	for(unsigned short i = 0; i <= str.length() - 1; i++) {
		c = str[i];
		
		if (isdigit(c)) {
			if (x > 0) { //if x is 0, c should be sequence(round) number
				y *= 10;
				y += (c - 0x30); //see ASCII table
			} else
				if (i < str.length() - 1 && str[i + 1] == '-') return true;
		} else {
			if (x != 0 && y != 0) {
				if (! this->domove(Move(x - 1, y - 1))) return false;
				x = 0; y = 0;
			}
			if (isalpha(c)) { //is letter
				// it's second mover's choice in 4th item, and in case of the second mover then choose to do two moves,
				// the next move will be white, so it's still a swap choice for the first mover to choose black in 6th item.
				if (this->count == 4 - 1 || this->count == 6 - 1) {
					if (tolower(c) == 'b' && i + 4 <= str.length() - 1 && str[i + 1] == 'l') {
						this->domove(Move(true)); i += 4; continue;
					} else if (tolower(c) == 'w' && i + 4 <= str.length() - 1 && str[i + 1] == 'h') {
						this->domove(Move(false)); i += 4; continue;
					}
				}
				if (islower(c)) x = c - (islower(c)? 0x61 : 0x41) + 1;
			} else if (c == '-') //pass
				if (! this->domove(Move(false))) return false;
		}
	}
			
	if (x != 0 && y != 0)
		if (! this->domove(Move(x - 1, y - 1))) return false;
	
	return true;	
}

void Recording::output(ostream& ost, bool show_round_num) const
{
	if (this->count < 1) {ost << "(Empty Recording)"; return;}
	
	bool black = true;
	for (unsigned short i = 0; i < this->count; i++)
	{
		if (black) {
			if (i > 0) cout << " ";
			if (show_round_num) ost << to_string((i + 1)/2 + 1) + ". ";
		} else
			cout << " ";

		if (this->moves[i].position_null() && (i == 4 - 1 || i == 6 - 1))
			ost << (moves[i].swap? "black" : "white");
		else
			ost << (string) this->moves[i];
		
		black = !black;
	}
}

const Position_State** Recording::board_ptr() const
{
	return (const Position_State**) this->positions;
}

unsigned char Recording::board_diagonals_count() const
{
	return this->count * 2 - 1;
}

unsigned char Recording::board_get_row(Board_Row_Direction direction, unsigned char index, Position_State* po) const
{
	if (direction == Board_Row_Horizontal || direction == Board_Row_Vertical)
		if (index > this->board_size - 1) return 0;
	else
		if (index > this->board_diagonals_count() - 1) return 0;
	
	Move pc; unsigned char cnt = 0; //starting point of the diagonal line, current output count
	switch (direction) {
		case Board_Row_Horizontal:
			for (unsigned char i = 0; i < this->board_size; i++)
				po[i] = this->positions[i][index];
			return this->board_size;
		
		case Board_Row_Vertical:
			for (unsigned char i = 0; i < this->board_size; i++)
				po[i] = this->positions[index][i];
			return this->board_size;
		
		case Board_Row_Left_Diagonal:
			if (index <= this->board_size - 1) //the line starts from left boundary of the board
				pc = Move(0, this->board_size - 1 - index);
			else
				pc = Move(index - this->board_size + 1, 0);
			
			while (pc.x < this->board_size && pc.y < this->board_size) {
				*po = this->positions[pc.x][pc.y];
				cnt++; pc.x++; pc.y++; po++;
			}
			return cnt;
		
		case Board_Row_Right_Diagonal:
			if (index <= this->board_size - 1) //the line starts from the bottom of the board
				pc = Move(0, index);
			else
				pc = Move(index - this->board_size + 1, this->board_size);
			
			while (pc.x < this->board_size) {
				*po = this->positions[pc.x][pc.y];
				cnt++; if (pc.y == 0) break;
				pc.x++; pc.y--; po++;
			}
			return cnt;
	}
	
	return 0; //it should be unreachable
}

unsigned char Recording::board_get_row(Board_Row_Direction direction, Move pos, Position_State* po) const
{
	if (! pos.position_valid(this->board_size)) return 0;
	
	unsigned char index;
	switch (direction) {
		case Board_Row_Horizontal:
			index = pos.y; break;
		case Board_Row_Vertical:
			index = pos.x; break;
		case Board_Row_Left_Diagonal:
			// if y > x, index = (board_size - 1) - (y - x), equals the expression below
			index = (this->board_size - 1) + pos.x - pos.y; break;
		case Board_Row_Right_Diagonal:
			// simplified from:  let ry = (board_size - 1) - y, then
			// index = ry > x?  (board_size - 1) - (ry - x)  :  (board_size - 1) + (x - ry)
			index = pos.x + pos.y;
	}
	
	return this->board_get_row(direction, index, po);
}

void Recording::board_rotate(Position_Rotation rotation, bool rotate_back)
{
	if (this->count < 1) return;

	for (unsigned short i = 0; i < this->count; i++) {
		if (this->moves[i].position_null()) continue;
		this->moves[i].rotate(this->board_size, rotation, rotate_back);
	}
	this->refresh_positions();
}

void Recording::board_print(ostream &ost, unsigned short dots_count, Move* pdots, bool use_ascii) const
{
	for(char v = this->board_size - 1; v >= 0; v--) {
		if (v + 1 < 10) ost << " ";
		ost << " " << (short) (v + 1);
		for(char h = 0; h < this->board_size; h++) {
			bool is_dot = false;
			switch (this->positions[h][v]) {
				case Position_Empty:
					if (dots_count != 0 && pdots != NULL) {
						for (unsigned short d = 0; d < dots_count; d++)
							if (pdots[d].x == h && pdots[d].y == v)
								{is_dot = true; ost << (use_ascii? " *" : "·"); break;}
					} if (is_dot) break;
					if (use_ascii) {ost << " ."; break;}
					
					if (v == 0 && h == 0)
						ost << "└";
					else if (v == 0 && h == this->board_size - 1)
						ost << "┘";
					else if (v == this->board_size - 1 && h == 0)
						ost << "┌";
					else if (v == this->board_size - 1 && h == this->board_size - 1)
						ost << "┐";
					else if (v == 0)
						ost << "┴";
					else if (v == this->board_size - 1)
						ost << "┬";
					else if (h == 0)
						ost << "├";
					else if (h == this->board_size - 1)
						ost << "┤";
					else
						ost << "┼";
					break;
				case Position_Black:
					if (use_ascii) {ost << " X"; break;}
					if (this->moves[count - 1].x == h && this->moves[count - 1].y == v)
					 	ost << "◆";
					else
					 	ost << "●";
					break;
				case Position_White:
					if (use_ascii) {ost << " O"; break;}
					if (this->moves[count - 1].x == h && this->moves[count - 1].y == v)
						ost << "⊙";
					else
						ost << "○";
					break;
				default:
					ost <<"??"; break;
			}
		}
		
		ost << endl;
	}
	
	ost << "    ";
	for (char h = 0; h < this->board_size; h++)
		ost << (char) (h + 0x61) << ' ';
	ost << endl;
}

