#pragma once

#define ENGINE_VERSION "Welcome to bayless's Chess AI CLI (v0.1)" 

//#define WINGLET_DEBUG_PERF
//#define WINGLET_DEBUG_MOVES
#define WINGLET_VERBOSE_EVAL
//#define WINGLET_DEBUG_EVAL

class FEN {
public:
	bool readFen(char *, int);
	static void setupFen(char *, char *, char *, char *, int, int);
};

class CLI: public FEN {
public:
	CLI();
	
	void readCommands();

private:
	bool doCommand(const char *);
	void setup();
	void info();
	void init();
	static const int MAX_CMD_BUFF = 256; // Console command input buffer
	char CMD_BUFF[MAX_CMD_BUFF];
	int CMD_BUFF_COUNT;
};
