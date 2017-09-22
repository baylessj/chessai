// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "cli.h"
#include "globals.h"
#include <iostream>

int main(int argc, char* argv[])
{
	std::cout << ENGINE_VERSION << std::endl;
	cli.dataInit();
	cli.readCommands();
    return 0;
}

