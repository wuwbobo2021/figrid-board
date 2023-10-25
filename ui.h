// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_UI_H
#define FIGRID_UI_H

#include "session.h"

namespace Figrid
{

class FigridUI
{
protected:
	Session* session;
	FigridUI(Session* s): session(s) {}

public:	
	virtual int run() = 0;
	virtual void refresh() = 0;
};

}
#endif

