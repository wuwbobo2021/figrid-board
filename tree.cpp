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

Tree::Tree(unsigned short board_sz): board_size(board_sz), crec(board_sz) //the recording is initialized here
{
	this->proot = new Node;
	this->seq = new Node*[board_size * board_size];
	this->pos_goto_root();
	
	for (unsigned char i = 0; i < 8; i++)
		this->rotations.push_back(Recording(board_size)); //prepare for function `query()`
}

Tree::~Tree()
{
	this->pos_goto_root();
	this->delete_current_pos();
	
	if (this->seq != NULL) {
		delete[] this->seq;
		this->seq = NULL;
	}
}

const Node* Tree::root_ptr() const
{
	return this->proot;
}

const Node* Tree::current_ptr() const
{
	return this->ppos;
}

unsigned short Tree::current_depth() const
{
	return this->cdepth;
}

unsigned short Tree::current_degree() const
{
	if (this->ppos == NULL || this->ppos->down == NULL) return 0;
	
	unsigned short cnt = 0; Node* p = this->ppos->down;
	while (p != NULL) {
		cnt++;
		p = p->right;
	}
	return cnt;
}

Move Tree::get_current_move(bool rotate_back) const
{
	Move mv;
	if (this->ppos == NULL) return mv; //return Null_Position_Move
	
	mv = this->ppos->pos;
	if (rotate_back)
		mv.rotate(this->board_size, this->tag_rotate, true); //rotate back
	return mv;
}

Recording Tree::get_current_recording(bool rotate_back) const
{
	Recording rec = this->crec;
	if (rotate_back)
		rec.board_rotate(this->tag_rotate, true);
	return rec;
}

void Tree::print_current_board(ostream& ost, bool use_ascii) const
{
	Recording rec = this->get_current_recording(true); //rotate back to meet original query
	
	Move dots[this->current_degree()];
	if (this->ppos != NULL && this->ppos->down != NULL) {
		Node* p = this->ppos->down;
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
	if (this->ppos == NULL) return false;
	
	if (mark_start)
		return this->ppos->marked_start;
	else
		return this->ppos->marked; 
}

void Tree::set_current_mark(bool val, bool mark_start)
{
	if (this->ppos == NULL) return;
	
	if (mark_start)
		this->ppos->marked_start = val;
	else
		this->ppos->marked = val;
}

bool Tree::get_current_comment(string& comment) const
{
	if (this->ppos == NULL) return false;
	if (! this->ppos->has_comment) return false;
	
	comment = this->comments[this->ppos->tag_comment];
	return true;
}

void Tree::set_current_comment(string& comment)
{
	if (this->ppos == NULL) return;
	
	if (comment != "") {
		if (this->ppos->has_comment)
			this->comments[this->ppos->tag_comment] = comment;
		else {
			this->comments.push_back(comment);
			this->ppos->has_comment = true;
			this->ppos->tag_comment = this->comments.size() - 1;
		}
	} else
		this->ppos->has_comment = false;
}
	
bool Tree::pos_move_down()
{
	if (this->ppos == NULL) return false;
	if (this->ppos->down == NULL) return false;
	
	this->ppos = this->ppos->down;
	this->seq[++this->cdepth] = this->ppos;
	this->crec.domove(this->ppos->pos);
	return true;
}

bool Tree::pos_move_up()
{
	if (this->cdepth < 1) return false;
	
	this->ppos = this->seq[--this->cdepth];
	this->crec.undo();
	return true;
}

bool Tree::pos_move_right()
{
	if (this->ppos == NULL) return false;
	if (this->ppos->right == NULL) return false;
	
	this->ppos = this->ppos->right;
	this->seq[this->cdepth] = this->ppos;
	this->crec.undo(); this->crec.domove(this->ppos->pos);
	return true;
}

bool Tree::pos_move_left()
{
	if (this->cdepth < 1) return false;
	
	Node* p = this->seq[this->cdepth - 1]->down; //goto the left sibling
	while (p != NULL && p->right != this->ppos)
		p = p->right;
	
	if (p == NULL || p->right != this->ppos) return false;
	this->ppos = p;
	this->seq[this->cdepth] = this->ppos;
	this->crec.undo(); this->crec.domove(this->ppos->pos);
	return true;
}

void Tree::pos_goto_root()
{
	this->ppos = this->proot;
	this->seq[this->cdepth = 0] = this->proot;
	this->crec.clear();
	this->tag_rotate = Rotate_None;
}

bool Tree::pos_goto_fork()
{
	if (this->ppos == NULL) return false;
	
	while (this->ppos->down != NULL) {
		if (this->ppos->down->right != NULL) return true;
		this->ppos = this->ppos->down;
		this->seq[++this->cdepth] = this->ppos;
		this->crec.domove(this->ppos->pos);
	};
	
	return false;
}

Node* Tree::new_descendent()
{
	if (this->ppos == NULL) return NULL;
	
	if (this->ppos->down == NULL)
		return (this->ppos->down = new Node);
	else {
		Node* p = this->ppos->down;
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
	if (this->ppos == NULL) return;
	bool del_root = (this->ppos == this->proot);
	if (! del_root) {
		Node* pleft = NULL;
		if (this->pos_move_left()) {pleft = this->ppos; this->pos_move_right();}
		
		if (pleft != NULL)
			pleft->right = this->ppos->right;
		else
			this->seq[cdepth - 1]->down = this->ppos->right;
	}
	
	//delete all nodes in postorder sequence
	Node* psubroot = this->ppos; Node* ptmp;
	stack<Node*> node_stack;
	while (true) {
		if (this->ppos->down != NULL) {
			node_stack.push(this->ppos);
			this->ppos = this->ppos->down;
		} else {
			if (this->ppos->has_comment) {
				this->comments[this->ppos->tag_comment].clear();
				this->comments[this->ppos->tag_comment].shrink_to_fit(); //release memory usage
			}
			
			if (this->ppos == psubroot) {delete this->ppos;  break;}
			
			if (this->ppos->right != NULL) { //delete current node and goto right of it
				ptmp = this->ppos->right;
				delete this->ppos;
				this->ppos = ptmp;
			} else { //delete current node and go up
				delete this->ppos;
				if (node_stack.empty()) break;
				this->ppos = node_stack.top(); node_stack.pop();
				this->ppos->down = NULL; //mark
			}
		}
	}
	
	if (! del_root) {
		this->ppos = this->seq[--this->cdepth];
		this->crec.undo();
	} else { //it may happen when the tree object is being destructed, or when a library file is to be opened
		this->comments.clear();
		this->proot = new Node;
		this->pos_goto_root();
	}
}

Node* Tree::query(Move pos)
{
	if (this->ppos == NULL) return NULL;
	if (this->ppos->down == NULL) return NULL;
	
	Node* p = this->ppos->down;
	while (p != NULL) {
		if (p->pos == pos) {
			this->ppos = p;
			this->seq[++this->cdepth] = p;
			this->crec.domove(pos);
			return p;
		} else
			p = p->right;
	}
	
	return NULL;
}

unsigned short Tree::fixed_query(const Recording* record) //private, doesn't involve rotation
{
	if (record->moves_count() < 1) return 0;
	if (this->proot->down == NULL) return 0; //the tree is empty
	
	Position_Rotation tag_back = this->tag_rotate;
	this->pos_goto_root(); this->tag_rotate = tag_back;
	
	const Move* prec = record->recording_ptr();
	Node* pq = this->query(prec[0]);
	
	if (pq != NULL) { //the first move in this recording is a descendent of the root in the tree
		unsigned short mcnt;
		for (mcnt = 1; mcnt < record->moves_count(); mcnt++) {
			//check next move
			if (this->ppos->down == NULL) return mcnt;
			this->pos_move_down();
			bool found = false;
			do {
				if (prec[mcnt] == this->ppos->pos) //prec[mcnt] is the move after <mcnt>th move
					{found = true; break;}
			} while (this->pos_move_right());
			
			if (!found)
				{this->pos_move_up(); return mcnt;}
		} //mcnt++
		return mcnt;
	} else
		return 0;
}

unsigned short Tree::query(const Recording* record) //it might set the rotate tag
{
	if (record->moves_count() < 1) return 0;
	if (this->proot->down == NULL) return 0; //the tree is empty
	
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

void Tree::string_to_lower_case(string& str)
{
	for (unsigned int i = 0; i < str.length(); i++)
		str[i] = tolower(str[i]);
}

void Tree::search(Node_Search* sch, bool rotate)
{
	if (this->ppos == NULL) return;
	
	Move spos = sch->pos; if (rotate) spos.rotate(this->board_size, this->tag_rotate);
	string sstr = sch->str; string_to_lower_case(sstr);
	
	Node* psubroot = this->ppos;
	stack<Node*> node_stack; Recording tmprec(this->board_size);
	
	while (true) {
		bool suc = true;
		if (sch->mode == Node_Search_None)
			if (this->ppos->down != NULL && this->ppos->down->right == NULL) suc = false; //current node has (only) one descendent
		else {
			if (sch->mode & Node_Search_Mark)
				if (! this->ppos->marked) suc = false;
			if (suc)
				if (sch->mode & Node_Search_Start)
					if (! this->ppos->marked_start) suc = false;
			if (suc)
				if (sch->mode & Node_Search_Position)
					if (ppos->pos != spos) suc = false;
			if (suc)
				if (sch->mode & Node_Search_Comment) {
					if (! this->ppos->has_comment)
						suc = false;
					else {
						string strlower = this->comments[this->ppos->tag_comment];
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
		
		if (this->ppos->down != NULL) {
			if (this->ppos != psubroot && this->ppos->right != NULL)
				node_stack.push(this->ppos);
			this->ppos = this->ppos->down;
			tmprec.domove(this->ppos->pos);
		}
		else if (this->ppos->right != NULL) {
			this->ppos = this->ppos->right;
			tmprec.undo(); tmprec.domove(this->ppos->pos);
		}
		else if (! node_stack.empty()) {			
			//recover the recording (go back to node_stack.top())
			Move lmov = node_stack.top()->pos;
			const Move* prec = tmprec.recording_ptr();
			for (unsigned short i = tmprec.moves_count() - 1; i >= 0; i--)
				if (prec[i] == lmov)
					{tmprec.goback(i); break;}
			
			this->ppos = node_stack.top()->right; node_stack.pop();
			tmprec.domove(this->ppos->pos);
		}
		else break;
	}
	
	this->ppos = psubroot; //recover original position
}

void Tree::write_recording(const Recording* record)
{
	if (record->moves_count() == 0) return;
	
	Recording rec = *record;
	this->pos_goto_root(); //involves clearing rotate tag (for rerotation check)
	unsigned short excnt = this->query(&rec); //count of existing moves (begins from root)
	if (excnt == rec.moves_count()) return; //nothing to write
	rec.board_rotate(this->tag_rotate);
	
	const Move* prec = rec.recording_ptr();
	for (unsigned short i = excnt; i < rec.moves_count(); i++) {
		if (! this->crec.domove(prec[i])) {rec.goback(i); this->crec = rec; return;}
		
		this->ppos = this->new_descendent(prec[i]);
		if (this->ppos == NULL) return; //Out of memory
		this->seq[++this->cdepth] = this->ppos;
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

void Tree::string_manage_multiline(string& str, bool back_to_crlf = false) //private
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
	this->pos_goto_root();
	this->delete_current_pos();
	this->ppos = this->proot = new Node;
	
	//read the nodes in preorder to reconstruct the tree
	Renlib_Node rnode; stack<Node*> node_stack; Node* pnextpos; bool root = true;
	while (ifs && (! ifs.eof())) {
		ifs.read((char*) &rnode, 2); //read a node
		
		if (root && (rnode.x != 0 || rnode.y != 0)) {
			//make sure the root of the reconstructed tree has a null position
			this->new_descendent();
			this->pos_move_down();
		}
		root = false;
		
		pnextpos = new Node; //in which the `pos` is initialized as a null position
		if (pnextpos == NULL) return false; //Out of memory
		
		if (rnode.x != 0 || rnode.y != 0) {
			//convert x and y into Namespace_Figrid::Move format, in which (0, 0) represents a1.
			this->ppos->pos.x = rnode.x - 1;
			this->ppos->pos.y = 15 - rnode.y - 1;
		}
		this->ppos->marked = rnode.mark;
		this->ppos->marked_start = rnode.start;
		
		if (! rnode.is_leaf) {
			//the top of the stack will point to the parent node of the next node, which is its left descendent
			if (rnode.has_sibling)
				node_stack.push(this->ppos);
			this->ppos->down = pnextpos;
		} else if (rnode.has_sibling) {
			this->ppos->right = pnextpos; //the next node will be its right sibling
		} else if (! node_stack.empty()) { //the next node will be the parent's right sibling
			node_stack.top()->right = pnextpos; node_stack.pop();
		} else
			break; //the program has gone through the entire tree
		
		if (rnode.comment || rnode.old_comment) {
			this->ppos->has_comment = true;
			string str = ""; char ch;
			while (ifs && (! ifs.eof())) {
				ifs.read(&ch, 1);  if (ch == '\0') break;
				str += ch;
			}
			string_manage_multiline(str); //replace "\r\n" with '\n'
			this->comments.push_back(str); //to C++ string
			this->ppos->tag_comment = this->comments.size() - 1;
			
			//goto the byte right of the last '\0'
			while (ifs && (! ifs.eof())) {
				ifs.read(&ch, 1);
				if (ch != '\0') {ifs.putback(ch); break;}
			}
		}
				
		this->ppos = pnextpos; //this will not happen if current node is the last node, see the 'break;' above
	}
	
	bool comp = (this->ppos != pnextpos); //this should be true if the file is not broken
	if (comp) delete pnextpos;
	this->pos_goto_root();
	return comp;
}

bool Tree::save_renlib(const string& file_path)
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
	
	Recording rec_back = this->crec;
	
	this->ppos = this->proot;
	stack<Node*> node_stack;  Renlib_Node rnode;
	rnode.old_comment = rnode.no_move = rnode.extension = false; //reserved?
	
	while (true) {
		if (! this->ppos->pos.position_null()) {
			rnode.x = this->ppos->pos.x + 1;
			rnode.y = 15 - (this->ppos->pos.y + 1);
		} else rnode.x = rnode.y = 0;
		
		rnode.comment = this->ppos->has_comment;
		rnode.mark = this->ppos->marked;
		rnode.start = this->ppos->marked_start;
		rnode.has_sibling = (this->ppos->right != NULL);
		rnode.is_leaf = (this->ppos->down == NULL);
		
		ofs.write((char*) &rnode, sizeof(rnode));
		
		if (this->ppos->has_comment) {
			string str = this->comments[this->ppos->tag_comment];
			string_manage_multiline(str, true); //replace '\n' with "\r\n"
			ofs.write(str.c_str(), str.length() + 1); // +1 to write '\0'
		}
		
		//go through the nodes in preorder sequence
		if (this->ppos->down != NULL) {
			if (this->ppos->right != NULL)
				node_stack.push(this->ppos->right);
			this->ppos = this->ppos->down;
		} else if (this->ppos->right != NULL) {
			this->ppos = this->ppos->right;
		} else if (! node_stack.empty()) {
			this->ppos = node_stack.top(); node_stack.pop();
		} else
			break;
	}
	
	this->pos_goto_root();
	this->query(&rec_back); //recover
	return true;
}
