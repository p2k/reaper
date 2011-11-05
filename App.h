#ifndef APP_H
#define APP_H

#include "Curl.h"
#include "AppOpenCL.h"
#include "CPUMiner.h"

class App
{
private:
	Curl curl;
	OpenCL opencl;
	CPUMiner cpuminer;

	clock_t workupdate;

	uint getworks;

public:
	void Main(vector<string> args);
	void Parse(string data);
};

#endif
