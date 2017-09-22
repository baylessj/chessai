#pragma once

#define ENGINE_VERSION "Welcome to bayless's Chess AI CLI (v0.1)" 

class FEN {
public:
	//bool readfen();
	bool readFen(char *, int);
	static void setupFen(char *, char *, char *, char *, int, int);
};

class CLI: public FEN {
public:
	CLI();
	bool doCommand(const char *);
	void readCommands();
	//void dataInit();
	void info();
	void setup();
private:
	static const int MAX_CMD_BUFF = 256; // Console command input buffer
	char CMD_BUFF[MAX_CMD_BUFF];
	int CMD_BUFF_COUNT;
};
