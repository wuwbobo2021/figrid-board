// Figrid v0.10
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

#include "rule_gomoku_original.h"

namespace Namespace_Figrid
{

Rule_Gomoku_Original::Rule_Gomoku_Original(): Rule() {}
Rule_Gomoku_Original::Rule_Gomoku_Original(Recording* rec): Rule(rec)
{
	this->check_recording();
}

bool Rule_Gomoku_Original::check_row(unsigned char row_length, Position_State* prow)
{
	unsigned char cb = 0, cw = 0;
	for (unsigned char i = 0; i < row_length; i++) {
		switch (prow[i]) {
			case Position_Empty: cb = 0; cw = 0; break;
			case Position_Black: cb++;   cw = 0; break;
			case Position_White: cb = 0; cw++;   break;
		}
		if (cb == 5) {
			if (i == row_length - 1 || prow[i + 1] != Position_Black) {
				this->cstatus = (Game_Status) (Game_Status_Ended | Game_Status_First_Mover);
				return true;
			}
		}
		if (cw == 5) {
			if (i == row_length - 1 || prow[i + 1] != Position_White) {
				this->cstatus = (Game_Status) (Game_Status_Ended | Game_Status_Second_Mover);
				return true;
			}
		}
	}
	return false;
}

bool Rule_Gomoku_Original::domove(Move mv)
{
	if (this->crec == NULL) return false;
	if (this->cstatus & Game_Status_Ended) return false;
	if (this->cstatus & Game_Status_Foul) return false;
	
	if (! this->crec->domove(mv)) return false;
	
	Position_State prow[this->crec->board_size];
	unsigned char row_length;
	for (unsigned char i = 0; i < 4; i++) {
		row_length = this->crec->board_get_row((Board_Row_Direction) i, mv, prow);
		if (row_length >= 5)
			if (this->check_row(row_length, prow)) return true;
	}
	
	switch (this->crec->color_next()) {
		case Position_Black:
			this->cstatus = (Game_Status) (Game_Status_First_Mover  | Game_Status_Black); break;
		case Position_White:
			this->cstatus = (Game_Status) (Game_Status_Second_Mover | Game_Status_White); break;
		case Position_Empty:
			this->cstatus = (Game_Status) (Game_Status_Ended); break; //Tie
	}
	return true;
}

bool Rule_Gomoku_Original::check_recording()
{
	if (this->crec == NULL) return false;
	if (this->crec->moves_count() == 0) {
		this->cstatus = (Game_Status) (Game_Status_First_Mover | Game_Status_Black);
		return true;
	}
	this->cstatus = (Game_Status) 0;
	
	unsigned short cnt; //count of moves checked
	Recording rec = * this->crec; const Move* p = rec.recording_ptr();
	this->crec->clear();
	for (cnt = 0; cnt < rec.moves_count(); cnt++) {
		if (this->cstatus & Game_Status_Ended) {
			this->invalid_count = rec.moves_count() - cnt;
			this->cstatus = Game_Status_Foul;
			* this->crec = rec; //recover the bad recording
			return false;
		}
		
		this->domove(p[cnt]);
	}
	
	if (rec.moves_count() == rec.moves_count_max)
		this->cstatus = Game_Status_Ended;
	
	return true;
}

}
