// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#include <cstring>
#include <cctype>
#include <stdexcept>
#include <sstream>
#include "recording.h"

using namespace std;
using namespace Figrid;

const unsigned short Null_Pos_Count_Max = 32;

Move::Move(): x(Board_Pos_Null), y(Board_Pos_Null), swap(false) {}
Move::Move(unsigned char nx, unsigned char ny): x(nx), y(ny), swap(false) {}
Move::Move(bool sw): x(Board_Pos_Null), y(Board_Pos_Null), swap(sw) {}

bool Move::operator==(const Move& p) const
{
	if (this->pos_is_null() && p.pos_is_null())
		return this->swap == p.swap;
	else
		return this->x == p.x && this->y == p.y;
}

bool Move::operator!=(const Move& p) const
{
	if (this->pos_is_null() && p.pos_is_null())
		return this->swap != p.swap;
	else
		return this->x != p.x || this->y != p.y;
}

Move::operator string() const
{
	if (this->pos_is_null()) {
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

bool Move::pos_is_null() const
{
	return this->x == Board_Pos_Null && this->y == Board_Pos_Null;
}

void Move::rotate(unsigned char board_size, Position_Rotation rotation)
{
	if (this->pos_is_null()) return;
	
	// Note: the topmost x and rightmost y are both board_size - 1.
	if ((rotation >> 2) & 1) //Reflect_Horizontal bit is true
		this->x = (board_size - 1) - this->x;
	
	unsigned char ro = rotation & 3; //get 2 lower bits
	if (ro == 0) return;
	
	unsigned char xo = this->x, yo = this->y;
	switch (ro) {
		case Rotate_Clockwise:
			this->x = yo;                     this->y = (board_size - 1) - xo;  break;
		case Rotate_Counterclockwise:
			this->x = (board_size - 1) - yo;  this->y = xo;  break;
		case Rotate_Central_Symmetric:
			this->x = (board_size - 1) - xo;  this->y = (board_size - 1) - yo;  break;
	}
}

Position_Rotation Move::standardize(unsigned char board_size)
{
	if (this->pos_is_null()) return Rotate_None;

	unsigned char sz_half = board_size / 2;

	Position_Rotation ro = Rotate_None;
	if (this->x < sz_half) {
		this->x = (board_size - 1) - this->x;
		ro = conbine_rotation(ro, Reflect_Horizontal);
	}
	if (this->y < sz_half) {
		this->y = (board_size - 1) - this->y;
		ro = conbine_rotation(ro, Reflect_Vertical);
	}
	if (this->x < this->y) {
		unsigned char tmp = this->x;
		this->x = this->y; this->y = tmp;
		ro = conbine_rotation(ro, Reflect_Left_Diagonal);
	}
	
	return ro;
}

Position_Rotation Figrid::conbine_rotation(Position_Rotation r1, Position_Rotation r2)
{
	// r1 + r2 = (fl1 + ro1) + (fl2 + ro2) = fl1 + (ro1 + fl2) + ro2
	// fl2 = 0: ro1 + fl2 = fl2 + ro1
	// fl2 = 1: ro1 + fl2 = fl2 + (0b100 - ro1)
	unsigned char fl = (r1 >> 2) + (r2 >> 2), ro; //fl = fl1 + fl2
	if ((r2 >> 2) & 1) //fl2 = 1
		ro = 4 - (r1 & 3) + (r2 & 3);         //ro = (4 - ro1) + ro2
	else
		ro = (r1 & 3) + (r2 & 3);             //ro = ro1 + ro2
	fl &= 1; ro &= 3;
	return (Position_Rotation)((fl << 2) | ro);
}

Position_Rotation Figrid::reverse_rotation(Position_Rotation r)
{
	unsigned char fl = (r >> 2) & 1, ro;
	if (! fl)
		ro = (4 - (r & 3)) & 3;
	else
		ro = r & 3; //ro = 4 - (4 - (r & 3));
	return (Position_Rotation)((fl << 2) | ro);
}

Move Figrid::read_single_move(const string& str)
{
	Recording tmprec(Board_Size_Max);
	istringstream sstr(str); tmprec.input(sstr);
	return tmprec.last_move();
}

void Recording::refresh_positions() //low level operation clearing grid `positions`
{
	for (unsigned char i = 0; i < this->board_size; i++)
		for (unsigned char j = 0; j < this->board_size; j++)
			this->positions[i][j] = Position_Empty;
	
	for (unsigned short i = 0; i < this->cnt; i++) {
		if (this->moves[i].pos_is_null()) continue;
		this->positions[this->moves[i].x][this->moves[i].y]
			= ((i % 2 == 0)? Position_Black : Position_White);
	}
}

void Recording::copy_from(const Recording& from)
{
	if (from.board_size > this->board_size) throw InvalidBoardSizeException();

	const Move* pf = from.recording_ptr();
	for (unsigned short i = 0; i < from.count_all(); i++) {
		this->moves[i] = pf[i];
		if (this->moves[i].pos_is_null() && i < from.count())
			this->cnt_null_pos++;
	}

	this->cnt = from.count(); this->cnt_all = from.count_all();
	this->refresh_positions();
}

Recording::Recording(unsigned char sz):
	board_size(sz), pos_count_max(sz * sz),
	moves_count_max(sz*sz + Null_Pos_Count_Max)
{
	if (sz < 5 || sz > Board_Size_Max) throw InvalidBoardSizeException();
	
	this->moves = new Move[moves_count_max];
	
	this->positions = new Position_State*[board_size];
	for (unsigned char i = 0; i < this->board_size; i++)
		this->positions[i] = new Position_State[board_size];
	
	this->refresh_positions();
}

Recording::Recording(const Recording& from):
	board_size(from.board_size), pos_count_max(from.pos_count_max),
	moves_count_max(from.moves_count_max)
{
	this->moves = new Move[moves_count_max];

	this->positions = new Position_State*[board_size];
	for (unsigned char i = 0; i < this->board_size; i++)
		this->positions[i] = new Position_State[board_size];

	this->copy_from(from);
}

Recording& Recording::operator=(const Recording& rec)
{
	this->copy_from(rec);
	return *this; //actually converted `this` from pointer to reference
}

Recording::~Recording()
{
	delete[] this->moves;
	
	for (unsigned char i = 0; i < this->board_size; i++)
		delete[] this->positions[i];
	delete[] this->positions;
}

unsigned short Recording::count() const
{
	return this->cnt;
}

unsigned short Recording::count_all() const
{
	return this->cnt_all;
}

Move Recording::operator[](unsigned short i) const
{
	if (i >= this->moves_count_max)
		i = this->moves_count_max - 1;
	return this->moves[i];
}

Move Recording::last_move() const
{
	if (this->cnt == 0) return Move(); //null move
	return this->moves[this->cnt - 1];
}

Position_State Recording::color_next() const
{
	return (this->cnt % 2 == 0)? Position_Black : Position_White;
}

bool Recording::operator==(const Recording& rec) const
{
	if (rec.board_size != this->board_size) return false;
	if (rec.count() != this->cnt) return false;
	
	const Move* pr = rec.recording_ptr();
	for (unsigned short i = 0; i < this->cnt; i++)
		if (pr[i] != this->moves[i]) return false;
		
	return true;
}

bool Recording::operator!=(const Recording& rec) const
{
	return !(*this == rec);
}

bool Recording::domove(Move mv)
{
	if (mv.pos_is_null()) {
		if (this->cnt_null_pos >= Null_Pos_Count_Max) return false;
		this->moves[this->cnt] = mv;
		this->cnt++; this->cnt_null_pos++;
		this->cnt_all = this->cnt;
		return true;
	}
	
	if (! mv.position_valid(this->board_size)) return false;
	if (this->positions[mv.x][mv.y] != Position_Empty) return false;
	
	this->positions[mv.x][mv.y] = this->color_next();
	this->cnt++;
	if (this->moves[this->cnt - 1] != mv) {
		this->moves[this->cnt - 1] = mv;
		this->cnt_all = this->cnt;
	} else
		if (this->cnt_all < this->cnt) this->cnt_all = this->cnt;

	return true;
}

bool Recording::append(const Recording* record)
{
	bool suc = true;
	const Move* prec = record->recording_ptr();
	for (unsigned short i = 0; i < record->count(); i++)
		if (! this->domove(prec[i])) suc = false;
	
	return suc;
}

bool Recording::undo(unsigned short steps)
{
	if (steps > this->cnt) return false;
	
	for (short i = this->cnt - 1; i >= this->cnt - steps; i--) {
		if (! this->moves[i].pos_is_null())
			this->positions[this->moves[i].x][this->moves[i].y] = Position_Empty;
		else
			this->cnt_null_pos--;
	}
	
	this->cnt -= steps;
	return true;
}

bool Recording::redo(unsigned short steps)
{
	unsigned short new_cnt = this->cnt + steps;
	if (new_cnt > this->cnt_all) return false;

	for (unsigned short i = this->cnt; i < new_cnt; i++) {
		if (! this->moves[i].pos_is_null())
			this->positions[this->moves[i].x][this->moves[i].y] = this->color_next();
		else
			this->cnt_null_pos++;
		this->cnt++;
	}
	
	return true;
}

bool Recording::goto_num(unsigned short num)
{
	if (num <= this->cnt)
		return this->undo(this->cnt - num);
	else
		return this->redo(num - this->cnt);
}

bool Recording::goto_move(Move mv, bool back)
{
	int n;
	if (! back)
		n = this->cnt_all - 1;
	else
		n = this->cnt - 1;
	for (int i = n; i >= 0; i--) {
		if (this->moves[i] == mv) {
			this->goto_num(i + 1); return true;
		}
	}
	return false;
}

void Recording::clear()
{
	this->cnt = this->cnt_null_pos = 0;
	this->refresh_positions();
}

bool Recording::input(istream& ist, bool multiline)
{
	const unsigned int llmax = Board_Size_Max * 13 + 10; 
	string str = ""; char buf[llmax]; char cch;
	while (ist && (! ist.eof())) {
		ist.getline(buf, llmax);
		if (strlen(buf) == 0) continue; //empty line
		if (buf[0] == '#' || buf[0] == ';' || buf[0] == '/') continue; //comment
		if (buf[0] == '[') continue; //TODO: improve the class `Recording` to load more info from PGN texts
		break;
	}
	while (true) {
		str += (string) buf; str += " ";
		
		if (!multiline || !ist || ist.eof()) break;
		ist >> cch;
		if (cch == '[') {ist.putback(cch); break;} //beginning of the next PGN recording
		ist.putback(cch);
		
		ist.getline(buf, llmax);
		if (strlen(buf) == 0) break; //empty line
	}
	
	bool suc = true;
	unsigned char c; unsigned char x = 0, y = 0;
	for (unsigned short i = 0; i <= str.length() - 1; i++) {
		c = str[i];
		
		if (isdigit(c)) {
			if (x > 0) { //if x is 0, c should be sequence(round) number
				y *= 10;
				y += (c - 0x30); //see ASCII table
			} else
				if (i < str.length() - 1 && str[i + 1] == '-') return suc;
		} else {
			if (x != 0 && y != 0) {
				if (! this->domove(Move(x - 1, y - 1))) suc = false;
				x = 0; y = 0;
			}
			if (isalpha(c)) { //is letter
				// it's the second mover's choice in 4th item, and in case of
				// the second mover then choose to do two moves, the next move
				// will be white, so it's still a swap choice for the first mover
				// to choose black in 6th item.
				if (this->cnt == 4 - 1 || this->cnt == 6 - 1) {
					if (tolower(c) == 'b' && i + 4 <= str.length() - 1 && str[i + 1] == 'l') {
						this->domove(Move(true)); i += 4; continue;
					} else if (tolower(c) == 'w' && i + 4 <= str.length() - 1 && str[i + 1] == 'h') {
						this->domove(Move(false)); i += 4; continue;
					}
				}
				if (islower(c)) x = c - (islower(c)? 0x61 : 0x41) + 1;
			} else if (c == '-') //pass
				if (! this->domove(Move(false))) suc = false;
		}
	}
	
	if (x != 0 && y != 0)
		if (! this->domove(Move(x - 1, y - 1))) suc = false;
	
	return suc;	
}

void Recording::output(ostream& ost, bool show_round_num) const
{
	if (this->cnt < 1) {ost << "(Empty Recording)"; return;}
	
	bool black = true;
	for (unsigned short i = 0; i < this->cnt; i++)
	{
		if (black) {
			if (i > 0) ost << " ";
			if (show_round_num) ost << to_string((i + 1)/2 + 1) + ". ";
		} else
			ost << " ";

		if (this->moves[i].pos_is_null() && (i == 4 - 1 || i == 6 - 1))
			ost << (this->moves[i].swap? "black" : "white");
		else
			ost << (string) this->moves[i];
		
		black = !black;
	}
}

bool Recording::board_is_filled() const
{
	return this->cnt - this->cnt_null_pos >= this->pos_count_max;
}

unsigned char Recording::board_diagonals_count() const
{
	return this->cnt * 2 - 1;
}

unsigned char Recording::board_get_row(Board_Row_Direction direction, unsigned char index, Position_State* po) const
{
	if (direction == Board_Row_Horizontal || direction == Board_Row_Vertical) {
		if (index > this->board_size - 1) return 0;
	} else
		if (index > this->board_diagonals_count() - 1) return 0;
	
	Move pc; unsigned char cnt_out = 0; //starting point of the diagonal line, current output count
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
				cnt_out++; pc.x++; pc.y++; po++;
			}
			return cnt_out;
		
		case Board_Row_Right_Diagonal:
			if (index <= this->board_size - 1) //the line starts from the bottom of the board
				pc = Move(0, index);
			else
				pc = Move(index - this->board_size + 1, this->board_size);
			
			while (pc.x < this->board_size) {
				*po = this->positions[pc.x][pc.y];
				cnt_out++; if (pc.y == 0) break;
				pc.x++; pc.y--; po++;
			}
			return cnt_out;
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
			index = pos.x + pos.y; break;
	}
	
	return this->board_get_row(direction, index, po);
}

void Recording::board_rotate(Position_Rotation rotation)
{
	if (this->cnt_all < 1 || rotation == Rotate_None) return;

	for (unsigned short i = 0; i < this->cnt_all; i++) {
		if (this->moves[i].pos_is_null()) continue;
		this->moves[i].rotate(this->board_size, rotation);
	}
	this->refresh_positions();
}

void Recording::board_print(ostream &ost, bool use_ascii, Move* pdots, unsigned short dots_count) const
{
	for(char v = this->board_size - 1; v >= 0; v--) {
		if (v + 1 < 10) ost << " ";
		ost << " " << (short) (v + 1);
		for(char h = 0; h < this->board_size; h++) {
			Move cur_pos(h, v); bool is_dot = false;
			switch (this->positions[h][v]) {
				case Position_Empty:
					if (dots_count != 0 && pdots != NULL) {
						for (unsigned short d = 0; d < dots_count; d++)
							if (cur_pos == pdots[d])
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
					if (cur_pos == this->last_move())
					 	ost << "◆";
					else
					 	ost << "●";
					break;
				case Position_White:
					if (use_ascii) {ost << " O"; break;}
					if (cur_pos == this->last_move())
						ost << "⊙";
					else
						ost << "○";
					break;
				default:
					ost <<"??"; break;
			}
		}
		
		ost << '\n';
	}
	
	ost << "    ";
	for (char h = 0; h < this->board_size; h++)
		ost << (char) (h + 0x61) << ' ';
	ost << '\n';
}

const Move* Recording::recording_ptr() const
{
	return this->moves;
}

const Position_State** Recording::board_ptr() const
{
	return (const Position_State**) this->positions;
}
