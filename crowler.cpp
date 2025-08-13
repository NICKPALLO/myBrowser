#include "crowler.h"
#include <iostream>
#include <algorithm>


void Crowler::linkSearching(const std::string& request)
{
	std::vector<std::string> links;
	int beginLink = 0;
	int endLink = 0;
	while (beginLink != NOTFOUND || endLink!= NOTFOUND)
	{
		int beginLink = request.find("href=", endLink);
		if (beginLink == NOTFOUND)
		{
			break;
		}
		beginLink += 6;
		endLink = request.find('"', beginLink);
		if (endLink != NOTFOUND)
		{
			std::string ref = request.substr(beginLink, endLink - beginLink);
			if (isItLink(ref))
			{
				links.push_back(ref);
			}
		}
	}
	//---------------------------------------------------
	//далее вывод
	if (!links.empty())
	{
		for (int i = 0; i < links.size();++i)
		{
			std::cout << i + 1 << ") " << links[i]<<std::endl;
		}
	}
	else
	{
		std::cout << "nothing";
	}
}

void Crowler::downloading()
{

}

void Crowler::indexing()
{

}

bool Crowler::isItLink(const std::string& ref)
{
	if (ref.find("https://") != NOTFOUND || ref.find("http://") != NOTFOUND)
	{
		return true;
	}
	return false;
}
