// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_SESSION_H
#define FIGRID_SESSION_H

#include "recording.h"
#include "rule.h"
#include "tree.h"

namespace Figrid
{

enum Session_Mode
{
	Session_Mode_None = 0,
	Session_Mode_Library_Read,
	Session_Mode_Library_Write
};

class Session
{
	Session_Mode mode = Session_Mode_None;
	
	Rule* p_rule;
	
	Recording rec;
	Tree tree;
	
	void output_current_node(ostream& ost, bool print_comment, bool multiline) const; //private

public:
	Session(unsigned char board_size, Rule* rule_checker);
	
	Session_Mode current_mode() const;
	string current_mode_str() const;
	bool has_library() const;
	void set_mode(Session_Mode new_mode);
	
	Game_Status game_status() const;
	unsigned short moves_count() const;
	unsigned short match_count() const;
	
	void input(istream& ist, bool multiline = true);

	void node_set_mark(bool val, bool mark_start);
	void node_set_comment(string& comment);
	
	Position_Rotation query_rotate_flag() const; //indicates how the board is rotated to match an existing route in the tree
	void output(ostream& ost, bool show_round_num = true) const;
	void output_game_status(ostream& ost) const;
	void output_node_info(ostream& ost, bool print_comment = false); //single-line comment
	void board_print(ostream& ost, bool use_ascii = false);
	
	void search(NodeSearch* sch);
	
	bool undo(unsigned short steps = 1);
	bool goto_num(unsigned short num);
	bool goto_next();
	bool goto_move(Move mv);
	void go_straight_down();
	void clear();
	void rotate(Position_Rotation rotation);
	void rotate_into_tree();

	bool tree_node_move_left(Move pos);
	bool tree_node_move_right(Move pos);
	void tree_merge_rotations();
	void tree_help_standardize();
	void tree_delete_node();
	
	bool load_pgn_file(const string& file_path);
	bool load_renlib(const string& file_path);
	bool load_node_list(const string& file_path);
	bool load_file(const string& file_path, bool is_node_list = false);
	bool save_renlib(const string& file_path);
	bool save_node_list(const string& file_path);

	const Recording* recording_ptr();
	const Tree* tree_ptr();
};


}
#endif

