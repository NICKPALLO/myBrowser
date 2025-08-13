#pragma once
#include <vector>
#include <string>


#ifndef NOTFOUND
#define NOTFOUND std::string::npos
#endif


class Crowler
{
public:
	void linkSearching(const std::string& request);
	void downloading();
	void indexing();

private:
	bool isItLink(const std::string& ref);
};