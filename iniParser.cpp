#include "iniParser.h"

ini_parser::ini_parser(std::string filename)
{
	fin.open(filename);
	if (!fin.is_open())
	{
		throw std::exception("file does not exist");
	}
}

ini_parser::~ini_parser()
{
	fin.close();
}

template<class T>
T ini_parser::get_value(std::string section, std::string valueName)
{
	fin.seekg(0);
	if (!find_Section(section))
	{
		throw std::exception("Section is not exist!");
	}
	return find_Value<T>(valueName);
}

bool ini_parser::find_Section(std::string& section)
{
	std::string buff = "";
	do
	{
		if (fin.eof())
		{
			return false;
		}
		fin >> buff;
	} while (buff != "[" + section + "]");
	return true;
}

template<class T>
T ini_parser::find_Value(std::string& value)
{
	std::string buff = "";
	char nextSymb = ' ';
	while (nextSymb != '[' && !fin.eof()) //пока не дошли до следующей секции и файл не закончился
	{
		fin.get(nextSymb);
		buff += nextSymb;
		if (buff == value + "=")
		{
			break;
		}
		else if (nextSymb == '\n')
		{
			buff = "";
		}
		else if (nextSymb == ';')
		{
			next_line();
			buff = "";
		}
	}
	if (fin.eof())
	{
		throw std::exception("Value is not exist!");
	}
	else if (nextSymb == '[')
	{
		fin.seekg(-1, std::ios_base::cur);
		throw std::exception("Value is not exist!");
	}
	return read_Value<T>();
}

void ini_parser::next_line()
{
	char nextSymb = ' ';
	while (nextSymb != '\n' && !fin.eof())
	{
		fin.get(nextSymb);
	}
}

template <class T>
T ini_parser::read_Value()
{
	T answer;
	char nextSymb = ' ';
	std::string buff = "";
	while (true)
	{
		fin.get(nextSymb);
		if (nextSymb == ';' || nextSymb == '\n' || nextSymb == ' ')
		{
			break;
		}
		buff += nextSymb;
	}
	if (buff != "")
	{
		if (typeid(T) == typeid(int))
		{
			answer = std::stoi(buff);
		}
		else if (typeid(T) == typeid(double))
		{
			answer = std::stod(buff);
		}
	}
	else
	{
		throw std::exception("Value is not exist!");
	}
	return answer;
}

template <>
std::string ini_parser::read_Value()
{
	char nextSymb = ' ';
	std::string buff = "";
	while (true)
	{
		fin.get(nextSymb);
		if (nextSymb == ';' || nextSymb == '\n')
		{
			break;
		}
		buff += nextSymb;
	}
	if (buff == "")
	{
		throw std::exception("Value is not exist!");
	}
	return buff;
}


