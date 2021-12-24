// By wuwbobo2021 <https://www.github.com/wuwbobo2021>, <wuwbobo@outlook.com>.
//     If you have found bugs in this program, or you have any suggestion (especially
// suggestions about adding comments), please pull an issue, or contact me. 
// Released Under GPL 3.0 License.

// Note: If you want to build this program on Windows, check `platform_specific.cpp`.

#ifndef PLATFORM_SPECIFIC_H
#define PLATFORM_SPECIFIC_H

#ifdef _WIN32
	#include "windows.h"
#else
	#include "unistd.h"
#endif

using namespace std;

int file_size(const string& path);

extern void terminal_color_change(); //change to white background and black foreground
extern void terminal_color_change_back();
extern void terminal_clear();
extern void terminal_pause();

#endif
