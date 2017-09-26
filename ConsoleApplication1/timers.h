#pragma once

#include <sys/timeb.h>

class Timer
{
public:
	Timer();
	unsigned long long   startTime;
	unsigned long long   stopTime;
	unsigned long long   currentTime;
	unsigned long long   stopTimeDelta;
	timeb startBuffer;
	timeb stopBuffer;
	timeb currentBuffer;
	bool running;

	void init();               // start the timer
	void stop();               // stop the timer
	void reset();              // reset the timer
	void display();                   // display time in seconds with 2 decimals
	void displayhms();         // display time in hh:mm:ss.dd
	unsigned long long getms();               // return time in milliseconds
	unsigned long long getsysms();         // return system time
};