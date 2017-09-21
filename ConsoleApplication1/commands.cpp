#include <iostream>
#include "defines.h"
#include "protos.h"
#include "extglobals.h"

void readCommands()
{
	int nextc;

	{
		std::cout << "wt> ";
	}

	std::cout.flush();

	//     ===========================================================================
	//     Read a command and call doCommand:
	//     ===========================================================================
	while ((nextc = getc(stdin)) != EOF)
	{
		if (nextc == '\n')
		{
			CMD_BUFF[CMD_BUFF_COUNT] = '\0';
			while (CMD_BUFF_COUNT)
			{
				if (!doCommand(CMD_BUFF)) return;
			}

			{
				std::cout << "wt> ";
			}

			std::cout.flush();
		}
		else
		{
			if (CMD_BUFF_COUNT >= MAX_CMD_BUFF - 1)
			{
				std::cout << "Warning: command buffer full !! " << std::endl;
				CMD_BUFF_COUNT = 0;
			}
			CMD_BUFF[CMD_BUFF_COUNT++] = nextc;
		}
	}
}

bool doCommand(const char *buf)
{

	//     =================================================================
	//  return when command buffer is empty
	//     =================================================================

	if (!strcmp(buf, ""))
	{
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  help, h, or ?: show this help
	//     =================================================================
	if ((!strcmp(buf, "help")) || (!strcmp(buf, "h")) || (!strcmp(buf, "?")))
	{
		std::cout << std::endl << "help:" << std::endl;
		std::cout << "black               : BLACK to move" << std::endl;
		std::cout << "cc                  : play computer-to-computer " << std::endl;
		std::cout << "d                   : display board " << std::endl;
		std::cout << "exit                : exit program " << std::endl;
		std::cout << "eval                : show static evaluation of this position" << std::endl;
		std::cout << "game                : show game moves " << std::endl;
		std::cout << "go                  : computer next move " << std::endl;
		std::cout << "help, h, or ?       : show this help " << std::endl;
		std::cout << "info                : display variables (for testing purposes)" << std::endl;
		std::cout << "move e2e4, or h7h8q : enter a move (use this format)" << std::endl;
		std::cout << "moves               : show all legal moves" << std::endl;
		std::cout << "new                 : start new game" << std::endl;
		std::cout << "perf                : benchmark a number of key functions" << std::endl;
		std::cout << "perft n             : calculate raw number of nodes from here, depth n " << std::endl;
		std::cout << "quit                : exit program " << std::endl;
		std::cout << "r                   : rotate board " << std::endl;
		std::cout << "readfen filename n  : reads #-th FEN position from filename" << std::endl;
		std::cout << "sd n                : set the search depth to n" << std::endl;
		std::cout << "setup               : setup board... " << std::endl;
		std::cout << "undo                : take back last move" << std::endl;
		std::cout << "white               : WHITE to move" << std::endl;
		std::cout << std::endl;
		CMD_BUFF_COUNT = '\0';
		return true;
	}

	//     =================================================================
	//  exit or quit: exit program
	//     =================================================================
	if ((!strcmp(buf, "exit")) || (!strcmp(buf, "quit")))
	{
		CMD_BUFF_COUNT = '\0';
		return false;
	}

	//     =================================================================
	//  unknown command
	//     =================================================================
	std::cout << "    command not implemented: " << buf << ", type 'help' for more info" << std::endl;
	CMD_BUFF_COUNT = '\0';
	return true;
}