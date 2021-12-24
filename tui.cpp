// Figrid v0.10
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

#include <sstream>

#include "ui.h"
#include "tui.h"
#include "platform_specific.h"

namespace Namespace_Figrid
{

Figrid_TUI::Figrid_TUI(Figrid* f): Figrid_UI(f), figrid(f) {}

Figrid_TUI::~Figrid_TUI()
{
	terminal_color_change_back();
}

void Figrid_TUI::output_help()
{
	cout << "undo, u [count]\tUndo move(s) in current recording\n"
	     << "goback <num>\tGo back to <num>th move\n"
	     << "clear, root, r\tClear current recording\n";
	if (this->figrid->current_mode() != Figrid_Mode_None)
		cout << "rotate\t\tRotate current board to match with existing route in the library\n"
		     << "down, d\t\tGoto the next fork in the tree\n";
	
	cout << "open\t\tOpen PGN or Renlib file\n";
	if (this->figrid->current_mode() != Figrid_Mode_Library_Write)
		cout << "write\t\tSwitch to library writing mode\n";
	else
		cout << "mark [start]\tMark current node\n"
		     << "unmark [start]\tUnmark current node\n"
		     << "comment\t\tSet comment for current node\n"
		     << "uncomment\tUncomment current node\n"
		     << "delete\t\tDelete current node and go back to parent node\n";
	
	if (this->figrid->current_mode() != Figrid_Mode_None)
		cout << "save\t\tSave current tree as Renlib file\n"
		     << "close\t\tClose current tree\n";
	
	cout << "exit, quit\tExit this program\n";
}

void Figrid_TUI::execute(string& strin)
{
	if (strin.length() == 0) return;
	
	string cpstr = strin;
	stringstream sstr(cpstr);
	string command = "", param = ""; int param_num = 0;
	sstr >> command;
	
	if (command == "undo" || command == "u") {
		if (sstr >> param_num)
			this->figrid->undo(param_num);
		else
			this->figrid->undo();
	} else if (command == "goback") {
		if (sstr >> param_num)
			this->figrid->goback(param_num);
	} else if (command == "clear" || command == "root" || command == "r") {
		this->figrid->clear();
	} else if (command == "open") {
		sstr >> param;
		if (! this->figrid->load_file(param)) {
			cout << "Failed to load file.";
			terminal_pause();
		}
	} else if (command == "write") {
		this->figrid->set_mode(Figrid_Mode_Library_Write);	
	} else if (command == "rotate") {
		this->figrid->rotate_into_tree();
	} else if (command == "down" || command == "d") {
		this->figrid->tree_goto_fork();	
	} else if (command == "mark") {
		sstr >> param;
		this->figrid->node_set_mark(true, param == "start");
	} else if (command == "unmark") {
		sstr >> param;
		this->figrid->node_set_mark(false, param == "start");
	} else if (command == "comment") {
		if (this->figrid->current_mode() != Figrid_Mode_Library_Write) return;
		
		terminal_clear();
		string comment;
		this->figrid->tree()->get_current_comment(comment);
		if (comment.length() > 0)
			cout << "Current comment:\n" << comment << '\n' << "Input comment to be appended, ";
		else
			cout << "Input comment, ";
		cout << "then enter \"end\" to continue:\n";
		const unsigned short llmax = 1024; char str[llmax]; bool multiline = false;
		while (cin.getline(str, llmax)) {
			if ((string) str == "end") break;
			if (multiline) comment += '\n';
			comment += (string) str;
			multiline = true;
		}
		this->figrid->node_set_comment(comment);
	} else if (command == "uncomment") {
		string strempty = "";
		this->figrid->node_set_comment(strempty);
	} else if (command == "delete") {
		this->figrid->tree_delete_node();
	} else if (command == "save") {
		if (this->figrid->current_mode() != Figrid_Mode_Library_Write) {
			cout << "Invalid command. "; terminal_pause(); return;
		}
		sstr >> param;
		if (param == "") {
			cout << "Enter library file name, including path: ";
			cin >> param;
		}
		if (! this->figrid->save_renlib(param)) {
			cout << "Failed to save file \"" << param << "\". "; terminal_pause();
		} else
			this->figrid->set_mode(Figrid_Mode_Library_Read);
	} else if (command == "close") {
		this->figrid->set_mode(Figrid_Mode_None);
	} else if (command == "help") {
		terminal_clear();
		this->output_help();
		terminal_pause();
	} else if (command == "exit" || command == "quit") {
		tag_exit = true;
		if (this->figrid->current_mode() == Figrid_Mode_Library_Write) {
			cout << "Discard current data and exit? (y/n) ";
			cin >> param;
			if (param != "y") tag_exit = false;
		}
	} else {
		sstr = stringstream(strin);
		this->figrid->input(sstr);
	}
}

void Figrid_TUI::refresh()
{
	terminal_clear();
	cout << "Figrid v0.10\t" << this->figrid->current_mode_str() << "\n";
	this->figrid->board_print(cout);
	this->figrid->output(cout); cout << '\n';
	if (this->figrid->current_mode() != Figrid_Mode_None)
		this->figrid->output_node_info(cout, false);
	figrid->output_game_status(cout);
	cout << "> ";
}

int Figrid_TUI::run()
{
	terminal_color_change();
	this->refresh();
	
	const unsigned short llmax = 1024; 
	char strin[llmax]; string strcpp;
	
	while (cin.getline(strin, llmax))
	{
		strcpp = (string) strin;
		this->execute(strcpp);
		if (this->tag_exit) break;
		refresh();
	}
	
	return 0;
}

}

