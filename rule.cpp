// Figrid v0.11
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#include "rule.h"

using namespace std;
using namespace Namespace_Figrid;

Rule::Rule() {}

Rule::Rule(Recording* rec): crec(rec) {}

void Rule::set_recording(Recording* rec)
{
	this->crec = rec;
	this->check_recording();
}

Game_Status Rule::game_status() const
{
	return this->cstatus;
}

unsigned short Rule::invalid_moves_count() const
{
	return this->invalid_count;
}

bool Rule::check_recording()
{
	if (this->crec == NULL) return false;
	if (this->crec->moves_count() == 0) {
		this->cstatus = (Game_Status) (Game_Status_First_Mover | Game_Status_Black);
		return true;
	}
	this->cstatus = (Game_Status) 0;
	
	unsigned short cnt; //count of moves checked
	Recording rec = * this->crec; const Move* p = rec.recording_ptr(); //copy current recording
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

unsigned short Rule::undo_invalid_moves()
{
	if (this->crec == NULL) return 0;
	
	if (this->invalid_count == 0)
		this->check_recording();
		
	unsigned short cnt = this->invalid_count;
	this->crec->undo(cnt);
	this->cstatus = (Game_Status) (this->cstatus & (~Game_Status_Foul));
	this->check_recording();
	return cnt;
}

