// Figrid v0.15
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

// Original Renlib code: <https://www.github.com/gomoku/Renlib>, by Frank Arkbo.

#include <cstdint>
#include <locale> //only for `tolower()`
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include "tree.h"

using namespace std;
using namespace Namespace_Figrid;

const int Renlib_Header_Size = 20;	
//                                   0      1     2     3     4     5     6    7
unsigned char Renlib_Header[20] = { 0xFF,  'R',  'e',  'n',  'L',  'i',  'b', 0xFF,
//                                  VER    VER   10    11    12    13    14   15
                                    0x03, 0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//                                  16     17    18    19
                                    0xFF, 0xFF, 0xFF, 0xFF };

struct Renlib_Node //it corresponds with every node in a Renlib file
{
	uint8_t x: 4; // bit 0~3
	uint8_t y: 4; // bit 4~7

	//byte 2, bit 0~7
	bool extension: 1; // reserved? ignored
	bool no_move: 1;   // reserved? ignored
	bool start: 1;
	bool comment: 1;
	bool mark: 1;
	bool old_comment: 1;
	bool is_leaf: 1; //marked with right commas ): it has no left descendent. ("right" in original Renlib code)
	bool has_sibling: 1; //left commas (: it has a right sibling. ("down" in original Renlib code)
	//Reference: Data Structure Techniques by Thomas A. Standish, 3.5.2, Algorithm 3.4
};

Tree::Tree(unsigned short board_sz): board_size(board_sz), rec(board_sz) //the recording is initialized here
{
	this->root = new Node;
	this->seq = new Node*[board_size * board_size];
	this->cur_goto_root();
	
	for (unsigned char i = 0; i < 8; i++)
		this->rotations.push_back(Recording(board_size)); //prepare for function `query()`
}

Tree::~Tree()
{
	this->cur_goto_root();
	this->delete_current_pos();
	
	if (this->seq != NULL) {
		delete[] this->seq;
		this->seq = NULL;
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

unsigned short Tree::current_depth() const
{
	return this->cdepth;
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

Move Tree::get_current_move(bool rotate_back) const
{
	Move mv;
	if (this->cur == NULL) return mv; //return Null_Position_Move
	
	mv = this->cur->pos;
	if (rotate_back)
		mv.rotate(this->board_size, this->tag_rotate, true); //rotate back
	return mv;
}

Recording Tree::get_current_recording(bool rotate_back) const
{
	Recording rec = this->rec;
	if (rotate_back)
		rec.board_rotate(this->tag_rotate, true);
	return rec;
}

void Tree::print_current_board(ostream& ost, bool use_ascii) const
{
	Recording rec = this->get_current_recording(true); //rotate back to meet original query
	
	Move dots[this->current_degree()];
	if (this->cur != NULL && this->cur->down != NULL) {
		Node* p = this->cur->down;
		for (unsigned short i = 0; i < this->current_degree(); i++) {
			dots[i] = p->pos;
			dots[i].rotate(this->board_size, tag_rotate, true); //rotate back
			p = p->right;
		}
		rec.board_print(ost, this->current_degree(), dots, use_ascii);
	} else
		rec.board_print(ost, 0, NULL, use_ascii);
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
	this->seq[++this->cdepth] = this->cur;
	this->rec.domove(this->cur->pos);
	return true;
}

bool Tree::cur_move_up()
{
	if (this->cdepth < 1) return false;
	
	this->cur = this->seq[--this->cdepth];
	this->rec.undo();
	return true;
}

bool Tree::cur_move_right()
{
	if (this->cur == NULL) return false;
	if (this->cur->right == NULL) return false;
	
	this->cur = this->cur->right;
	this->seq[this->cdepth] = this->cur;
	this->rec.undo(); this->rec.domove(this->cur->pos);
	return true;
}

bool Tree::cur_move_left()
{
	if (this->cdepth < 1) return false;
	
	Node* p = this->seq[this->cdepth - 1]->down; //goto the left sibling
	while (p != NULL && p->right != this->cur)
		p = p->right;
	
	if (p == NULL || p->right != this->cur) return false;
	this->cur = p;
	this->seq[this->cdepth] = this->cur;
	this->rec.undo(); this->rec.domove(this->cur->pos);
	return true;
}

void Tree::cur_goto_root()
{
	this->cur = this->root;
	this->seq[this->cdepth = 0] = this->root;
	this->rec.clear();
	this->tag_rotate = Rotate_None;
}

bool Tree::cur_goto_fork()
{
	if (this->cur == NULL) return false;
	
	while (this->cur->down != NULL) {
		if (this->cur->down->right != NULL) return true;
		this->cur = this->cur->down;
		this->seq[++this->cdepth] = this->cur;
		this->rec.domove(this->cur->pos);
	};
	
	return false;
}

Node* Tree::new_descendent()
{
	if (this->cur == NULL) return NULL;
	
	if (this->cur->down == NULL)
		return (this->cur->down = new Node);
	else {
		Node* p = this->cur->down;
		while (p->right != NULL)
			p = p->right;
		return p->right = new Node;
	}
}

Node* Tree::new_descendent(Move pos)
{
	Node* n = this->new_descendent();
	n->pos = pos;
	return n;
}

void Tree::delete_current_pos()
{
	if (this->cur == NULL) return;
	bool del_root = (this->cur == this->root);
	if (! del_root) {
		Node* pleft = NULL;
		if (this->cur_move_left()) {pleft = this->cur; this->cur_move_right();}
		
		if (pleft != NULL)
			pleft->right = this->cur->right;
		else
			this->seq[cdepth - 1]->down = this->cur->right;
	}
	
	//delete all nodes in postorder sequence
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
		this->cur = this->seq[--this->cdepth];
		this->rec.undo();
	} else { //it may happen when the tree object is being destructed, or when a library file is to be opened
		this->comments.clear();
		this->root = new Node;
		this->cur_goto_root();
	}
}

Node* Tree::query(Move pos)
{
	if (this->cur == NULL) return NULL;
	if (this->cur->down == NULL) return NULL;
	
	Node* p = this->cur->down;
	while (p != NULL) {
		if (p->pos == pos) {
			this->cur = p;
			this->seq[++this->cdepth] = p;
			this->rec.domove(pos);
			return p;
		} else
			p = p->right;
	}
	
	return NULL;
}

unsigned short Tree::fixed_query(const Recording* record) //private, doesn't involve rotation
{
	if (record->moves_count() < 1) return 0;
	if (this->root->down == NULL) return 0; //the tree is empty
	
	Position_Rotation tag_back = this->tag_rotate;
	this->cur_goto_root(); this->tag_rotate = tag_back;
	
	const Move* prec = record->recording_ptr();
	Node* pq = this->query(prec[0]);
	
	if (pq != NULL) { //the first move in this recording is a descendent of the root in the tree
		unsigned short mcnt;
		for (mcnt = 1; mcnt < record->moves_count(); mcnt++) {
			//check next move
			if (this->cur->down == NULL) return mcnt;
			this->cur_move_down();
			bool found = false;
			do {
				if (prec[mcnt] == this->cur->pos) //prec[mcnt] is the move after <mcnt>th move
					{found = true; break;}
			} while (this->cur_move_right());
			
			if (!found)
				{this->cur_move_up(); return mcnt;}
		} //mcnt++
		return mcnt;
	} else
		return 0;
}

unsigned short Tree::query(const Recording* record) //it might set the rotate tag
{
	if (record->moves_count() < 1) return 0;
	if (this->root->down == NULL) return 0; //the tree is empty
	
	if (this->tag_rotate != Rotate_None) { //it's already rotated last time, don't do more useless things
		Recording rec = *record;
		rec.board_rotate(this->tag_rotate);
		return this->fixed_query(&rec);
	}
	
	unsigned short mcnt[8] = {0};
	for (unsigned char r = 0; r < 8; r++) {
		this->rotations[r] = *record;
		this->rotations[r].board_rotate((Position_Rotation) r);
		mcnt[r] = this->fixed_query(& rotations[r]);
		if (mcnt[r] == this->rotations[r].moves_count()) { //completely matched
			this->tag_rotate = (Position_Rotation) r;
			return mcnt[r];
		}
	}
	
	unsigned char idx = 0;
	for (unsigned char r = 0; r < 8; r++)
		if (mcnt[r] > mcnt[idx]) idx = r;
	
	if (mcnt[idx] == 0) return 0;
	this->tag_rotate = (Position_Rotation) idx;
	return this->fixed_query(& this->rotations[idx]);
}


Position_Rotation Tree::query_rotate_tag() const
{
	return this->tag_rotate;
}

void Tree::clear_rotate_tag()
{
	this->tag_rotate = Rotate_None;
}

void string_to_lower_case(string& str)
{
	for (unsigned int i = 0; i < str.length(); i++)
		str[i] = tolower(str[i]);
}

void Tree::search(Node_Search* sch, bool rotate) const
{
	if (this->cur == NULL) return;
	
	Move spos = sch->pos; if (rotate) spos.rotate(this->board_size, this->tag_rotate);
	string sstr = sch->str; string_to_lower_case(sstr);
	
	Node* psubroot = this->cur; Node* pcur = psubroot;
	stack<Node*> node_stack; Recording tmprec(this->board_size);
	
	while (true) {
		bool suc = true;
		if (sch->mode == Node_Search_None) {
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
			if (! rotate)
				sch->result->push_back(tmprec);
			else {
				Recording tmprec2 = tmprec; tmprec2.board_rotate(this->tag_rotate, true); //rotate back
				sch->result->push_back(tmprec2);
			}
		}
		
		if (pcur->down != NULL && !(suc && (sch->mode & Node_Search_Position))) {
			if (pcur != psubroot && pcur->right != NULL)
				node_stack.push(pcur);
			pcur = pcur->down;
			tmprec.domove(pcur->pos);
		}
		else if (pcur->right != NULL) {
			pcur = pcur->right;
			tmprec.undo(); tmprec.domove(pcur->pos);
		}
		else if (! node_stack.empty()) {
			//recover the recording (go back to node_stack.top())
			Move lmov = node_stack.top()->pos;
			const Move* prec = tmprec.recording_ptr();
			for (int i = tmprec.moves_count() - 1; i >= 0; i--) {
				if (prec[i] == lmov)
					{tmprec.goback(i); break;}
			}
			
			pcur = node_stack.top()->right; node_stack.pop();
			tmprec.domove(pcur->pos);
		}
		else break;
	}
}

void Tree::write_recording(const Recording* record)
{
	if (record->moves_count() == 0) return;
	
	Recording rec = *record;
	this->cur_goto_root(); //involves clearing rotate tag (for rerotation check)
	unsigned short excnt = this->query(&rec); //count of existing moves (begins from root)
	if (excnt == rec.moves_count()) return; //nothing to write
	rec.board_rotate(this->tag_rotate);
	
	const Move* prec = rec.recording_ptr();
	for (unsigned short i = excnt; i < rec.moves_count(); i++) {
		if (! this->rec.domove(prec[i])) {rec.goback(i); this->rec = rec; return;}
		
		this->cur = this->new_descendent(prec[i]);
		if (this->cur == NULL) return; //Out of memory
		this->seq[++this->cdepth] = this->cur;
	}
}

bool Tree::is_renlib_file(const string& file_path)
{
	char head[Renlib_Header_Size + 2];
	ifstream ifs(file_path, ios_base::binary | ios_base::in);
	if (! ifs.is_open()) return false;
	ifs.read(head, Renlib_Header_Size + 2);
	if (! ifs) return false;
	
	for (unsigned char i = 0; i < 8; i++)
		if ((unsigned char) head[i] != Renlib_Header[i]) return false;
	
	return true;
}

void string_manage_multiline(string& str, bool back_to_crlf = false) //private
{
	//0x08 "\b" is the multiline comment tag at the end of first comment line in Renlib
	size_t pos = str.find(back_to_crlf? "\n" : "\b"); 
	if (pos == string::npos) return;
	str.replace(pos, 1, back_to_crlf? "\b" : "\n");  pos++;
	
	string strfind = back_to_crlf? "\n" : "\r\n"; //problem: don't call this function for double times
	size_t strfind_len = back_to_crlf? 1 : 2;
	string strto = back_to_crlf? "\r\n" : "\n";
	size_t strto_len = back_to_crlf? 2 : 1;
	
	while (true) {
		pos = str.find(strfind, pos);
		if (pos == string::npos) break;
		str.replace(pos, strfind_len, strto);
		pos += strto_len;
	}
}

bool Tree::load_renlib(const string& file_path)
{
	if (this->board_size != 15) throw Invalid_Board_Size_Exception();
	if (! Tree::is_renlib_file(file_path)) return false;
	
	ifstream ifs(file_path, ios_base::binary | ios_base::in);
	if (! ifs.is_open()) return false;
	ifs.ignore(Renlib_Header_Size);
	
	//make sure the loader begins with an empty tree
	this->cur_goto_root();
	this->delete_current_pos();
	this->cur = this->root = new Node;
	
	//read the nodes in preorder to reconstruct the tree
	Renlib_Node rnode; stack<Node*> node_stack; bool root = true;
	Node* pcur = this->root; Node* pnext = NULL;
	while (ifs && (! ifs.eof())) {
		ifs.read((char*) &rnode, 2); //read a node
		
		if (root && (rnode.x != 0 || rnode.y != 0)) {
			//make sure the root of the reconstructed tree has a null position
			this->root->down = new Node;
			pcur = this->root->down;
		}
		root = false;
		
		pnext = new Node; //in which the `pos` is initialized as a null position
		if (pnext == NULL) return false; //Out of memory
		
		if (rnode.x != 0 || rnode.y != 0) {
			//convert x and y into Namespace_Figrid::Move format, in which (0, 0) represents a1.
			pcur->pos.x = rnode.x - 1;
			pcur->pos.y = 15 - rnode.y - 1;
		}
		pcur->marked = rnode.mark;
		pcur->marked_start = rnode.start;
		
		if (! rnode.is_leaf) {
			//the top of the stack will point to the parent node of the next node, which is its left descendent
			if (rnode.has_sibling)
				node_stack.push(pcur);
			pcur->down = pnext;
		} else if (rnode.has_sibling) {
			pcur->right = pnext; //the next node will be its right sibling
		} else if (! node_stack.empty()) { //the next node will be the parent's right sibling
			node_stack.top()->right = pnext; node_stack.pop();
		} else
			break; //the program has gone through the entire tree
		
		if (rnode.comment || rnode.old_comment) {
			pcur->has_comment = true;
			string str = ""; char ch;
			while (ifs && (! ifs.eof())) {
				ifs.read(&ch, 1);  if (ch == '\0') break;
				str += ch;
			}
			string_manage_multiline(str); //replace "\r\n" with '\n'
			this->comments.push_back(str); //to C++ string
			pcur->tag_comment = this->comments.size() - 1;
			
			//goto the byte right of the last '\0'
			while (ifs && (! ifs.eof())) {
				ifs.read(&ch, 1);
				if (ch != '\0') {ifs.putback(ch); break;}
			}
		}
				
		pcur = pnext; //this will not happen if current node is the last node, see the 'break;' above
	}
	
	bool comp = (pcur != pnext); //this should be true if the file is not broken
	if (comp) delete pnext;
	this->cur_goto_root();
	return comp;
}

bool Tree::save_renlib(const string& file_path) const
{
	if (this->board_size != 15) throw Invalid_Board_Size_Exception();
	
	if (ifstream(file_path, ios_base::binary | ios_base::in)) { //file exists
		string bak_path = file_path + ".bak";
		if (ifstream(bak_path, ios_base::binary | ios_base::in))
			remove(bak_path.c_str());
		rename(file_path.c_str(), bak_path.c_str());
	}
	
	ofstream ofs;
	ofs.open(file_path, ios_base::binary | ios_base::out);
	if (! ofs.is_open()) return false;
	ofs.write((const char*)Renlib_Header, Renlib_Header_Size);
	
	Node* pcur = this->root;
	stack<Node*> node_stack;  Renlib_Node rnode;
	rnode.old_comment = rnode.no_move = rnode.extension = false; //reserved?
	
	while (true) {
		if (! pcur->pos.position_null()) {
			rnode.x = pcur->pos.x + 1;
			rnode.y = 15 - (pcur->pos.y + 1);
		} else rnode.x = rnode.y = 0;
		
		rnode.comment = pcur->has_comment;
		rnode.mark = pcur->marked;
		rnode.start = pcur->marked_start;
		rnode.has_sibling = (pcur->right != NULL);
		rnode.is_leaf = (pcur->down == NULL);
		
		ofs.write((char*) &rnode, sizeof(rnode));
		
		if (pcur->has_comment) {
			string str = this->comments[pcur->tag_comment];
			string_manage_multiline(str, true); //replace '\n' with "\r\n"
			ofs.write(str.c_str(), str.length() + 1); // +1 to write '\0'
		}
		
		//go through the nodes in preorder sequence
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
