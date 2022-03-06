// Figrid v0.15
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#include <string>
#include <fstream>

#include "figrid.h"

using namespace std;
using namespace Namespace_Figrid;

Figrid::Figrid(unsigned char board_size, Rule* rule_checker): rec(board_size), tree(board_size),
                                                              p_rule(rule_checker)
{
	this->p_rule->set_recording(&this->rec);
}

Figrid_Mode Figrid::current_mode() const
{
	return this->mode;
}

string Figrid::current_mode_str() const
{
	switch (this->mode) {
		case Figrid_Mode_None: return "Recording Mode";
		case Figrid_Mode_Library_Read:  return "Library Reading Mode (" + to_string(this->match_count()) + ")";
		case Figrid_Mode_Library_Write: return "Library Writing Mode";
	}
	return "??"; //it should be impossible
}

void Figrid::set_mode(Figrid_Mode new_mode)
{
	if (new_mode == this->mode) return;
	
	switch (new_mode) {
		case Figrid_Mode_None:
			this->tree.cur_goto_root();
			this->tree.delete_current_pos();
			break;
		case Figrid_Mode_Library_Write:
			this->tree.write_recording(& this->rec);
			break;
	}
	this->mode = new_mode;
}

Game_Status Figrid::game_status() const
{
	return this->p_rule->game_status();
}

unsigned short Figrid::moves_count() const
{
	return this->rec.moves_count();
}

unsigned short Figrid::match_count() const
{
	if (this->mode == Figrid_Mode_None) return 0;
	return this->tree.current_depth();
}

void Figrid::input(istream& ist)
{
	unsigned short ocnt = this->rec.moves_count(), dcnt;
	this->rec.input(ist);
	dcnt = this->rec.moves_count() - ocnt;
	if (dcnt == 0) return;
	
	if (dcnt == 1) {
		Move mv = this->rec.last_move();
		this->rec.undo();
		if (! this->p_rule->domove(mv)) return;
	} else {
		if (! this->p_rule->check_recording())
			this->p_rule->undo_invalid_moves();	
		dcnt = this->rec.moves_count() - ocnt;
		if (dcnt == 0) return;
	}
	
	if (this->mode == Figrid_Mode_Library_Read) {
		if (dcnt > 1 || this->tree.current_depth() <= 5)
			this->tree.query(& this->rec);
		else
			this->tree.query(this->rec.last_move());
	} else if (this->mode == Figrid_Mode_Library_Write)
		this->tree.write_recording(& this->rec);
}

void Figrid::node_set_mark(bool val, bool mark_start)
{
	if (this->mode != Figrid_Mode_Library_Write) return;

	this->tree.set_current_mark(val, mark_start);
}

void Figrid::node_set_comment(string& comment)
{
	if (this->mode == Figrid_Mode_Library_Write)
		this->tree.set_current_comment(comment);
}

Position_Rotation Figrid::query_rotate_tag() const
{
	if (this->mode == Figrid_Mode_Library_Read || this->mode == Figrid_Mode_Library_Write)
		return this->tree.query_rotate_tag();
	else
		return Rotate_None;
}


void Figrid::output(ostream& ost, bool show_round_num) const
{
	this->rec.output(ost, show_round_num);
}

void Figrid::output_game_status(ostream& ost) const
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
			ost << "Black " << this->rec.moves_count() + 1;
		else if (this->p_rule->game_status() & Game_Status_Second_Mover)
			ost << "White " << this->rec.moves_count() + 1;
	}
}

void Figrid::output_current_node(ostream& ost, bool comment = true, bool multiline = false) const //private
{
	if ((this->tree.query_rotate_tag() != Rotate_None) && this->tree.current_depth() > 0)
		ost << (string) this->tree.get_current_move(false) << "->";
	ost << (string) this->tree.get_current_move();
	
	if (this->tree.current_mark(true))   ost << ":" << "Start";
	if (this->tree.current_mark())       ost << ":" << "Mark";
	
	if (comment) {
		string com; this->tree.get_current_comment(com);
		if (com.length() > 0) {
			size_t n = com.find('\n');
			if (multiline)
				ost << ((n == string::npos)? "  " : "\n") << com;
			else
				ost << '\t' << ((n == string::npos)? com : com.substr(0, n));
		}
		ost << '\n';
	} else
		ost << "    ";
}

void Figrid::output_node_info(ostream& ost, bool comment)
{
	if (this->mode == Figrid_Mode_None) return;
	if (this->match_count() < this->rec.moves_count()) return;
	
	ost << "Current: ";
	this->output_current_node(ost, comment, true);
	if (! comment) ost << '\n';
	
	unsigned short deg = this->tree.current_degree();
	if (deg > 0) {
		this->tree.cur_move_down();
		for (unsigned short i = 0; i < deg; i++) {
			this->output_current_node(ost, false);			
			this->tree.cur_move_right();
		}
		this->tree.cur_move_up();
		ost << '\n';
		
		if (comment) {
			this->tree.cur_move_down();
			for (unsigned short i = 0; i < deg; i++) {
				if (! this->tree.current_ptr()->has_comment) continue;
				this->output_current_node(ost, true, false);			
				this->tree.cur_move_right();
			}
			this->tree.cur_move_up();
		}
	}
}

void Figrid::board_print(ostream& ost, bool use_ascii)
{
	if (this->mode != Figrid_Mode_None && this->match_count() >= this->rec.moves_count())
		this->tree.print_current_board(ost, use_ascii);
	else
		this->rec.board_print(ost, 0, NULL, use_ascii);
}

void Figrid::search(Node_Search* sch)
{
	if (this->mode != Figrid_Mode_None) {
		this->tree.search(sch);
		if (sch->result->size() == 1) { //only one search result
			this->rec.append(&((* sch->result)[0]));
			if (! this->p_rule->check_recording())
				this->p_rule->undo_invalid_moves();
			this->tree.cur_goto_root(); //clear rotate tag
			this->tree.query(& this->rec);
		}
	}
}

void Figrid::undo(unsigned short steps)
{
	if (steps == 0) return;
	this->rec.undo(steps);
	this->p_rule->check_recording();
	
	if (this->mode != Figrid_Mode_None) {
		if (this->rec.moves_count() <= 5) { //check if rotation is still needed
			this->tree.cur_goto_root(); //and clear rotate tag
			this->tree.query(&this->rec);
		} else if (this->rec.moves_count() < this->match_count()) {
			for (unsigned short i = 0; i < this->match_count() - this->rec.moves_count(); i++)
				this->tree.cur_move_up();
		}
	}
}

void Figrid::goback(unsigned short num)
{
	if (this->rec.moves_count() < num) return;
	this->undo(this->rec.moves_count() - num); 
}

void Figrid::clear()
{
	this->rec.clear();
	if (this->mode != Figrid_Mode_None)
		this->tree.cur_goto_root();
	
	this->p_rule->check_recording();
}

void Figrid::rotate(Position_Rotation rotation)
{
	this->rec.board_rotate(rotation);
	if (this->mode != Figrid_Mode_None) {
		this->tree.cur_goto_root(); //it involves clearing rotate tag
		this->tree.query(&this->rec);
	}
}

void Figrid::rotate_into_tree()
{
	if (this->mode == Figrid_Mode_None) return;
	this->tree.clear_rotate_tag();
	this->rec = this->tree.get_current_recording();
}

void Figrid::tree_goto_fork()
{
	if (this->mode == Figrid_Mode_None) return;
	if (this->rec.moves_count() > this->match_count()) return;
	
	this->tree.cur_goto_fork();
	this->rec = this->tree.get_current_recording();
	if (! this->p_rule->check_recording()) { //an invalid route exists in the tree
		for (unsigned short i = 0; i < this->p_rule->invalid_moves_count(); i++)
			this->tree.cur_move_up();
		this->p_rule->undo_invalid_moves();
	}
}

void Figrid::tree_delete_node()
{
	if (this->mode != Figrid_Mode_Library_Write) return;
	this->tree.delete_current_pos();
	this->rec.undo();
}

bool Figrid::load_pgn_file(const string& file_path)
{
	ifstream ifs(file_path, ios_base::in);
	if (! ifs.is_open()) return false;
	
	this->rec.clear();
	this->input(ifs);
	return true;
}

bool Figrid::load_renlib(const string& file_path)
{	
	if (! this->tree.load_renlib(file_path)) return false;
	this->tree.query(& this->rec);
	this->set_mode(Figrid_Mode_Library_Read); return true;
}

bool Figrid::save_renlib(const string& file_path)
{
	if (this->mode != Figrid_Mode_Library_Write) return false;
	this->tree.save_renlib(file_path); return true;
}

bool Figrid::load_file(const string& file_path)
{
	if (this->tree.is_renlib_file(file_path))
		return this->load_renlib(file_path);
	else
		return this->load_pgn_file(file_path);
}

const Recording* Figrid::recording_ptr()
{
	return & this->rec;
}

const Tree* Figrid::tree_ptr()
{
	return & this->tree;
}

