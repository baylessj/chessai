#pragma once

#define ENGINE_VERSION "Welcome to the baylessj Chess AI CLI v0.1" 

class FEN {
public:
	bool readfen();
	bool readFen(char *, int);
	static void setupFen(char *, char *, char *, char *, int, int);
};

class CLI: public FEN {
public:
	static const int MAX_CMD_BUFF = 256; // Console command input buffer
	bool doCommand(const char *);
	void readCommands();
};

