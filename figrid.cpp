// Figrid v0.10
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

#include <string>
#include <fstream>

#include "figrid.h"

using namespace std;

namespace Namespace_Figrid
{

Figrid::Figrid(unsigned char board_size, Rule* rule_checker): crec(board_size), ctree(board_size),
                                                              rule(rule_checker)
{
	this->rule->set_recording(&crec);
}

Figrid_Mode Figrid::current_mode() const
{
	return this->mode;
}

string Figrid::current_mode_str() const
{
	switch (this->mode) {
		case Figrid_Mode_None: return "Recording Mode";
		case Figrid_Mode_Library_Read:  return "Library Reading Mode (" + to_string(cntmatch) + ")";
		case Figrid_Mode_Library_Write: return "Library Writing Mode";
	}
	return "??"; //it should be impossible
}

void Figrid::set_mode(Figrid_Mode new_mode)
{
	if (new_mode == this->mode) return;
	
	switch (new_mode) {
		case Figrid_Mode_None:
			this->ctree.pos_goto_root();
			this->ctree.delete_current_pos();
			break;
		case Figrid_Mode_Library_Write:
			this->ctree.write_recording(& this->crec);
			break;
	}
	this->mode = new_mode;
}

Game_Status Figrid::game_status() const
{
	return this->rule->game_status();
}

unsigned short Figrid::moves_count() const
{
	return this->crec.moves_count();
}

unsigned short Figrid::match_count() const
{
	if (this->mode == Figrid_Mode_None) return 0;
	return this->cntmatch;
}

void Figrid::input(istream& ist)
{
	unsigned short ocnt = crec.moves_count(), dcnt;
	this->crec.input(ist);
	dcnt = crec.moves_count() - ocnt;
	if (dcnt == 0) return;
	
	if (dcnt == 1) {
		Move mv = crec.last_move();
		this->crec.undo();
		if (! this->rule->domove(mv)) return;
	} else {
		if (! this->rule->check_recording())
			this->rule->undo_invalid_moves();	
		dcnt = crec.moves_count() - ocnt;
		if (dcnt == 0) return;
	}
	
	if (this->mode == Figrid_Mode_Library_Read) {
		if (dcnt > 1) {
			if (this->ctree.query(& this->crec))
				this->cntmatch = this->ctree.current_depth();
		} else if (this->ctree.query(crec.last_move()) != NULL)
			this->cntmatch++;
	} else if (this->mode == Figrid_Mode_Library_Write) {
		this->ctree.write_recording(& this->crec);
		this->cntmatch = this->crec.moves_count();
	}
}

void Figrid::node_set_mark(bool val, bool mark_start)
{
	if (this->mode != Figrid_Mode_Library_Write) return;

	this->ctree.set_current_mark(val, mark_start);
}

void Figrid::node_set_comment(string& comment)
{
	if (this->mode == Figrid_Mode_Library_Write)
		this->ctree.set_current_comment(comment);
}

Position_Rotation Figrid::query_rotate_tag() const
{
	if (this->mode == Figrid_Mode_Library_Read || this->mode == Figrid_Mode_Library_Write)
		return this->ctree.query_rotate_tag();
	else
		return Rotate_None;
}


void Figrid::output(ostream& ost) const
{
	this->crec.output(ost, true);
}

void Figrid::output_game_status(ostream& ost) const
{
	if (this->rule->game_status() & Game_Status_Ended) {
		if (this->rule->game_status() & Game_Status_First_Mover)
			ost << "Black Wins";
		else if (this->rule->game_status() & Game_Status_Second_Mover)
			ost << "White Wins";
		else
			ost << "Tie";
	} else {
		if (this->rule->game_status() & Game_Status_First_Mover)
			ost << "Black " << this->crec.moves_count() + 1;
		else if (this->rule->game_status() & Game_Status_Second_Mover)
			ost << "White " << this->crec.moves_count() + 1;
	}
}

void Figrid::output_current_node(ostream& ost, bool comment = true, bool multiline = false) const //private
{
	if (this->ctree.query_rotate_tag())
		ost << (string) this->ctree.get_current_move(false) << "->";
	ost << (string) this->ctree.get_current_move();
	
	if (this->ctree.current_mark(true))   ost << ":" << "Start";
	if (this->ctree.current_mark())       ost << ":" << "Mark";
	
	if (comment) {
		string com; this->ctree.get_current_comment(com);
		if (com.length() > 0) {
			size_t n = com.find('\n');
			if (multiline)
				ost << ((n == string::npos)? '\t' : '\n') << com;
			else
				ost << '\t' << (n == string::npos)? com : com.substr(0, n);
		}
		ost << '\n';
	} else
		ost << '\t';
}

void Figrid::output_node_info(ostream& ost, bool descendents_comment)
{
	if (this->mode == Figrid_Mode_None) return;
	
	cout << "Current Node: ";
	this->output_current_node(ost, true, true);
	
	unsigned short deg = this->ctree.current_degree();
	if (deg > 0) {
		this->ctree.pos_move_down();
		for (unsigned short i = 0; i < deg; i++) {
			this->output_current_node(ost, descendents_comment, false);			
			this->ctree.pos_move_right();
		}
		this->ctree.pos_move_up();
	}
	
	if (! descendents_comment) ost << '\n';
}

void Figrid::board_print(ostream& ost)
{
	if (this->mode != Figrid_Mode_None && this->cntmatch >= this->crec.moves_count())
		this->ctree.print_current_board(ost);
	else
		this->crec.board_print(ost);
}
	
void Figrid::undo(unsigned short steps)
{
	this->crec.undo(steps);
	this->rule->check_recording();
	
	if (this->mode != Figrid_Mode_None) {
		for (unsigned short i = 0; i < steps; i++)
			this->ctree.pos_move_up();
		if (this->crec.moves_count() < this->cntmatch)
			this->cntmatch = this->crec.moves_count();
	}
}

void Figrid::goback(unsigned short num)
{
	if (this->crec.moves_count() < num) return;
	this->undo(this->crec.moves_count() - num); 
}

void Figrid::clear()
{
	this->crec.clear();
	if (this->mode != Figrid_Mode_None) {
		this->ctree.pos_goto_root();
		this->cntmatch = 0;
	}
}

void Figrid::rotate_into_tree()
{
	if (this->mode == Figrid_Mode_None) return;
	this->ctree.clear_rotate_tag();
	this->crec = ctree.get_current_recording();
}

void Figrid::tree_goto_fork()
{
	if (this->mode == Figrid_Mode_None) return;
	this->ctree.pos_goto_fork();
	this->crec = ctree.get_current_recording();
}

void Figrid::tree_delete_node()
{
	if (this->mode == Figrid_Mode_None) return;
	this->ctree.delete_current_pos();
	this->crec.undo();
}

bool Figrid::load_pgn_file(string& file_path)
{
	ifstream ifs(file_path, ios_base::in);
	if (! ifs.is_open()) return false;
	
	this->crec.clear();
	this->input(ifs);
	return true;
}

bool Figrid::load_renlib(string& file_path)
{	
	if (! this->ctree.load_renlib(file_path)) return false;
	this->ctree.query(& this->crec);
	this->set_mode(Figrid_Mode_Library_Read); return true;
}

bool Figrid::save_renlib(string& file_path)
{
	if (this->mode != Figrid_Mode_Library_Write) return false;
	this->ctree.save_renlib(file_path); return true;
}

bool Figrid::load_file(string& file_path)
{
	if (this->ctree.is_renlib_file(file_path))
		return this->load_renlib(file_path);
	else
		return this->load_pgn_file(file_path);
}

const Recording* Figrid::recording()
{
	return & this->crec;
}

const Tree* Figrid::tree()
{
	return & this->ctree;
}

}
