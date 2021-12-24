// Figrid v0.10
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

#ifndef FIGRID_TUI_H
#define FIGRID_TUI_H

#include "ui.h"

namespace Namespace_Figrid
{

class Figrid_TUI: Figrid_UI
{
	Figrid* figrid;
	bool tag_exit = false;
	
	void output_help();
	void execute(string& strin);
	
public:
	Figrid_TUI(Figrid* f);
	~Figrid_TUI();
	int run();
	void refresh();
};

}
#endif

