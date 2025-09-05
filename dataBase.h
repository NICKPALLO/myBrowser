#pragma once
#include <pqxx/pqxx>
#include<iostream>
#include <vector>

class DB
{
public:
	DB(std::string Host, std::string Port, std::string Dbname, std::string User, std::string Password);
	void createTables();
	void addDoc(const std::string& url);
	void addWord(const std::string& word);
	//void updateRelevance(const std::string& url, const std::string& word, const int relevance);
	//int getRelevance(const std::string& url);
	bool isEmpty();
	bool findURL(const std::string& url);
	bool findWord(const std::string& word);
	void addRelevance(const std::string& url, const std::string& word, const int relevance);
	std::vector<std::string> getResults(const std::vector<std::string>& reqWords);
	void deleteAll();
private:
	std::string host;
	std::string port;
	std::string dbname;
	std::string user;
	std::string password;
	std::string data;
	pqxx::connection* c;
};