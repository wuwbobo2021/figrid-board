// Figrid v0.10
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

#ifndef FIGRID_FIGRID_H
#define FIGRID_FIGRID_H

#include "recording.h"
#include "rule.h"
#include "rule_gomoku_original.h"
#include "tree.h"

namespace Namespace_Figrid
{

enum Figrid_Mode
{
	Figrid_Mode_None = 0,
	Figrid_Mode_Library_Read,
	Figrid_Mode_Library_Write
};

class Figrid
{
	Figrid_Mode mode = Figrid_Mode_None;
	
	Rule* rule;
	
	Recording crec;
	Tree ctree;
	unsigned short cntmatch = 0;
	
	void output_current_node(ostream& ost, bool comment, bool multiline) const; //private

public:
	Figrid(unsigned char board_size, Rule* rule_checker);
	
	Figrid_Mode current_mode() const;
	string current_mode_str() const;
	void set_mode(Figrid_Mode new_mode);
	
	Game_Status game_status() const;
	unsigned short moves_count() const;
	unsigned short match_count() const;
	
	void input(istream& ist);

	void node_set_mark(bool val, bool mark_start);
	void node_set_comment(string& comment);
	
	Position_Rotation query_rotate_tag() const; //indicates how the board is rotated to match an existing route in the tree
	void output(ostream& ost) const;
	void output_game_status(ostream& ost) const;
	void output_node_info(ostream& ost, bool descendents_comment = false); //`with_comment`: outputs one-line comments
	void board_print(ostream& ost);
	
	void undo(unsigned short steps = 1);
	void goback(unsigned short num);
	void clear();
	void rotate(Position_Rotation rotation);
	void rotate_into_tree();
	void tree_goto_fork();
	void tree_delete_node();
	
	bool load_pgn_file(string& file_path);
	bool load_renlib(string& file_path);
	bool load_file(string& file_path); //implements simplest file format auto checking
	bool save_renlib(string& file_path);
	
	const Recording* recording();
	const Tree* tree();
};


}
#endif

