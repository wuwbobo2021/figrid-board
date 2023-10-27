// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#include <string>
#include <sstream>
#include <cctype>

#include "recording.h"
#include "ui.h"
#include "tui.h"

using namespace Figrid;

static inline void skip_spaces(string& str) //remove spaces at the beginning of the string
{
	if (str.length() == 0) return;
	
	unsigned int i;
	for (i = 0; i < str.length(); i++)
		if (! isblank(str[i])) break;
	
	if (i < str.length())
		str = str.substr(i, str.length() - i);
	else
		str = "";
}

static inline bool parse_word(string& str, const string& word) //remove word if found
{
	skip_spaces(str);
	
	unsigned int word_len = word.length();
	if (!word_len || str.length() < word_len)
		return false;
	if (str.substr(0, word_len) != word)
		return false;
	if (str.length() == word_len)
		{str = ""; return true;}
	if (! isblank(str[word_len]))
		return false;
	
	str.replace(0, word_len, ""); skip_spaces(str);
	return true;
}

static inline bool parse_num(string& str, int* p_num)
{
	istringstream iss(str); int num;
	if (iss >> num) {
		*p_num = num;
		stringbuf buf; //store the remaining
		iss >> &buf; str = buf.str();
		return true;
	} else
		return false;
}

bool ask_yes_no(const string& str_question)
{
	cout << str_question << " (y/n) " << flush;
	string str_ans; cin >> str_ans;
	return (str_ans.length() == 1 && tolower(str_ans[0]) == 'y');
}

static inline void terminal_color_change()
{
#ifdef _WIN32
	int ret = system("color f0");
#else
	cout << "\033[30;107m"; //white background and black foreground
	int ret = system("clear");
#endif
}

static inline void terminal_color_change_back()
{
#ifdef _WIN32
	int ret = system("color");
#else
	cout << "\033[0m" << flush;
#endif
}

static inline void terminal_clear()
{
#ifdef _WIN32
	int ret = system("cls");
#else
	cout << "\033[2J\033[3J\033[1;1H"; //"\033[H\033[J"; //<< flush;
#endif
}

void FigridTUI::terminal_pause()
{
#ifdef _WIN32
	cout << flush; int ret = system("pause");
#else
	cout << "Press Enter to continue..." << flush;
	string str_tmp;
	this->get_line(str_tmp); //system("read") doesn't work
#endif
}

FigridTUI::FigridTUI(Session* s): FigridUI(s)
{
	cout.sync_with_stdio(false); //detach from C stdio
}

void FigridTUI::set_pipe_mode()
{
	this->flag_pipe = true;
}

void FigridTUI::set_xo_board_mode()
{
	this->flag_xo_board = true;
}

bool FigridTUI::get_line(string& str)
{
	if (cin.getline(this->buf_getline, sizeof(this->buf_getline))) {
		str = string(buf_getline); return true;
	} else {
		str = ""; return false;
	}
}

void FigridTUI::output_help()
{
	cout << "A recording software for the Five-in-a-Row game compatible with Renlib.\n"
	     << "By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.\n"
	     << "(Original Renlib: <http://www.renju.se/renlib>, by Frank Arkbo)\n"
	     << "If you have any suggestion or you have found bug(s), please contact me.\n";
	if (! this->flag_xo_board)
		cout << "Note: Choose proper font, and ambiguous width characters should be fullwidth.\n"
		     << "      If there is no such option in the terminal, execute this program with -x.\n";
	cout << '\n';
	
	cout << "undo, u [count]\t\tUndo move(s) in current recording\n"
	     << "next, n\t\t\tGoto the next move in the tree/recording\n"
	     << "down, d\t\t\tGoto the next fork in the tree/recording\n"
	     << "goto <num>|<pos>\tGoto <num>th or <pos> move in the recording\n"
	     << "clear, root, r\t\tClear current recording\n";
	
	if (this->session->has_library())
		cout << "search mark|start\tSearch marks under current node\n"
		     << "search pos <pos>\tSearch nodes of position <pos> under current node\n"
		     << "search <comment>\tSearch in comments of nodes under current node\n"
		     << "rotate\t\t\tRotate current board to match with existing route\n";
	
	if (this->session->current_mode() == Session_Mode_Library_Write)
		cout << "rotate merge\t\tMerge rotations in the library to the entered recording\n";

	cout << "rotate <d>\t\tRotate current board. <d>: [+|-] 90|180|270|1|2|3\n"
	     << "reflect, flip <d>\tReflect current board. <d>: h|v|ld|rd\n";
	
	cout << "open [list] <path>\tOpen PGN, Renlib or list text file\n";
	if (this->session->current_mode() != Session_Mode_Library_Write)
		cout << "write\t\t\tSwitch to library writing mode\n";
	else
		cout << "mark [start]\t\tMark current node\n"
		     << "unmark [start]\t\tUnmark current node\n"
		     << "comment\t\t\tSet comment for current node\n"
		     << "uncomment\t\tDelete comment of current node\n"
		     << "move l|r <pos>\t\tAdjust the sequence of child nodes by moving one of them\n"
		     << "delete\t\t\tDelete current node and go back to parent node\n"
		     << "standardize\t\tHelp standardize the library by auto-merging of rotations\n"
		     << "lock\t\t\tSwitch to library reading mode\n";
	
	if (this->session->has_library())
		cout << "save [list]\t\tSave current tree as Renlib or list text file\n"
		     << "close\t\t\tDiscard current tree and switch to recording mode\n";
	else
		cout << "save\t\t\tSave current recording ending with current move\n";

	cout << "exit, quit\t\tExit this program\n";
	
	cout << flush;
}

bool FigridTUI::check_has_library()
{
	bool suc = this->session->has_library();
	if (!suc && !this->flag_pipe) {
		cout << "This command is invalid under recording mode. "; terminal_pause();
	}
	return suc;
}

bool FigridTUI::check_library_write_mode()
{
	bool suc = (this->session->current_mode() == Session_Mode_Library_Write);
	if (!suc && !this->flag_pipe) {
		cout << "This command is valid under write mode. "; terminal_pause();
	}
	return suc;
}

void FigridTUI::execute(string& strin)
{
	if (strin.length() > 0)
		this->str_prev_cmd = "";
	else {
		if (this->flag_pipe) return;
		if (this->str_prev_cmd.length() == 0) return;
		strin = this->str_prev_cmd;
	}

	string cmd = strin; int param_num = 0;
	
	if (parse_word(cmd, "output")) { //for pipe mode
		this->session->output(cout, false); cout << endl;
	}
	else if (parse_word(cmd, "undo") || parse_word(cmd, "u")) {
		if (parse_num(cmd, &param_num)) {
			if (param_num > 0) this->session->undo(param_num);
		} else
			this->session->undo();
		this->str_prev_cmd = strin;
	}
	else if (parse_word(cmd, "next") || parse_word(cmd, "n")) {
		this->session->goto_next();
		this->str_prev_cmd = strin;
	}
	else if (parse_word(cmd, "goto")) {
		if (parse_num(cmd, &param_num)) {
			if (param_num >= 0) this->session->goto_num(param_num);
		} else
			this->session->goto_move(read_single_move(cmd));
	}
	else if (parse_word(cmd, "down") || parse_word(cmd, "d")) {
		this->session->go_straight_down();	
	}
	else if (parse_word(cmd, "clear") || parse_word(cmd, "root") || parse_word(cmd, "r")) {
		this->session->clear();
	}
	else if (parse_word(cmd, "open")) {
		bool is_node_list = parse_word(cmd, "list");
		if (cmd.length() == 0) return;
		if (this->flag_pipe == false
		&&  this->session->current_mode() == Session_Mode_Library_Write
		&&  Tree::is_renlib_file(cmd)) {
			if (! ask_yes_no("Discard current data?")) return;
		}
		if (! this->session->load_file(cmd, is_node_list)) {
			cout << "Failed to load file. "; terminal_pause();
		}
	}
	else if (parse_word(cmd, "write")) {
		this->session->set_mode(Session_Mode_Library_Write);
	}
	else if (parse_word(cmd, "lock")) {
		if (this->session->current_mode() == Session_Mode_Library_Write)
			this->session->set_mode(Session_Mode_Library_Read);
	}
	else if (parse_word(cmd, "rotate")) {
		if (parse_num(cmd, &param_num)) {
			Position_Rotation rotation;
			switch (param_num) {
				case  90: case -270: case  1: case -3:
					rotation = Rotate_Clockwise; break;
				case 180: case -180: case  2: case -2:
					rotation = Rotate_Central_Symmetric; break;
				case -90: case  270: case -1: case  3:
					rotation = Rotate_Counterclockwise; break;
				default: return;
			}
			this->session->rotate(rotation);
			this->str_prev_cmd = strin;
		}
		else if (parse_word(cmd, "merge")) {
			if (! this->check_library_write_mode()) return;
			this->session->tree_merge_rotations();
		}
		else {
			if (! this->check_has_library()) return;
			this->session->rotate_into_tree();
		}
	}
	else if (parse_word(cmd, "reflect") || parse_word(cmd, "flip")) {
		if (parse_word(cmd, "h"))
			this->session->rotate(Reflect_Horizontal);
		else if (parse_word(cmd, "v"))
			this->session->rotate(Reflect_Vertical);
		else if (parse_word(cmd, "ld"))
			this->session->rotate(Reflect_Left_Diagonal);
		else if (parse_word(cmd, "rd"))
			this->session->rotate(Reflect_Right_Diagonal);
		else return;
		this->str_prev_cmd = strin;
	}
	else if (parse_word(cmd, "search")) {
		if (! this->check_has_library()) return;

		NodeSearch sch;
		sch.mode = Node_Search_Leaf;
		sch.direct_output = true; sch.p_ost = &cout;
		
		while (true) {
			if (parse_word(cmd, "pos")) {
				sch.mode = (Node_Search_Mode)(sch.mode | Node_Search_Position);
				sch.pos = read_single_move(cmd);
				if (! parse_word(cmd, (string)(sch.pos))) break;
			}
			else if (parse_word(cmd, "mark"))
				sch.mode = (Node_Search_Mode)(sch.mode | Node_Search_Mark);
			else if (parse_word(cmd, "start"))
				sch.mode = (Node_Search_Mode)(sch.mode | Node_Search_Start);
			else if (cmd.length() > 0) {
				sch.mode = (Node_Search_Mode)(sch.mode | Node_Search_Comment);
				sch.str = cmd; break; //take the remaining of the line
			}
			else break;
		}
		this->session->search(&sch); cout << flush;

		if (!this->flag_pipe && sch.match_count > 1)
			terminal_pause();
	}
	else if (parse_word(cmd, "mark")) {
		if (! this->check_library_write_mode()) return;
		this->session->node_set_mark(true, parse_word(cmd, "start"));
	}
	else if (parse_word(cmd, "unmark")) {
		if (! this->check_library_write_mode()) return;
		this->session->node_set_mark(false, parse_word(cmd, "start"));
	}
	else if (parse_word(cmd, "comment")) {
		if (this->flag_pipe && this->session->current_mode() == Session_Mode_Library_Read) {
			string comment; this->session->get_current_comment(comment);
			if (comment.length() == 0) return;
			cout << comment << endl; return;
		}

		if (! this->check_library_write_mode()) return;

		string comment; this->session->get_current_comment(comment);
		if (! this->flag_pipe) {
			terminal_clear();
			if (comment.length() > 0)
				cout << "Current comment:" << endl << comment << endl << "Input comment to be appended, ";
			else
				cout << "Input comment, ";
			cout << "then enter a line \"end\" to continue:" << endl;
		} else
			if (comment != "") cout << comment << endl;
		
		string new_comment = ""; unsigned short new_lines_count = 0;
		string str_line;
		while (this->get_line(str_line)) {
			if (str_line == "end") break;
			if (new_lines_count >= 1) new_comment += '\n';
			new_comment += str_line;  new_lines_count++;
		}
		if (new_comment != "") {
			if (comment.find("\n") != string::npos || (comment != "" && new_lines_count > 1))
				comment += '\n';
			else if (comment != "")
				comment += " ";
			comment += new_comment;
			this->session->node_set_comment(comment);
		}
	}
	else if (parse_word(cmd, "uncomment")) {
		if (! this->check_library_write_mode()) return;
		string str_empty = "";
		this->session->node_set_comment(str_empty);
	}
	else if (parse_word(cmd, "move")) {
		if (! this->check_library_write_mode()) return;
		bool move_left;
		if (parse_word(cmd, "l"))
			move_left = true;
		else if (parse_word(cmd, "r"))
			move_left = false;
		else return;

		Move pos = read_single_move(cmd);
		if (move_left)
			this->session->tree_node_move_left(pos);
		else
			this->session->tree_node_move_right(pos);
		this->str_prev_cmd = strin;
	}
	else if (parse_word(cmd, "delete")) {
		if (! this->check_library_write_mode()) return;
		this->session->tree_delete_node();
	}
	else if (parse_word(cmd, "standardize")) {
		if (! this->check_library_write_mode()) return;
		this->session->tree_help_standardize();
	}
	else if (parse_word(cmd, "save")) {
		bool is_node_list = parse_word(cmd, "list");
		if (! this->session->has_library()) is_node_list = true;
		if (cmd == "") {
			if (! this->flag_pipe) cout << "Enter library file name, including path: " << flush;
			if (! this->get_line(cmd)) return;
		}

		bool suc;
		if (! is_node_list)
			suc = this->session->save_renlib(cmd);
		else
			suc = this->session->save_node_list(cmd);

		if (! suc) {
			cout << "Failed to save file \"" << cmd << "\". "; terminal_pause();
		} else if (this->session->has_library())
			this->session->set_mode(Session_Mode_Library_Read);
	}
	else if (parse_word(cmd, "close")) {
		if (!this->flag_pipe && this->session->current_mode() == Session_Mode_Library_Write) {
			if (! ask_yes_no("Discard current data?")) return;
		}
		this->session->set_mode(Session_Mode_None);
	}
	else if (parse_word(cmd, "exit") || parse_word(cmd, "quit")) {
		if (!this->flag_pipe && this->session->current_mode() == Session_Mode_Library_Write)
			flag_exit = ask_yes_no("Discard current data and exit?");
		else
			flag_exit = true;
	}
	else if (parse_word(cmd, "help") || parse_word(cmd, "h") || parse_word(cmd, "?")) {
		terminal_clear();
		this->output_help();
		terminal_pause();
	}
	else {
		istringstream iss(cmd);
		this->session->input(iss);
	}
}

void FigridTUI::refresh()
{
	terminal_clear();
	cout << "Figrid v0.20\t" << this->session->current_mode_str() << '\n';
	this->session->board_print(cout, this->flag_xo_board);
	this->session->output(cout); cout << '\n';
	if (this->session->has_library())
		this->session->output_node_info(cout, true);
	session->output_game_status(cout);
	cout << "> " << flush;
}

int FigridTUI::run()
{
	if (! this->flag_pipe) {
		#ifdef _WIN32
			this->flag_xo_board = true;
		#endif
		terminal_color_change();
		this->refresh();
	}
	
	string str_cmd;
	while (this->get_line(str_cmd)) {
		this->execute(str_cmd);
		if (this->flag_exit) break;
		if (! this->flag_pipe)
			this->refresh();
		else {
			this->session->output_node_info(cout, false);
			cout << flush;
		}
	}
	
	if (! this->flag_pipe)
		terminal_color_change_back();
	return 0;
}
