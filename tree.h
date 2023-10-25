// Figrid v0.20
// a recording software for the Five-in-a-Row game compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#ifndef FIGRID_TREE_H
#define FIGRID_TREE_H

#include <vector>
#include "recording.h"

using namespace std;

namespace Figrid
{

struct Node //the node structure in memory
{
	Move pos;   //board position, its original x and y are determined by the definition of `Position`
	bool marked = 0;
	bool marked_start = 0;
	bool has_comment = false;
	
	unsigned int tag_comment; //used by class Tree to store the index of its comment in the vector of comments
	Node* down = NULL;  //left descendent
	Node* right = NULL; //right sibling
};

enum Node_Search_Mode
{
	Node_Search_Leaf = 0,     //0000
	Node_Search_Mark = 1,     //0001
	Node_Search_Start = 2,    //0010
	Node_Search_Position = 4, //0100
	Node_Search_Comment = 8   //1000
};

struct NodeSearch
{
	Node_Search_Mode mode = Node_Search_Leaf;
	Move pos;
	string str;
	bool keep_cur_rec_in_result = false;

	vector<Recording>* result;
};

class Tree //Note: not designed for multi-thread processing
{
	Node* root; //root of the tree (which has a null move)
	Node* cur;  //current position
	
	vector<string> comments; //stored seperately
	
	Node** seq; //the route from root to current node. seq[0] is root
	unsigned short cur_depth = 0; //current depth
	Recording rec; //current recording. rec.count() equals cur_depth
	
	// 0: the accepted query doesn't need rotation; 1: clockwise 90 deg...
	// 3: clockwise 270 deg; 4: horizontal reflect; 5: horizontal reflect, then clockwise 90 deg...
	Position_Rotation flag_rotate = Rotate_None;
	vector<Recording> rotations; //temporarily used by function `query()` which perform rotations
	
	unsigned short fixed_query(const Recording* recording); //without rotation
	bool merge_sub_tree(Node* src_root, Position_Rotation prerotation = Rotate_None);

public:
	const unsigned short board_size;
	
	Tree(unsigned short board_sz);
	Tree(const Tree&) = delete;
	Tree& operator=(const Tree&) = delete;
	~Tree();

	unsigned short current_depth() const;
	unsigned short current_degree() const;
	Move current_move(bool rotate_back = true) const; //`rotate_back`: back to the direction of the original query
	Recording get_current_recording(bool rotate_back = true) const; //get a copy of current recording
	void print_current_board(ostream& ost, bool use_ascii = false) const;
	
	bool current_mark(bool mark_start = false) const;
	void set_current_mark(bool val, bool mark_start = false);
	bool get_current_comment(string& comment) const;
	void set_current_comment(string& comment); //set comment for the current node
	
	bool cur_move_down();
	bool cur_move_up();
	bool cur_move_right();
	bool cur_move_left();
	void cur_goto_root(); //clears the rotate flag
	bool cur_goto_fork(); //go straight down to the next fork
	
	bool query_move(Move pos); //find in descendents of current node (without rotation), goto the position if found
	unsigned short query_recording(const Recording* record); //check from root and goto the last existing move, involves rotation
	Position_Rotation query_rotate_flag() const;
	void clear_rotate_flag();
	void search(NodeSearch* sch, bool rotate = true) const; //search in descendents of current node

	bool write_move(Move pos); //without rotation
	bool node_move_left(Move pos);
	bool node_move_right(Move pos);
	bool write_recording(const Recording* record, bool disable_rotation = false);
	bool merge_rotations();
	void help_standardize();

	void delete_current_pos();
	
	const Node* root_ptr() const;
	const Node* current_ptr() const;
	
	// functions for Renlib
	static bool is_renlib_file(const string& file_path);
	bool load_renlib(const string& file_path); //destructs the current tree, then load the library
	bool save_renlib(const string& file_path) const;
};

}
#endif
