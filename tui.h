// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_TUI_H
#define FIGRID_TUI_H

#include "ui.h"

namespace Figrid
{

class FigridTUI: public FigridUI
{
	char buf_getline[2048] = {0}; //getline buffer
	bool flag_pipe = false, flag_xo_board = false, flag_exit = false;
	string str_prev_cmd = "";
	
	bool get_line(string& str);
	void terminal_pause();
	void output_help();
	bool check_has_library();
	bool check_library_write_mode();
	void execute(string& strin);
	
public:
	FigridTUI(Session* s);
	void set_pipe_mode();
	void set_xo_board_mode();
	int run() override;
	void refresh() override;
};

}
#endif

