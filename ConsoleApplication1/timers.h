#pragma once

#include <sys/timeb.h>

class Timer {
public:
	Timer();
	
	void start();               // start the timer
	void stop();               // stop the timer
	void reset();              // reset the timer
	void display();                   // display time in seconds with 2 decimals
	void displayhms();         // display time in hh:mm:ss.dd
	unsigned long long getms();               // return time in milliseconds
	unsigned long long getsysms();         // return system time
private:
	unsigned long long  startTime, stopTime, currentTime, stopTimeDelta;
	timeb startBuffer, stopBuffer, currentBuffer;
	bool running;
};

unsigned long long perft(int, int);