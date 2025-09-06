#pragma once
#include<string>

#ifndef NOTFOUND
#define NOTFOUND std::string::npos
#endif
#ifndef HTTP_PORT
#define HTTP_PORT "80"
#endif
#ifndef HTTPS_PORT
#define HTTPS_PORT "443"
#endif


class URLParser
{
public:
	URLParser(const std::string& url);
	std::string get_string() const;
	std::string host;
	std::string port;
	std::string target;
};