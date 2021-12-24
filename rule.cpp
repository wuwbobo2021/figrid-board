// Figrid v0.10
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

#include "rule.h"

using namespace std;

namespace Namespace_Figrid
{

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

}

