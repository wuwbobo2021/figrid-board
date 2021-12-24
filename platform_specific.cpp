// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

// Note:
//     this module is for Linux. If you don't know about C++ but want to build the whole program
// on Windows as soon as possible, just delete `#include <sys/stat.h>`, replace the lines in function
// `file_size(string&)` with `return 8 * 1024 * 1024` (8 MB, that will be the memory usage and the maximum
//  size read from the Renlib file), and change console colors manually before starting the program.

#include <string>
#include <iostream>
#include <sys/stat.h>

#include "platform_specific.h"

int file_size(const string& path)
{
	struct stat info;
	stat(path.c_str(), &info);
	return info.st_size;
}

void terminal_color_change()
{
#ifdef _WIN32
	system("color f0");
#else	
	std::cout << "\033[30;47m"; //white background and black foreground
#endif
}

void terminal_color_change_back()
{
#ifdef _WIN32
	system("color");
#else
	std::cout << "\033[0m"; //for linux
#endif
}

void terminal_clear()
{
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
}

void terminal_pause()
{
#ifdef _WIN32
	system("pause");
#else
	std::cout << "Press Enter to continue...";
	//system("read"); //useless
	const unsigned short llmax = 1024; 
	char strin[llmax];
	cin.getline(strin, llmax);
#endif
}

