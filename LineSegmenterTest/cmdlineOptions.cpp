#include "stdafx.h"
#include "cmdlineOptions.h"
#include <dimcli/cli.h>
#include <iostream>


CCmdlineOptions::CCmdlineOptions()
{
	
}


int CCmdlineOptions::Parse(char** argv, int argc)
{
	Dim::Cli cli;
	auto & srcFilename = cli.opt<std::string>("s source").desc("source filename");

	bool b = cli.parse(std::cerr, argc, argv);
	if (!b)
	{
		return cli.exitCode();
	}


	this->sourceFilename = *srcFilename;
	return 0;
}