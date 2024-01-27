// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#include <cstddef>
#include <string>
#include <fstream>
#include <vector>

#include "session.h"
#include "tree.h"

using namespace std;
using namespace Figrid;

Session::Session(unsigned char board_size, Rule* rule_checker):
	rec(board_size), tree(board_size), p_rule(rule_checker)
{
	this->p_rule->set_recording(&this->rec);
}

Session_Mode Session::current_mode() const
{
	return this->mode;
}

string Session::current_mode_str() const
{
	switch (this->mode) {
		case Session_Mode_None:
			return "Recording Mode";
		case Session_Mode_Library_Read: 
			return "Library Reading Mode (" + to_string(this->match_count()) + ")";
		case Session_Mode_Library_Write:
			return "Library Writing Mode";
	}
	return "??"; //it should be impossible
}

bool Session::has_library() const
{
	return this->mode == Session_Mode_Library_Read
	    || this->mode == Session_Mode_Library_Write;
}

void Session::set_mode(Session_Mode new_mode)
{
	if (new_mode == this->mode) return;
	
	switch (new_mode) {
		case Session_Mode_None:
			this->tree.cur_goto_root();
			this->tree.delete_current_pos();
			break;
		case Session_Mode_Library_Read:
			break;
		case Session_Mode_Library_Write:
			this->tree.write_recording(& this->rec);
			break;
	}
	this->mode = new_mode;
}

Game_Status Session::game_status() const
{
	return this->p_rule->game_status();
}

unsigned short Session::moves_count() const
{
	return this->rec.count();
}

unsigned short Session::match_count() const
{
	if (! this->has_library()) return 0;
	return this->tree.current_depth();
}

void Session::input(istream& ist, bool multiline)
{
	unsigned short cnt_prev = this->rec.count(), cnt_inc;
	this->rec.input(ist, multiline);
	cnt_inc = this->rec.count() - cnt_prev;
	if (cnt_inc == 0) return;
	
	if (cnt_inc == 1) {
		Move mv = this->rec.last_move();
		this->rec.undo();
		if (! this->p_rule->domove(mv)) return;
	} else {
		if (! this->p_rule->check_recording())
			this->p_rule->undo_invalid_moves();	
		cnt_inc = this->rec.count() - cnt_prev;
		if (cnt_inc == 0) return;
	}
	
	if (this->mode == Session_Mode_Library_Read) {
		if (cnt_inc > 1 || this->tree.current_depth() <= 5 || this->query_rotate_flag())
			this->tree.query_recording(& this->rec);
		else
			this->tree.query_move(this->rec.last_move());
	}
	else if (this->mode == Session_Mode_Library_Write)
		this->tree.write_recording(& this->rec);
}

void Session::node_set_mark(bool val, bool mark_start)
{
	if (this->mode != Session_Mode_Library_Write) return;

	this->tree.set_current_mark(val, mark_start);
}

void Session::node_set_comment(string& comment)
{
	if (this->mode == Session_Mode_Library_Write)
		this->tree.set_current_comment(comment);
}

Position_Rotation Session::query_rotate_flag() const
{
	if (this->has_library())
		return this->tree.query_rotate_flag();
	else
		return Rotate_None;
}


void Session::output(ostream& ost, bool show_round_num) const
{
	this->rec.output(ost, show_round_num);
}

void Session::output_game_status(ostream& ost) const
{
	if (this->p_rule->game_status() & Game_Status_Ended) {
		if (this->p_rule->game_status() & Game_Status_First_Mover)
			ost << "Black Wins";
		else if (this->p_rule->game_status() & Game_Status_Second_Mover)
			ost << "White Wins";
		else
			ost << "Tie";
	} else if (this->p_rule->game_status() & Game_Status_Foul) {
		ost << "Foul " << this->p_rule->invalid_moves_count();
	} else {
		if (this->p_rule->game_status() & Game_Status_First_Mover)
			ost << "Black " << this->rec.count() + 1;
		else if (this->p_rule->game_status() & Game_Status_Second_Mover)
			ost << "White " << this->rec.count() + 1;
	}
}

void Session::output_current_node(ostream& ost, bool print_comment = true, bool multiline = false) const //private
{
	if (this->has_library() && this->tree.current_depth() > 0
	&&  this->tree.query_rotate_flag() != Rotate_None)
		ost << (string) this->tree.current_move(true) << "->";
	ost << (string) this->tree.current_move();
	
	if (this->tree.current_mark(true))   ost << ":" << "Start";
	if (this->tree.current_mark())       ost << ":" << "Mark";
	
	if (! print_comment) {
		ost << "    "; return;
	}
	string com; this->tree.get_current_comment(com);
	if (com.length() > 0) {
		size_t n = com.find('\n');
		if (multiline)
			ost << ((n == string::npos)? "  " : "\n") << com;
		else
			ost << '\t' << ((n == string::npos)? com : com.substr(0, n));
	}
	ost << '\n';
}

void Session::output_node_info(ostream& ost, bool print_comment)
{
	if (! this->has_library()) return;
	if (this->match_count() < this->rec.count()) return;
	
	ost << "Current: ";
	this->output_current_node(ost, print_comment, true);
	if (! print_comment) ost << '\n';
	
	unsigned short deg = this->tree.current_degree();
	if (deg == 0) return;

	// print info of its descendents
	this->tree.cur_move_down();
	for (unsigned short i = 0; i < deg; i++) {
		this->output_current_node(ost, false);
		this->tree.cur_move_right();
	}
	this->tree.cur_move_up();
	ost << '\n';
	
	if (! print_comment) return;

	// print single-line comment for each descendent
	this->tree.cur_move_down();
	for (unsigned short i = 0; i < deg; i++) {
		if (! this->tree.current_ptr()->has_comment) continue;
		this->output_current_node(ost, true, false);			
		this->tree.cur_move_right();
	}
	this->tree.cur_move_up();
}

void Session::board_print(ostream& ost, bool use_ascii) const
{
	if (this->has_library() && this->match_count() >= this->rec.count())
		this->tree.print_current_board(ost, use_ascii);
	else
		this->rec.board_print(ost, use_ascii);
}

bool Session::get_current_comment(string& comment) const
{
	if (! this->has_library()) return false;
	return this->tree.get_current_comment(comment);
}

void Session::search(NodeSearch* sch)
{
	if (! this->has_library()) return;

	vector<Recording> vect_rec;
	if (sch->p_result == NULL) sch->p_result = &vect_rec;

	this->tree.search(sch);

	if (sch->match_count == 1) { //only one search result
		this->rec.append(&((* sch->p_result)[0]));
		if (! this->p_rule->check_recording())
			this->p_rule->undo_invalid_moves();
		this->tree.query_recording(& this->rec);
	}
}

bool Session::undo(unsigned short steps)
{
	if (steps > this->rec.count()) return false;
	return this->goto_num(this->rec.count() - steps);
}

bool Session::goto_num(unsigned short num)
{
	if (! this->rec.goto_num(num)) return false;

	Recording rec_back = this->rec;
	if (! this->p_rule->check_recording())
		this->p_rule->undo_invalid_moves();

	if (this->has_library()) {
		this->tree.cur_goto_root();
		this->tree.query_recording(& this->rec);
	}

	return true;
}

bool Session::goto_next()
{
	if (this->has_library() && this->tree.current_degree() == 1) {
		this->tree.cur_move_down();
		if (this->p_rule->domove(this->tree.current_move()))
			return true;
		else
			this->tree.cur_move_up();
	}

	return this->goto_num(this->rec.count() + 1);
}

bool Session::goto_move(Move mv)
{
	if (! this->rec.goto_move(mv)) return false;
	return this->goto_num(this->rec.count());
}

void Session::go_straight_down()
{
	if (! this->has_library()) {
		this->rec.goto_num(this->rec.count_all());
	} else {
		if (this->rec.count() > this->match_count()) return;
		
		this->tree.cur_goto_fork();
		this->rec = this->tree.get_current_recording();
		if (! this->p_rule->check_recording()) { //an invalid route exists in the tree
			for (unsigned short i = 0; i < this->p_rule->invalid_moves_count(); i++)
				this->tree.cur_move_up();
			this->p_rule->undo_invalid_moves();
		}
	}
}

void Session::clear()
{
	this->rec.clear();
	if (this->has_library())
		this->tree.cur_goto_root();
	
	this->p_rule->check_recording();
}

void Session::rotate(Position_Rotation rotation)
{
	this->rec.board_rotate(rotation);
	if (this->has_library())
		this->tree.query_recording(& this->rec);
}

void Session::rotate_into_tree()
{
	if (! this->has_library()) return;
	this->tree.clear_rotate_flag();
	this->rec = this->tree.get_current_recording();
}

bool Session::tree_node_move_left(Move pos)
{
	if (this->mode != Session_Mode_Library_Write) return false;
	if (this->rec.count() > this->match_count()) return false;
	return this->tree.node_move_left(pos);
}

bool Session::tree_node_move_right(Move pos)
{
	if (this->mode != Session_Mode_Library_Write) return false;
	if (this->rec.count() > this->match_count()) return false;
	return this->tree.node_move_right(pos);
}

void Session::tree_merge_rotations()
{
	if (this->mode != Session_Mode_Library_Write) return;
	this->tree.merge_rotations();
}

void Session::tree_help_standardize()
{
	if (this->mode != Session_Mode_Library_Write) return;
	this->tree.help_standardize();
}

void Session::tree_delete_node()
{
	if (this->mode != Session_Mode_Library_Write) return;
	this->tree.delete_current_pos();
	this->rec.undo();
}

bool Session::load_pgn_file(const string& file_path)
{
	ifstream ifs(file_path, ios::in);
	if (! ifs.is_open()) return false;
	
	this->clear();
	this->input(ifs);
	return true;
}

bool Session::load_renlib(const string& file_path)
{	
	if (! this->tree.load_renlib(file_path)) return false;

	this->tree.query_recording(& this->rec);
	this->set_mode(Session_Mode_Library_Read); return true;
}

bool Session::load_node_list(const string& file_path)
{
	ifstream ifs(file_path, ios::in);
	if (! ifs.is_open()) return false;
	
	bool prev_writing = (this->mode == Session_Mode_Library_Write);

	this->set_mode(Session_Mode_Library_Write);
	while (ifs && !ifs.eof()) {
		this->clear();
		this->input(ifs, false);
	}

	if (! prev_writing)
		this->set_mode(Session_Mode_Library_Read);
	this->tree.query_recording(& this->rec);
	return true;
}

bool Session::load_file(const string& file_path, bool is_node_list)
{
	if (this->tree.is_renlib_file(file_path))
		return this->load_renlib(file_path);
	else {
		if (! is_node_list)
			return this->load_pgn_file(file_path);
		else
			return this->load_node_list(file_path);
	}
}

bool Session::save_renlib(const string& file_path)
{
	if (! this->has_library()) return false;
	return this->tree.save_renlib(file_path);
}

bool Session::save_node_list(const string& file_path)
{
	if (ifstream(file_path, ios::in)) return false; //file exists
	ofstream ofs(file_path, ios::out);
	if (! ofs.is_open()) return false;

	if (this->has_library()) {
		NodeSearch sch; 
		sch.mode = Node_Search_Leaf;
		sch.direct_output = true; sch.p_ost = &ofs;
		this->tree.cur_goto_root();
		this->tree.search(&sch);
	} else {
		this->rec.output(ofs); ofs << '\n';
	}

	ofs.flush(); ofs.close();

	this->tree.query_recording(& this->rec);
	return true;
}

const Recording* Session::recording_ptr()
{
	return & this->rec;
}

const Tree* Session::tree_ptr()
{
	return & this->tree;
}

