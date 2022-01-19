// Figrid v0.11
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_TREE_H
#define FIGRID_TREE_H

#include <vector>
#include "recording.h"

using namespace std;

namespace Namespace_Figrid
{

struct Node //the node structure in memory
{
	Move pos;   // board position, it's original x and y are determined by the definition of `Position`
	bool marked = 0;
	bool marked_start = 0;
	bool has_comment = false;
	
	unsigned int tag_comment; //used by class Tree to store the index of it's comment in the vector of comments
	Node* down = NULL;  // left descendent
	Node* right = NULL; // right sibling
};

class Tree
{
	Node* proot; // root of the tree
	Node* ppos;  // current position
	
	vector<string> comments; // since length of strings are undetermined, they are stored seperately
	
	Node** seq; //the route from root to current node
	unsigned short cdepth = 0; //current depth
	Recording crec; //current recording
	
	// 0: positions in the tree don't need rotation; 1: clockwise 90 deg...
	// 3: clockwise 270 deg; 4: horizontal reflect; 5: horizontal reflect, then clockwise 90 deg...
	Position_Rotation tag_rotate = Rotate_None;
	
	unsigned short fixed_query(const Recording* recording);
	void string_manage_multiline(string& str, bool back_to_crlf);

public:
	const unsigned short board_size;
	
	Tree(unsigned short board_sz);
	~Tree();
	
	const Node* root_ptr() const;
	const Node* current_ptr() const;
	
	unsigned short current_depth() const;
	unsigned short current_degree() const;
	Move get_current_move(bool rotate_back = true) const; //`rotate_back`: back to the direction of the original query
	Recording get_current_recording(bool rotate_back = true) const; //get a copy of current recording
	void print_current_board(ostream& ost) const;
	
	bool current_mark(bool mark_start = false) const;
	void set_current_mark(bool val, bool mark_start = false);
	bool get_current_comment(string& comment) const;
	void set_current_comment(string& comment); //set comment for the current node
	
	bool pos_move_down();
	bool pos_move_up();
	bool pos_move_right();
	bool pos_move_left();
	void pos_goto_root(); //it will clear the rotate tag
	bool pos_goto_fork(); //go straight down to the next fork
	
	Node* new_descendent(); //of current node
	Node* new_descendent(Move pos);
	void delete_current_pos(); //and goto root
	
	Node* query(Move pos); //find in descendents of current node, goto the position if found, return NULL if not found
	unsigned short query(const Recording* record); //check from root and goto the last existing move, and rotation is possible
	void write_recording(const Recording* record); //load a recording
	Position_Rotation query_rotate_tag() const;
	void clear_rotate_tag();

	static bool is_renlib_file(const string& file_path);
	bool load_renlib(const string& file_path); //it will destruct the current tree, then load the library
	bool save_renlib(const string& file_path);
};

}
#endif

