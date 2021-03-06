// Figrid v0.15
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_RULE_ORIGINAL_H
#define FIGRID_RULE_ORIGINAL_H

#include "rule.h"

namespace Namespace_Figrid
{

class Rule_Gomoku_Original: public Rule
{
	bool check_row(unsigned char row_length, Position_State* prow); //return true if a connection of five rocks is found
	
public:
	Rule_Gomoku_Original();
	Rule_Gomoku_Original(Recording* rec);

	bool domove(Move mv) override;
};

}
#endif

