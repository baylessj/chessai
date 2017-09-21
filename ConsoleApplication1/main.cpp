// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "defines.h"
#include "protos.h"
#include "globals.h"

#include <iostream>


int main(int argc, char* argv[])
{
	std::cout << WINGLET_PROG_VERSION << std::endl;
	dataInit();
	readCommands();
    return 0;
}

