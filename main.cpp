// Figrid v0.10 "Innocent Coincidence"
// a recording software for the Five-in-a-Row game which can run on Linux shell
// and is compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

#include <string>

#include "rule_gomoku_original.h"
#include "figrid.h"
#include "tui.h"

using namespace std;
using namespace Namespace_Figrid;

Rule_Gomoku_Original rule = Rule_Gomoku_Original();
Figrid figrid((unsigned char) 15, (Rule*) &rule);
Figrid_TUI tui(&figrid);

int main(int argc, char* argv[])
{
	if (argc > 1) {
		string file_path = (string) argv[1];
		figrid.load_file(file_path);
	}
	
	int r = tui.run();
	
	return r;
}

