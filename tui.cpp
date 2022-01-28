// Figrid v0.15
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#include <string>
#include <sstream>
#include <locale>
#include <codecvt>

#include "ui.h"
#include "tui.h"

using namespace Namespace_Figrid;

void Figrid_TUI::string_skip_spaces(string& str)
{
	if (str.length() == 0) return;
	
	unsigned int i;
	for (i = 0; i < str.length(); i++)
		if (str[i] != ' ' && str[i] != '\t') break;
	
	str = str.substr(i, str.length() - i);
}

void Figrid_TUI::string_transfer_to_ansi(string& str) //this function is not written by myself
{
	//to wstring
	wstring_convert<codecvt_utf8<wchar_t>> wconv;
	wstring wstr = wconv.from_bytes(str.c_str()); //transfer chars to wchars according to UTF-8 encoding
	
	//to ANSI (local page) string
	vector<char> buf(wstr.size());
	use_facet<ctype<wchar_t>>(locale("")).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
	str = string(buf.data(), buf.size());
}

void Figrid_TUI::terminal_color_change()
{
#ifdef _WIN32
	system("color f0");
#else	
	cout << "\033[30;47m"; //white background and black foreground
#endif
}

void Figrid_TUI::terminal_color_change_back()
{
#ifdef _WIN32
	system("color");
#else
	cout << "\033[0m";
#endif
}

void Figrid_TUI::terminal_clear()
{
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
}

void Figrid_TUI::terminal_pause()
{
#ifdef _WIN32
	system("pause");
#else
	cout << "Press Enter to continue...";
	//system("read"); //useless
	cin.getline(this->buf_getline, sizeof(this->buf_getline));
#endif
}

Figrid_TUI::Figrid_TUI(Figrid* f): Figrid_UI(f), figrid(f)
{
	cout.sync_with_stdio(false); //detach from C stdio
}

Figrid_TUI::~Figrid_TUI()
{
	terminal_color_change_back();
}

void Figrid_TUI::set_pipe_mode()
{
	if (this->tag_pipe) return;
	tag_pipe = true;
	cout.sync_with_stdio(true);
}

void Figrid_TUI::output_help()
{
	cout << "A recording software for the Five-in-a-Row game compatible with Renlib.\n"
	     << "By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.\n"
	     << "(Original Renlib: <http://www.renju.se/renlib>, by Frank Arkbo)\n"
	     << "If you have any suggestion or you have found bug(s), please contact me.\n"
	     << "Note: Choose proper font, and ambigious width characters should be fullwidth.\n\n";
	
	cout << "undo, u [count]\t\tUndo move(s) in current recording\n"
	     << "goback <num>\t\tGo back to <num>th move\n"
	     << "clear, root, r\t\tClear current recording\n";
	
	if (this->figrid->current_mode() != Figrid_Mode_None)
		cout << "down, d\t\t\tGoto the next fork in the tree\n"
		     << "search mark|start\tSearch marks under current node\n"
		     << "search pos <pos>\tSearch nodes of position <pos> under current node\n"
		     << "search <comment>\tSearch in comments of nodes under current node\n"
		     << "rotate\t\t\tRotate current board to match with existing route in the library\n";
	
	cout << "rotate <d>\t\tRotate current board. <d> can be +(-) 90, 180, 270\n"
	     << "reflect, flip <d>\tReflect current board. <d> can be 'h' or 'v'\n";
	
	cout << "open\t\t\tOpen PGN or Renlib file\n";
	if (this->figrid->current_mode() != Figrid_Mode_Library_Write)
		cout << "write\t\t\tSwitch to library writing mode\n";
	else
		cout << "mark [start]\t\tMark current node\n"
		     << "unmark [start]\t\tUnmark current node\n"
		     << "comment\t\t\tSet comment for current node\n"
		     << "uncomment\t\tUncomment current node\n"
		     << "delete\t\t\tDelete current node and go back to parent node\n"
		     << "lock\t\t\tSwitch to library reading mode\n";
	
	if (this->figrid->current_mode() != Figrid_Mode_None)
		cout << "save\t\t\tSave current tree as Renlib file and lock current tree\n"
		     << "close\t\t\tDiscard current tree\n";
	
	cout << "exit, quit\t\tExit this program\n";
}

void Figrid_TUI::execute(string& strin)
{
	if (strin.length() == 0) return;
	
	string cpstr = strin;
	stringstream sstr(cpstr);
	string command = "", param = ""; int param_num = 0;
	sstr >> command;
	
	if (command == "output") { //for pipe mode
		this->figrid->output(cout, false); cout << '\n';
	} else if (command == "undo" || command == "u") {
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
		param = cpstr; param.replace(0, command.length(), ""); string_skip_spaces(param);
		if (! this->tag_pipe) {
			if (this->figrid->current_mode() == Figrid_Mode_Library_Write
			 && this->figrid->tree()->is_renlib_file(param)) {
				cout << "Discard current data and exit? (y/n) ";
				cin >> param; if (param != "y") return;
			}
		}
		if (! this->figrid->load_file(param)) {
			cout << "Failed to load file. "; terminal_pause();
		}
	} else if (command == "write") {
		this->figrid->set_mode(Figrid_Mode_Library_Write);
	} else if (command == "lock") {
		if (this->figrid->current_mode() == Figrid_Mode_Library_Write)
			this->figrid->set_mode(Figrid_Mode_Library_Read);
	} else if (command == "rotate") {
		sstr >> param_num;
		if (param_num == 0)
			this->figrid->rotate_into_tree();
		else {
			Position_Rotation rotation;
			switch (param_num) {
				case 90: case -270: rotation = Rotate_Clockwise; break;
				case 180: case -180: rotation = Rotate_Central_Symmetric; break;
				case -90: case 270: rotation = Rotate_Counterclockwise; break;
			}
			this->figrid->rotate(rotation);
		}
	} else if (command == "reflect" || command == "flip") {
		sstr >> param;  if (param.length() == 0) return;
		char ch = param[0];
		if (ch == 'h')
			this->figrid->rotate(Reflect_Horizontal);
		else if (ch == 'v')
			this->figrid->rotate(Reflect_Vertical);
	} else if (command == "down" || command == "d") {
		this->figrid->tree_goto_fork();	
	} else if (command == "search") {
		if (this->figrid->current_mode() == Figrid_Mode_None) return;
		sstr >> param;
		
		Node_Search sch; vector<Recording> result;
		sch.result = &result;
		
		if (param == "pos") {
			sch.mode = Node_Search_Position;
			Recording tmprec(this->figrid->recording()->board_size); tmprec.input(sstr);
			if (tmprec.moves_count() == 0) return; //invalid move string
			sch.pos = tmprec.last_move(); //it's the only move in `tmprec`
		} else if (param == "mark")
			sch.mode = Node_Search_Mark;
		else if (param == "start")
			sch.mode = Node_Search_Start;
		else if (param.length() > 0){
			sch.mode = Node_Search_Comment;
			param = cpstr; param.replace(0, command.length(), ""); string_skip_spaces(param);
			sch.str = param;
		}
		this->figrid->search(&sch);
		
		if (this->tag_pipe || result.size() > 1) {
			for (unsigned short i = 0; i < result.size(); i++) {
				if (result[i].moves_count() == 0)
					cout << "(Current)\n";
				else {
					result[i].output(cout); cout << '\n';
				}
			}
			if (!this->tag_pipe) terminal_pause();
		}
	} else if (command == "mark") {
		sstr >> param;
		this->figrid->node_set_mark(true, param == "start");
	} else if (command == "unmark") {
		sstr >> param;
		this->figrid->node_set_mark(false, param == "start");
	} else if (command == "comment") {
		if (this->tag_pipe && this->figrid->current_mode() == Figrid_Mode_Library_Read) {
			string comment; this->figrid->tree()->get_current_comment(comment);
			if (comment.length() == 0) return;
			cout << comment << '\n'; return;
		}
		
		if (this->figrid->current_mode() != Figrid_Mode_Library_Write) return;
		
		string comment; this->figrid->tree()->get_current_comment(comment);
		if (! this->tag_pipe) {
			terminal_clear();
			if (comment.length() > 0)
				cout << "Current comment:\n" << comment << '\n' << "Input comment to be appended, ";
			else
				cout << "Input comment, ";
			cout << "then enter \"end\" to continue:\n";
		} else
			if (comment != "") cout << comment << '\n';
		
		string new_comment = ""; unsigned short new_lines_count = 0;
		while (cin.getline(this->buf_getline, sizeof(this->buf_getline))) {
			if ((string) this->buf_getline == "end") break;
			if (new_lines_count >= 1) new_comment += '\n';
			new_comment += (string) this->buf_getline;  new_lines_count++;
		}
		if (new_comment != "") {
			if (comment.find("\n") != string::npos || (comment != "" && new_lines_count > 1))
				comment += '\n';
			else if (comment != "")
				comment += " ";
			comment += new_comment;
			this->figrid->node_set_comment(comment);
		}
	} else if (command == "uncomment") {
		string strempty = "";
		this->figrid->node_set_comment(strempty);
	} else if (command == "delete") {
		if (this->figrid->current_mode() != Figrid_Mode_Library_Write) {
			cout << "Invalid command. "; terminal_pause(); return;
		}
		this->figrid->tree_delete_node();
	} else if (command == "save") {
		if (this->figrid->current_mode() != Figrid_Mode_Library_Write) {
			cout << "Invalid command. "; terminal_pause(); return;
		}
		param = cpstr; param.replace(0, command.length(), ""); string_skip_spaces(param);
		if (param == "") {
			if (! tag_pipe) cout << "Enter library file name, including path: ";
			cin >> param;
		}
		if (! this->figrid->save_renlib(param)) {
			cout << "Failed to save file \"" << param << "\". "; terminal_pause();
		} else
			this->figrid->set_mode(Figrid_Mode_Library_Read);
	} else if (command == "close") {
		this->figrid->set_mode(Figrid_Mode_None);
	} else if (command == "help" || command == "h" || command == "?") { //for user
		terminal_clear();
		this->output_help();
		terminal_pause();
	} else if (command == "exit" || command == "quit") {
		tag_exit = true;
		if (!tag_pipe && this->figrid->current_mode() == Figrid_Mode_Library_Write) {
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
	cout << "Figrid v0.15\t" << this->figrid->current_mode_str() << "\n";
	
	#ifdef _WIN32
		if (! tag_ascii) {
			ostringstream sstr;
			this->figrid->board_print(sstr);
			string str_board = sstr.str();
			string_transfer_to_ansi(str_board);
			
			if (str_board.find("?") != string::npos) //failed to transfer
				tag_ascii = true;
			else
				cout << str_board;
		}
		if (tag_ascii) {
			this->figrid->board_print(cout, true);
		}
	#else
		this->figrid->board_print(cout);
	#endif
	
	this->figrid->output(cout); cout << '\n';
	if (this->figrid->current_mode() != Figrid_Mode_None)
		this->figrid->output_node_info(cout, true);
	figrid->output_game_status(cout);
	cout << "> ";
}

int Figrid_TUI::run()
{
	if (! this->tag_pipe) {
		terminal_color_change();
		this->refresh();
	}
	
	string strcpp;
	while (cin.getline(this->buf_getline, sizeof(this->buf_getline)))
	{
		strcpp = (string) buf_getline;
		this->execute(strcpp);
		if (this->tag_exit) break;
		if (! this->tag_pipe)
			this->refresh();
		else
			this->figrid->output_node_info(cout, false);
	}
	
	return 0;
}

