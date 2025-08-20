
#include "MyBrowser.h"
#include "crowler.h"
#include "dataBase.h"
#include <vector>
#include <iostream>


int main()
{
    Crowler crowler;

    std::vector<std::string> request;
    request.push_back("music");

    crowler.startWork(request);

	return 0;
}