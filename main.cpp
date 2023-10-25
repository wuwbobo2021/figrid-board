// Figrid v0.20
// a recording software for the Five-in-a-Row game which can run on Linux shell
// and is compatible with Renlib.
// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
// If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please create an issue, or contact me. 
// Released Under GPL-3.0 License.

#include <string>

#include "rule_original.h"
#include "session.h"
#include "tui.h"

using namespace std;

Figrid::RuleOriginal rule = Figrid::RuleOriginal();
Figrid::Session session((unsigned char) 15, (Figrid::Rule*) &rule);
Figrid::FigridTUI tui(&session);

int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			string file_path = (string) argv[i];
			session.load_file(file_path);
			continue;
		}

		switch (argv[i][1]) {
		case 'p': tui.set_pipe_mode(); break;
		case 'x': tui.set_xo_board_mode(); break;
		case 'w': session.set_mode(Figrid::Session_Mode_Library_Write); break;
		default: break;
		}
	}
	
	int r = tui.run();
	
	return r;
}

