#include "URLParser.h"

URLParser::URLParser(const std::string& url)
{
	if (url.find("https://") != NOTFOUND)
	{
		port = HTTPS_PORT;
	}
	else if (url.find("http://") != NOTFOUND)
	{
		port = HTTP_PORT;
	}
	else
	{
		port = "NOTFOUND";
	}
	if (port != "NOTFOUND")
	{
		int beginHost = 0;
		int endHost = 0;
		if (port == HTTPS_PORT)
		{
			beginHost = url.find("https://");
			beginHost += 8;
		}
		else
		{
			beginHost = url.find("http://");
			beginHost += 7;
		}
		endHost = url.find('/', beginHost);
		if (endHost == NOTFOUND)
		{
			host = url.substr(beginHost, url.size() - 1);
			target = "/";
		}
		else
		{
			host = url.substr(beginHost, endHost - beginHost);
			target = url.substr(endHost, url.size() - 1);
		}
	}

}

std::string URLParser::get_string() const
{
	if (port == HTTP_PORT)
	{
		return "http://" + host + target;
	}
	return "https://" + host + target;
}