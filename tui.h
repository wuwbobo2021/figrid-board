// Figrid v0.15
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_TUI_H
#define FIGRID_TUI_H

#include "ui.h"

namespace Namespace_Figrid
{

class Figrid_TUI: Figrid_UI
{
	Figrid* figrid;
	char buf_getline[2048]; //getline buffer
	bool tag_pipe = false, tag_ascii = false, tag_exit = false;
	
	void string_skip_spaces(string& str); //remove spaces at the start of the string
	void string_transfer_to_ansi(string& str);
	
	void terminal_color_change(); //change to white background and black foreground
	void terminal_color_change_back();
	void terminal_clear();
	void terminal_pause();
	
	void output_help();
	void execute(string& strin);
	
public:
	Figrid_TUI(Figrid* f);
	~Figrid_TUI();
	void set_pipe_mode();
	int run();
	void refresh();
};

}
#endif

