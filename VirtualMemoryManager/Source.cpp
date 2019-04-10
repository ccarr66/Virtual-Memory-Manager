#include <iostream>
#include <fstream>
#include <Windows.h>

typedef unsigned char byte;
typedef unsigned int uint;

const uint pageFault = 256;
const uint pageSize	= 256;
const uint pageTableSize = 256;
const uint virtualMemorySize = 0xFFFF;
uint pageTable[pageTableSize];


const uint TLBSize = 16;
uint TLB[TLBSize][2];
uint TLBIndex = 0;

const uint frameSize = 256;
const uint numFrames = 256;
bool frameAvailable[numFrames];
byte physicalMemory[numFrames][frameSize];

uint pageFaults, TLBHits, addrLookups = 0;

const char fileName[] = "C:\\Users\\Connor\\source\\repos\\VirtualMemoryManager\\Debug\\BACKING_STORE.bin";

void memoryInitialization();
void virtualMemoryManager(const uint& pageNumber, const uint& offset, uint& physicalAddr, byte& output);
bool movePageIntoMemory(uint& frame);
void TLBaddFIFO(const uint& pageNumber, const uint& frame);
bool TLBLookup(const uint& pageNumber, uint& frame);
void pageNumberOffsetExtractor(const uint& logicalAddr, uint& pageNumber, uint& offset);
void pageNumberOffsetExtractor_TEST();

int main()
{
	memoryInitialization();

	uint pageNumber, offset, physicalAddr;
	byte output;


	for (uint lAddr = 0; lAddr < virtualMemorySize; lAddr++)
	{
		addrLookups++;
		std::cout << "\nLogical Addr: " << lAddr;
		pageNumberOffsetExtractor(lAddr, pageNumber, offset);
		std::cout << "\tPage Number: " << pageNumber << ", Offset: " << offset;
		virtualMemoryManager(pageNumber, offset, physicalAddr, output);
		std::cout << "\tPhysical Addr: " << physicalAddr << ", Output: " << output;
	}

	addrLookups = virtualMemorySize;
	std::cout << "\nPage-faults: " << pageFaults;
	std::cout << "\nPage-fault rate: " << (float)pageFaults / addrLookups * 100 << '%';
	std::cout << "\nTLB hits: " << TLBHits;
	std::cout << "\nTLB hit rate: " << (float)TLBHits / addrLookups * 100 << '%';

	Sleep(4 * 60 * 1000);

	
}

void memoryInitialization()
{
	for (uint i = 0; i < numFrames; i++)
	{
		for (uint j = 0; j < numFrames; j++)
			physicalMemory[i][j] = (byte)0;

		frameAvailable[i] = true;
	}

	for (uint i = 0; i < TLBSize; i++)
		for (uint j = 0; j < 2; j++)
			TLB[i][j] = pageFault;

	for (uint i = 0; i < pageTableSize; i++)
		pageTable[i] = pageFault;

}

void virtualMemoryManager(const uint& pageNumber, const uint& offset, uint& physicalAddr, byte& output)
{
	uint frame;

	if (!TLBLookup(pageNumber, frame))
	{
		if (pageTable[pageNumber] == pageFault)
		{
			pageFaults++;
			movePageIntoMemory(frame);
			pageTable[pageNumber] = frame;
		}
		else
			frame = pageTable[pageNumber];

		TLBaddFIFO(pageNumber, frame);
	}
	else
		TLBHits++;

	physicalAddr = (frame << 8) | offset;
	output = physicalMemory[frame][offset];
}

bool movePageIntoMemory(uint& frame)
{
	frame = 0;
	for (; !frameAvailable[frame]; frame++);
	frameAvailable[frame] = false;

	std::ifstream virtualMemorySpace(fileName);
	if (virtualMemorySpace.is_open())
	{
		virtualMemorySpace.seekg(frameSize*frame);
		virtualMemorySpace.read((char*)physicalMemory[frame], frameSize);
		virtualMemorySpace.close();

		return true;
	}
	else
		return false;

}

void TLBaddFIFO(const uint& pageNumber, const uint& frame)
{
	TLBIndex = (++TLBIndex) % TLBSize;
	TLB[TLBIndex][0] = pageNumber;
	TLB[TLBIndex][1] = frame;
}

bool TLBLookup(const uint& pageNumber, uint& frame)
{
	for (uint i = 0; i < TLBSize; i++)
		if (TLB[i][0] == pageNumber)
		{
			frame = TLB[i][1];
			return true;
		}
	return false;
}

void pageNumberOffsetExtractor(const uint& logicalAddr, uint& pageNumber, uint& offset)
{
	pageNumber = (logicalAddr & 0x0000FF00) >> 8;
	offset = (logicalAddr & 0x000000FF);
}

void pageNumberOffsetExtractor_TEST()
{
	uint pageNumber, offset;
	for (uint i = 0; i <= 10 * 256; i++)
	{
		pageNumberOffsetExtractor(i, pageNumber, offset);
		std::cout << "\n" << i << ":  Page#: " << pageNumber << '\t' << "Offset: " << offset;
	}
	Sleep(3 * 60 * 1000);
}