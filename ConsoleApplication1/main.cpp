// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "cli.h"
#include <iostream>

int main(int argc, char* argv[])
{
	std::cout << ENGINE_VERSION << std::endl;
#ifdef WINGLET_DEBUG_PERFT
	std::cout << "WINGLET_DEBUG_PERFT defined" << std::endl;
#endif
#ifdef WINGLET_DEBUG_MOVES
	std::cout << "WINGLET_DEBUG_MOVES defined" << std::endl;
#endif
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "WINGLET_VERBOSE_EVAL defined" << std::endl;
#endif
#ifdef WINGLET_DEBUG_EVAL
	std::cout << "WINGLET_DEBUG_EVAL defined" << std::endl;
#endif
	CLI cli;
	cli.readCommands();
    return 0;
}

