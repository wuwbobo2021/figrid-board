#include "board.h"

int main()
{
	game_board board(15);
	string strinput;
	
	system("clear");
	cout << "Fiffle v0.01 (Incomplete)\n" << ">";
	while (cin >> strinput)
	{
		system("clear");
		board.record_input(strinput);
		board.print(cout);
		cout << ">";
	}
	
	return 0;
}
