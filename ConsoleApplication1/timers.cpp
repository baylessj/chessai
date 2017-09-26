#include <sys/timeb.h>
#include <stdio.h>
#include "timers.h"
#include "board.h"

#pragma warning (disable: 4244)

Timer::Timer()
{
	running = false;
	startTime = 0;
	stopTime = 0;
	stopTimeDelta = 0;
}

void Timer::init()
{
	if (!running)
	{
		running = true;
		ftime(&startBuffer);
		startTime = startBuffer.time * 1000 + startBuffer.millitm + stopTimeDelta;
	}
	return;
}

void Timer::stop()
{
	if (running)
	{
		running = false;
		ftime(&stopBuffer);
		stopTime = stopBuffer.time * 1000 + stopBuffer.millitm;
		stopTimeDelta = startTime - stopTime;
	}
	return;
}

void Timer::reset()
{
	if (running)
	{
		ftime(&startBuffer);
		startTime = startBuffer.time * 1000 + startBuffer.millitm;
	}
	else
	{
		startTime = stopTime;
		stopTimeDelta = 0;
	}
	return;
}

void Timer::display()
{
	if (running)
	{
		ftime(&currentBuffer);
		currentTime = currentBuffer.time * 1000 + currentBuffer.millitm;
		printf("%6.2f", (currentTime - startTime) / 1000.0);
	}
	else
		printf("%6.2f", (stopTime - startTime) / 1000.0);
	return;
}

void Timer::displayhms()
{
	int hh, mm, ss;
	if (running)
	{
		ftime(&currentBuffer);
		currentTime = currentBuffer.time * 1000 + currentBuffer.millitm;
		hh = (currentTime - startTime) / 1000 / 3600;
		mm = ((currentTime - startTime) - hh * 3600000) / 1000 / 60;
		ss = ((currentTime - startTime) - hh * 3600000 - mm * 60000) / 1000;
		printf("%02d:%02d:%02d", hh, mm, ss);
	}
	else
	{
		hh = (stopTime - startTime) / 1000 / 3600;
		mm = ((stopTime - startTime) - hh * 3600000) / 1000 / 60;
		ss = ((stopTime - startTime) - hh * 3600000 - mm * 60000) / 1000;
		printf("%02d:%02d:%02d", hh, mm, ss);
	}
	return;
}

unsigned long long Timer::getms()
{
	if (running)
	{
		ftime(&currentBuffer);
		currentTime = currentBuffer.time * 1000 + currentBuffer.millitm;
		return (currentTime - startTime);
	}
	else
		return (stopTime - startTime);
}

unsigned long long Timer::getsysms()
{
	ftime(&currentBuffer);
	return (currentBuffer.time * 1000 + currentBuffer.millitm);
}

unsigned long long perft(int ply, int depth)
{

	// Raw node count, up to depth, doing a full tree search.
	// perft is very similar to the search algorithm - instead of evaluating the leaves, we count them.
	//
	// Be carefull when calling this function with depths > 7, because it can take a very long
	// time before the result is returned: the average branching factor in chess is 35, so every
	// increment in depth will require 35x more computer time.
	//
	// perft is a good way of verifying correctness of the movegenerator and (un)makeMove,
	// because you can compare the results with published results for certain test positions.
	//
	// perft is also used to measure the performance of the move generator and (un)makeMove in terms
	// of speed, and to compare different implementations of generating, storing and (un)making moves.

	unsigned long long retVal = 0;
	int i;

	// count this node
	if (depth == 0)
	{
		return 1;
	}

	// generate moves from this position
	board.moveBufLen[ply + 1] = movegen(board.moveBufLen[ply]);

	// loop over moves:
	for (i = board.moveBufLen[ply]; i < board.moveBufLen[ply + 1]; i++)
	{
		makeMove(board.moveBuffer[i]);
		{
			if (!isOtherKingAttacked())
			{
				// recursively call perft
				retVal += perft(ply + 1, depth - 1);

#ifdef WINGLET_DEBUG_PERFT
				if (depth == 1)
				{
					if (board.moveBuffer[i].isCapture()) ICAPT++;
					if (board.moveBuffer[i].isEnpassant()) IEP++;
					if (board.moveBuffer[i].isPromotion()) IPROM++;
					if (board.moveBuffer[i].isCastleOO()) ICASTLOO++;
					if (board.moveBuffer[i].isCastleOOO()) ICASTLOOO++;
					if (isOwnKingAttacked()) ICHECK++;
				}
#endif
			}
		}
		unmakeMove(board.moveBuffer[i]);
	}
	return retVal;
}