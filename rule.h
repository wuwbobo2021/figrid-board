// Figrid v0.15
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_RULE_H
#define FIGRID_RULE_H

#include "recording.h"

namespace Namespace_Figrid
{

enum Game_Status: unsigned char
{
	Game_Status_Black = 0,         // 00000000
	Game_Status_White = 1,         // 00000001
	Game_Status_Choice = 2,        // 00000010, choose whether or not to swap
	Game_Status_First_Mover = 4,   // 00000100
	Game_Status_Second_Mover = 8,  // 00001000
	Game_Status_Ended = 16,        // 00010000
	
	Game_Status_Foul = 128         // 10000000
};

class Rule
{
protected:
	Recording* crec;
	Game_Status cstatus;
	unsigned short invalid_count;

	Rule();
	Rule(Recording* rec);

public:
	virtual bool domove(Move mv) = 0;
	
	void set_recording(Recording* rec);
	Game_Status game_status() const;
	unsigned short invalid_moves_count() const;
	bool check_recording();
	unsigned short undo_invalid_moves();
};

}
#endif

