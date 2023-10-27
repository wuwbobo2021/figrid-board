// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

// Original Renlib code: <https://www.github.com/gomoku/Renlib>, by Frank Arkbo.

#include <cstdint>
#include <locale> //only for `tolower()`
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include "recording.h"
#include "tree.h"

using namespace std;
using namespace Figrid;

const int Renlib_Header_Size = 20;	
//                                   0      1     2     3     4     5     6    7
unsigned char Renlib_Header[20] = { 0xFF,  'R',  'e',  'n',  'L',  'i',  'b', 0xFF,
//                                  VER    VER   10    11    12    13    14   15
                                    0x03, 0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//                                  16     17    18    19
                                    0xFF, 0xFF, 0xFF, 0xFF };

struct RenlibNode //it corresponds with every node in a Renlib file
{
	uint8_t x: 4; // bit 0~3
	uint8_t y: 4; // bit 4~7

	// byte 2, bit 0~7
	bool extension: 1; // reserved? ignored
	bool no_move: 1;   // reserved? ignored
	bool start: 1;
	bool comment: 1;
	bool mark: 1;
	bool old_comment: 1;
	bool is_leaf: 1; //marked with right commas ): it has no left descendent. ("right" in original Renlib code)
	bool has_sibling: 1; //left commas (: it has a right sibling. ("down" in original Renlib code)
	// Reference: Data Structure Techniques by Thomas A. Standish, 3.5.2, Algorithm 3.4
};

Tree::Tree(unsigned short board_sz):
	board_size(board_sz), rec(board_sz) //the recording is initialized here
{
	this->root = new Node;
	this->seq = new Node*[board_size * board_size];
	this->cur_goto_root();
	
	for (unsigned char i = 0; i < 8; i++) //prepare for function query_recording()
		this->rotations.push_back(Recording(board_size));
}

Tree::~Tree()
{
	this->cur_goto_root();
	this->delete_current_pos();
	delete this->root;

	if (this->seq != NULL) {
		delete[] this->seq;
		this->seq = NULL;
	}
}

unsigned short Tree::current_depth() const
{
	return this->cur_depth;
}

unsigned short Tree::current_degree() const
{
	if (this->cur == NULL || this->cur->down == NULL) return 0;
	
	unsigned short cnt = 0; Node* p = this->cur->down;
	while (p != NULL) {
		cnt++;
		p = p->right;
	}
	return cnt;
}

Move Tree::current_move(bool disable_rotation) const
{
	Move mv;
	if (this->cur == NULL) return mv; //return null move
	
	mv = this->cur->pos;
	if (! disable_rotation)
		mv.rotate(this->board_size, reverse_rotation(this->flag_rotate)); //rotate back
	return mv;
}

Recording Tree::get_current_recording(bool disable_rotation) const
{
	Recording rec = this->rec;
	if (! disable_rotation)
		rec.board_rotate(reverse_rotation(this->flag_rotate));
	return rec;
}

void Tree::print_current_board(ostream& ost, bool use_ascii) const
{
	Recording rec = this->get_current_recording(false); //rotate back to meet original query
	
	Move dots[this->current_degree()];
	if (this->cur != NULL && this->cur->down != NULL) {
		Node* p = this->cur->down;
		Position_Rotation ro_back = reverse_rotation(this->flag_rotate);
		for (unsigned short i = 0; i < this->current_degree(); i++) {
			dots[i] = p->pos;
			dots[i].rotate(this->board_size, ro_back); //rotate back
			p = p->right;
		}
		rec.board_print(ost, use_ascii, dots, this->current_degree());
	} else
		rec.board_print(ost, use_ascii);
}

bool Tree::current_mark(bool mark_start) const
{
	if (this->cur == NULL) return false;
	
	if (mark_start)
		return this->cur->marked_start;
	else
		return this->cur->marked; 
}

void Tree::set_current_mark(bool val, bool mark_start)
{
	if (this->cur == NULL) return;
	
	if (mark_start)
		this->cur->marked_start = val;
	else
		this->cur->marked = val;
}

bool Tree::get_current_comment(string& comment) const
{
	if (this->cur == NULL) return false;
	if (! this->cur->has_comment) return false;
	
	comment = this->comments[this->cur->tag_comment];
	return true;
}

void Tree::set_current_comment(string& comment)
{
	if (this->cur == NULL) return;
	
	if (comment != "") {
		if (this->cur->has_comment)
			this->comments[this->cur->tag_comment] = comment;
		else {
			this->comments.push_back(comment);
			this->cur->has_comment = true;
			this->cur->tag_comment = this->comments.size() - 1;
		}
	} else
		this->cur->has_comment = false;
}
	
bool Tree::cur_move_down()
{
	if (this->cur == NULL) return false;
	if (this->cur->down == NULL) return false;
	
	this->cur = this->cur->down;
	this->cur_depth++;
	this->seq[this->cur_depth] = this->cur;
	this->rec.domove(this->cur->pos);
	return true;
}

bool Tree::cur_move_up()
{
	if (this->cur_depth < 1) return false;
	
	this->cur_depth--;
	this->cur = this->seq[this->cur_depth];
	this->rec.undo();
	return true;
}

bool Tree::cur_move_right()
{
	if (this->cur == NULL) return false;
	if (this->cur->right == NULL) return false;
	
	this->cur = this->cur->right;
	this->seq[this->cur_depth] = this->cur;
	this->rec.undo(); this->rec.domove(this->cur->pos);
	return true;
}

bool Tree::cur_move_left()
{
	if (this->cur_depth < 1) return false;
	
	Node* p = this->seq[this->cur_depth - 1]->down; //goto the left sibling
	while (p != NULL && p->right != this->cur)
		p = p->right;
	
	if (p == NULL || p->right != this->cur) return false;
	this->cur = p;
	this->seq[this->cur_depth] = this->cur;
	this->rec.undo(); this->rec.domove(this->cur->pos);
	return true;
}

void Tree::cur_goto_root()
{
	this->cur = this->root;
	this->cur_depth = 0;
	this->seq[this->cur_depth] = this->root;
	this->rec.clear();
	this->flag_rotate = Rotate_None;
}

bool Tree::cur_goto_fork()
{
	if (this->cur == NULL) return false;
	
	while (this->cur->down != NULL) {
		if (this->cur->down->right != NULL) return true;
		this->cur = this->cur->down;
		this->cur_depth++;
		this->seq[this->cur_depth] = this->cur;
		this->rec.domove(this->cur->pos);
	}
	
	return false;
}

bool Tree::query_move(Move pos)
{
	if (this->cur == NULL) return false;
	if (this->cur->down == NULL) return false;
	
	Node* p = this->cur->down;
	while (p != NULL) {
		if (p->pos == pos) {
			this->cur = p;
			this->cur_depth++;
			this->seq[this->cur_depth] = p;
			this->rec.domove(pos);
			return true;
		} else
			p = p->right;
	}
	
	return false;
}

unsigned short Tree::fixed_query(const Recording* record) //private, doesn't involve rotation
{
	if (record->count() < 1) return 0;
	if (this->root->down == NULL) return 0; //the tree is empty
	
	Position_Rotation flag_bak = this->flag_rotate;
	this->cur_goto_root(); this->flag_rotate = flag_bak;
	
	unsigned short mcnt;
	for (mcnt = 0; mcnt < record->count(); mcnt++) {
		// check next move
		if (! this->cur_move_down()) return mcnt;
		bool found = false;
		do {
			if ((*record)[mcnt] == this->cur->pos) { //check the <mcnt + 1>th move
				found = true; break;
			}
		} while (this->cur_move_right());
		
		if (! found) {
			this->cur_move_up(); return mcnt;
		}
	} //mcnt++
	return mcnt;
}

unsigned short Tree::query_recording(const Recording* record) //it might set the rotate flag
{
	if (record->count() < 1) return 0;
	if (this->root->down == NULL) return 0; //the tree is empty
	
	unsigned short mcnt[8] = {0};
	for (unsigned char r = 0; r < 8; r++) {
		this->rotations[r] = *record;
		this->rotations[r].board_rotate((Position_Rotation) r);
		mcnt[r] = this->fixed_query(& rotations[r]);
		if (mcnt[r] == this->rotations[r].count()) { //completely matched
			this->flag_rotate = (Position_Rotation) r;
			return mcnt[r];
		}
	}
	
	unsigned char idx = 0;
	for (unsigned char r = 0; r < 8; r++)
		if (mcnt[r] > mcnt[idx]) idx = r;
	
	if (mcnt[idx] == 0) return 0;
	this->flag_rotate = (Position_Rotation) idx;
	return this->fixed_query(& this->rotations[idx]);
}


Position_Rotation Tree::query_rotate_flag() const
{
	return this->flag_rotate;
}

void Tree::clear_rotate_flag()
{
	this->flag_rotate = Rotate_None;
}

static inline void string_to_lower_case(string& str)
{
	for (unsigned int i = 0; i < str.length(); i++)
		str[i] = tolower(str[i]);
}

void Tree::search(NodeSearch* sch) const
{
	if (this->cur == NULL || sch == NULL) return;
	if (sch->direct_output && sch->p_ost == NULL) return;
	if (!sch->direct_output && sch->p_result == NULL) return;
	sch->match_count = 0;

	bool rotate = (!sch->disable_rotation && this->flag_rotate != Rotate_None);

	Move spos = sch->pos; if (rotate) spos.rotate(this->board_size, this->flag_rotate);
	string sstr = sch->str; string_to_lower_case(sstr);
	
	Node* psubroot = this->cur; Node* pcur = psubroot;
	stack<Node*> node_stack; Recording tmprec(this->board_size);
	if (sch->keep_cur_rec_in_result) tmprec = this->rec;
	
	Recording* precadd; Recording tmprec_ro(this->board_size);
	if (rotate) precadd = &tmprec_ro; else precadd = &tmprec;
	
	while (true) {
		bool suc = true;
		if (sch->mode == Node_Search_Leaf) {
			if (pcur->down != NULL) suc = false; //only search for leaves
		} else {
			if (sch->mode & Node_Search_Mark)
				if (! pcur->marked) suc = false;
			if (suc)
				if (sch->mode & Node_Search_Start)
					if (! pcur->marked_start) suc = false;
			if (suc)
				if (sch->mode & Node_Search_Position)
					if (pcur->pos != spos) suc = false;
			if (suc)
				if (sch->mode & Node_Search_Comment) {
					if (! pcur->has_comment)
						suc = false;
					else {
						string strlower = this->comments[pcur->tag_comment];
						string_to_lower_case(strlower); //case insensitive
						if (strlower.find(sstr) == string::npos)
							suc = false;
					}
				}
		}
		
		if (suc) {
			sch->match_count++;
			if (rotate) {
				tmprec_ro = tmprec;
				tmprec_ro.board_rotate(reverse_rotation(this->flag_rotate)); //rotate back
			}
			if (sch->direct_output) {
				if (precadd->count())
					precadd->output(* sch->p_ost);
				else
					cout << "(Current)" << '\n';
				(* sch->p_ost) << '\n';
			}
			if (!sch->direct_output || (sch->p_result && sch->match_count == 1))
				sch->p_result->push_back(*precadd);
		}
		
		if (pcur->down != NULL && !(suc && (sch->mode & Node_Search_Position))) {
			if (pcur->right != NULL && pcur != psubroot)
				node_stack.push(pcur);
			pcur = pcur->down;
			tmprec.domove(pcur->pos);
		}
		else if (pcur->right != NULL) {
			if (pcur == psubroot) break;
			pcur = pcur->right;
			tmprec.undo(); tmprec.domove(pcur->pos);
		}
		else if (! node_stack.empty()) {
			tmprec.goto_move(node_stack.top()->pos, true); tmprec.undo();
			pcur = node_stack.top()->right; node_stack.pop();
			tmprec.domove(pcur->pos);
		}
		else break;
	}
}

bool Tree::write_move(Move pos)
{
	if (this->cur == NULL) return false;
	if (this->query_move(pos)) return false;

	if (! this->rec.domove(pos)) return false;

	Node* p = NULL; //node to be created
	if (this->cur->down == NULL)
		p = this->cur->down = new Node;
	else {
		p = this->cur->down;
		while (p->right != NULL)
			p = p->right;
		
		p->right = new Node;
		p = p->right; 
	}
	if (p == NULL) { //Out of memory
		this->rec.undo(); return false;
	}

	this->cur = p; this->cur_depth++;
	this->seq[this->cur_depth] = this->cur;
	this->cur->pos = pos;
	return true;
}

bool Tree::node_move_left(Move pos)
{
	if (! this->query_move(pos)) {
		pos.rotate(this->board_size, this->flag_rotate);
		if (! this->query_move(pos)) return false;
	}
	
	Node* p = this->cur;
	if (! this->cur_move_left()) {
		this->cur_move_up(); return false;
	}
	
	Node* pl = this->cur, *pll = NULL;
	if (this->cur_move_left()) pll = this->cur;
	this->cur_move_up();

	// before: (pll)  pl  p   (pr)
	// after:  (pll)  p   pl  (pr)

	if (pll == NULL) //pl is the left descendent
		this->cur->down = p;
	else
		pll->right = p;

	pl->right = p->right; p->right = pl;
	return true;
}

bool Tree::node_move_right(Move pos)
{
	if (! this->query_move(pos)) {
		pos.rotate(this->board_size, this->flag_rotate);
		if (! this->query_move(pos)) return false;
	}

	Node* p = this->cur;
	if (p->right == NULL) {
		this->cur_move_up(); return false;
	}
	
	Node* pl = NULL, *pr = p->right;
	if (this->cur_move_left()) pl = this->cur;
	this->cur_move_up();

	// before: (pl)  p   pr  (prr)
	// after:  (pl)  pr  p   (prr)

	if (pl == NULL) //p is the left descendent
		this->cur->down = pr;
	else
		pl->right = pr;

	p->right = pr->right; pr->right = p;
	return true;
}

bool Tree::write_recording(const Recording* record, bool disable_rotation)
{
	if (this->cur == NULL || record->count() == 0) return false;
	
	Recording rec = *record;
	this->cur_goto_root(); //involves clearing rotate flag (check for rotation again)
	unsigned short cnt_exist;
	if (! disable_rotation) {
		cnt_exist = this->query_recording(&rec); //count of existing moves (begins from root)
		rec.board_rotate(this->flag_rotate);
	} else
		cnt_exist = this->fixed_query(&rec);
	
	if (cnt_exist == rec.count()) return false; //nothing to write
	
	for (unsigned short i = cnt_exist; i < rec.count(); i++)
		if (! this->write_move(rec[i])) return false;
	
	return true;
}

bool Tree::merge_sub_tree(Node* src_root, Position_Rotation prerotation)
{
	if (this->cur == NULL || src_root == NULL || this->cur == src_root)
		return false;
	Move mv = src_root->pos; mv.rotate(this->board_size, prerotation);
	if (mv != this->current_move()) return false;

	Recording cur_rec = this->get_current_recording();
	this->write_recording(&cur_rec, true); //write exactly without rotation

	Node* psrc = src_root; stack<Node*> node_stack;
	while (true) {
		// sync with the node in the source tree
		mv = psrc->pos; mv.rotate(this->board_size, prerotation);
		bool suc = this->write_move(mv) || (psrc == src_root);

		if (suc) {
			if (psrc->marked) this->cur->marked = true;
			if (psrc->marked_start) this->cur->marked_start = true;
			if (this->cur->has_comment && psrc->has_comment)
				this->comments[this->cur->tag_comment]
					+= "\nMerged Comment:\n" + this->comments[psrc->tag_comment];
			else if (psrc->has_comment)
				this->set_current_comment(this->comments[psrc->tag_comment]);
		}

		// determine the next node in the source tree that needs to be merged,
		// and make sure this->cur can be the parent of that next node
		if (suc && psrc->down != NULL) {
			if (psrc->right != NULL && psrc != src_root)
				node_stack.push(psrc);
			psrc = psrc->down;
		}
		else if (psrc->right != NULL) {
			if (psrc == src_root) break;
			this->cur_move_up();
			psrc = psrc->right;
		}
		else if (! node_stack.empty()) {
			mv = node_stack.top()->pos; mv.rotate(this->board_size, prerotation);
			this->rec.goto_move(mv, true); this->rec.undo();
			this->cur_depth = this->rec.count();
			this->cur = this->seq[this->cur_depth];

			psrc = node_stack.top()->right; node_stack.pop();
		}
		else break;
	}
	
	this->query_recording(&cur_rec);
	return true;
}

bool Tree::merge_rotations()
{
	if (this->cur == NULL) return false;

	Recording rec_base = this->get_current_recording();
	this->write_recording(&rec_base, true); //write current recording exactly

	for (unsigned char r = 1; r < 8; r++) {
		Recording rec_ro = rec_base;
		rec_ro.board_rotate((Position_Rotation) r);
		if (rec_ro == rec_base) continue;
		if (this->fixed_query(&rec_ro) < rec_ro.count()) continue;

		Node* psrc = this->cur;
		this->fixed_query(&rec_base);
		this->merge_sub_tree(psrc, reverse_rotation((Position_Rotation) r));

		this->fixed_query(&rec_ro);
		this->delete_current_pos();
	}

	this->fixed_query(&rec_base);
	return true;
}

void Tree::help_standardize()
{
	if (this->root == NULL || this->root->down == NULL) return;

	Recording rec_back = this->get_current_recording();

	Recording tmprec(this->board_size);
	for (unsigned short i = 0; i < this->board_size; i++) {
		for (unsigned short j = 0; j < this->board_size; j++) {
			Move mv(i, j); mv.standardize(this->board_size);
			tmprec.clear(); tmprec.domove(mv);
			if (this->query_recording(&tmprec) < 1) continue;
			this->merge_rotations();
		}
	}

	if (this->board_size % 2 == 0) {
		this->query_recording(&rec_back); return;
	}

	unsigned short sz_half = this->board_size / 2;
	tmprec.clear(); tmprec.domove(Move(sz_half, sz_half)); //central point

	for (unsigned short i = 0; i < this->board_size; i++) {
		for (unsigned short j = 0; j < this->board_size; j++) {
			if (i == sz_half && j == sz_half) continue;
			Move mv(i, j); mv.standardize(this->board_size);
			tmprec.domove(mv);
			if (this->query_recording(&tmprec) == 2)
				this->merge_rotations();
			tmprec.undo();
		}
	}

	this->query_recording(&rec_back);
}

void Tree::delete_current_pos()
{
	if (this->cur == NULL) return;
	bool del_root = (this->cur == this->root);
	if (! del_root) {
		Node* pleft = NULL;
		if (this->cur_move_left()) {
			pleft = this->cur; this->cur_move_right();
		}
		if (pleft != NULL)
			pleft->right = this->cur->right;
		else
			this->seq[this->cur_depth - 1]->down = this->cur->right;
	}
	
	// delete all nodes in postorder sequence
	Node* psubroot = this->cur; Node* pcur = psubroot; Node* ptmp;
	stack<Node*> node_stack;
	while (true) {
		if (pcur->down != NULL) {
			node_stack.push(pcur);
			pcur = pcur->down;
		} else {
			if (pcur->has_comment) {
				this->comments[pcur->tag_comment].clear();
				this->comments[pcur->tag_comment].shrink_to_fit(); //release memory usage
			}
			
			if (pcur == psubroot) {delete pcur;  break;}
			
			if (pcur->right != NULL) { //delete current node and goto right of it
				ptmp = pcur->right;
				delete pcur;
				pcur = ptmp;
			} else { //delete current node and go up
				delete pcur;
				if (node_stack.empty()) break;
				pcur = node_stack.top(); node_stack.pop();
				pcur->down = NULL; //mark
			}
		}
	}
	
	if (! del_root) {
		this->cur = this->seq[--this->cur_depth];
		this->rec.undo();
	} else { //the tree object is being destructed, or a library file is to be opened
		this->comments.clear();
		this->root = new Node;
		this->cur_goto_root();
	}
}

const Node* Tree::root_ptr() const
{
	return this->root;
}

const Node* Tree::current_ptr() const
{
	return this->cur;
}

/*------------------------------ functions for Renlib ------------------------------*/

bool Tree::is_renlib_file(const string& file_path)
{
	char head[Renlib_Header_Size + 2];
	ifstream ifs(file_path, ios::binary | ios::in);
	if (! ifs.is_open()) return false;
	ifs.read(head, Renlib_Header_Size + 2);
	if (! ifs) return false;
	
	for (unsigned char i = 0; i < 8; i++)
		if ((unsigned char) head[i] != Renlib_Header[i]) return false;
	
	return true;
}

static void string_manage_multiline(string& str, bool back_to_renlib = false)
{
	// In Renlib file, 0x08 '\b' indicates multiline comment
	// at the end of first comment line, the others are "\r\n"
	size_t pos = str.find(back_to_renlib? "\n" : "\b"); 
	if (pos == string::npos) return;
	str.replace(pos, 1, back_to_renlib? "\b" : "\n");
	pos++;
	
	string strfind = (back_to_renlib? "\n" : "\r\n"); //Note: don't call this function for double times
	size_t strfind_len = (back_to_renlib? 1 : 2);
	string strto = (back_to_renlib? "\r\n" : "\n");
	size_t strto_len = (back_to_renlib? 2 : 1);
	
	while (true) {
		pos = str.find(strfind, pos);
		if (pos == string::npos) break;
		str.replace(pos, strfind_len, strto);
		pos += strto_len;
	}
}

bool Tree::load_renlib(const string& file_path)
{
	if (this->board_size != 15) throw InvalidBoardSizeException();
	if (! Tree::is_renlib_file(file_path)) return false;
	
	ifstream ifs(file_path, ios::binary | ios::in);
	if (! ifs.is_open()) return false;
	ifs.ignore(Renlib_Header_Size);
	
	// make sure the loader begins with an empty tree
	this->cur_goto_root();
	this->delete_current_pos();
	this->cur = this->root = new Node;
	
	// read the nodes in preorder to reconstruct the tree
	RenlibNode rnode; stack<Node*> node_stack; bool root = true;
	Node* pcur = this->root; Node* pnext = NULL;
	while (ifs && (! ifs.eof())) {
		ifs.read((char*) &rnode, 2); //read a node
		
		if (root && (rnode.x != 0 || rnode.y != 0)) {
			// make sure the root of the reconstructed tree has a null position
			this->root->down = new Node;
			pcur = this->root->down;
		}
		root = false;
		
		pnext = new Node; //in which the `pos` is initialized as a null position
		if (pnext == NULL) return false; //Out of memory
		
		if (rnode.x != 0 || rnode.y != 0) {
			// convert x and y into Figrid::Move format, in which (0, 0) represents a1.
			pcur->pos.x = rnode.x - 1;
			pcur->pos.y = 15 - rnode.y - 1;
		}
		pcur->marked = rnode.mark;
		pcur->marked_start = rnode.start;

		if (rnode.comment || rnode.old_comment) {
			pcur->has_comment = true;
			string str = ""; char ch;
			while (ifs && (! ifs.eof())) {
				ifs.read(&ch, 1);  if (ch == '\0') break;
				str += ch;
			}
			string_manage_multiline(str); //replace '\b' and "\r\n" with '\n'
			this->comments.push_back(str); //to C++ string
			pcur->tag_comment = this->comments.size() - 1;
			
			// goto the byte right of the last '\0'
			while (ifs && (! ifs.eof())) {
				ifs.read(&ch, 1);
				if (ch != '\0') {ifs.putback(ch); break;}
			}
		}
		
		if (! rnode.is_leaf) {
			// the top of the stack will point to the parent node of the next node, which is its left descendent
			if (rnode.has_sibling)
				node_stack.push(pcur);
			pcur->down = pnext;
		} else if (rnode.has_sibling) {
			pcur->right = pnext; //the next node will be its right sibling
		} else if (! node_stack.empty()) { //the next node will be the parent's right sibling
			node_stack.top()->right = pnext; node_stack.pop();
		} else
			break; //the program has gone through the entire tree
		
		pcur = pnext; //this will not happen if current node is the last node, see the 'break;' above
	}
	
	bool comp = (pcur != pnext); //this should be true if the file is not broken
	if (comp) delete pnext;
	this->cur_goto_root();
	return comp;
}

bool Tree::save_renlib(const string& file_path) const
{
	if (this->board_size != 15) throw InvalidBoardSizeException();
	if (this->root == NULL || this->root->down == NULL) return false;

	if (ifstream(file_path, ios::binary | ios::in)) { //file exists
		string bak_path = file_path + ".bak";
		if (ifstream(bak_path, ios::binary | ios::in)) //.bak file exists
			if (remove(bak_path.c_str()) != 0) return false;
		if (rename(file_path.c_str(), bak_path.c_str()) != 0) return false;
	}
	
	ofstream ofs;
	ofs.open(file_path, ios::binary | ios::out);
	if (! ofs.is_open()) return false;
	ofs.write((const char*)Renlib_Header, Renlib_Header_Size);
	
	Node* pcur = this->root;
	stack<Node*> node_stack;  RenlibNode rnode;
	rnode.old_comment = rnode.no_move = rnode.extension = false; //reserved?
	
	while (true) {
		if (! pcur->pos.pos_is_null()) {
			rnode.x = pcur->pos.x + 1;
			rnode.y = 15 - (pcur->pos.y + 1);
		} else rnode.x = rnode.y = 0;
		
		rnode.comment = pcur->has_comment;
		rnode.mark = pcur->marked;
		rnode.start = pcur->marked_start;
		rnode.has_sibling = (pcur->right != NULL);
		rnode.is_leaf = (pcur->down == NULL);
		
		ofs.write((char*) &rnode, 2);
		
		if (pcur->has_comment) {
			string str = this->comments[pcur->tag_comment];
			string_manage_multiline(str, true); //replace '\n' with "\r\n"
			ofs.write(str.c_str(), str.length() + 1); // +1 to write '\0'
		}
		
		// go through the nodes in preorder sequence
		if (pcur->down != NULL) {
			if (pcur->right != NULL)
				node_stack.push(pcur->right);
			pcur = pcur->down;
		} else if (pcur->right != NULL) {
			pcur = pcur->right;
		} else if (! node_stack.empty()) {
			pcur = node_stack.top(); node_stack.pop();
		} else
			break;
	}
	
	return true;
}
