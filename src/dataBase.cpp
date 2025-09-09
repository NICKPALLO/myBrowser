#include "dataBase.h"




DB::DB(std::string Host, std::string Port, std::string Dbname, std::string User, std::string Password) : host(Host), port(Port), dbname(Dbname), user(User), password(Password)
{
	m_ptr = std::make_unique<std::mutex>();
	c = new pqxx::connection("host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + password);
	createTables();
}
void DB::createTables()
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	pqxx::work tx{ *c };
	pqxx::result r = tx.exec("create table if not exists Words( "
		"id serial primary key, "
		"word varchar(40) not null)"
	);
	r = tx.exec("create table if not exists Documents( "
		"id serial primary key, "
		"document text unique not null)"
	);
	r = tx.exec("create table if not exists Relevants( "
		"document_id integer not null references Documents(id), "
		"word_id integer not null references Words(id), "
		"relevance integer not null,"
		"constraint pk1 primary key (document_id,word_id))"
	);
	tx.commit();
}

void DB::addDoc(const std::string& url)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	pqxx::work tx{ *c };
	tx.exec("insert into Documents(document) values ('" + tx.esc(url) + "')");
	tx.commit();
}

void DB::addWord(const std::string& word)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	pqxx::work tx{ *c };
	tx.exec("insert into Words(word) values ('" + tx.esc(word) + "')");
	tx.commit();
}

void DB::addRelevance(const std::string& url, const std::string& word, const int relevance)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	pqxx::work tx{ *c };
	tx.exec("insert into relevants values ("
		"(select id from documents where document = '" + tx.esc(url) + "'),"
		"(select id from words where word = '" + tx.esc(word) + "'),"+ std::to_string(relevance) + ");");
	tx.commit();
}

std::vector<std::string> DB::getResults(const std::vector<std::string>& reqWords)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	std::vector<std::string> results;
	results.reserve(10);
	pqxx::work tx{ *c };
	std::string request = "select d.document, sum(r.relevance) from relevants r "
		"join documents d on r.document_id = d.id "
		"join words w on r.word_id = w.id "
		"where";
	for (int i = 0; i < reqWords.size(); ++i)
	{
		if (i != 0)
		{
			request += " or";
		}
		request += " w.word = '" + tx.esc(reqWords[i]) + "'";
	}
	request += " group by d.document having sum(r.relevance) > 0 order by sum desc limit 10;";
	pqxx::result r = tx.exec(request);
	tx.commit();
	for (auto row : r)
	{
		results.push_back(row[0].c_str());
	}
	return results;
}

void DB::deleteAll()
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	pqxx::work tx_1{ *c };
	tx_1.exec("delete from Relevants;");
	tx_1.commit();
	pqxx::work tx_2{ *c };
	tx_2.exec("delete from Words;");
	tx_2.commit();
	pqxx::work tx_3{ *c };
	tx_3.exec("delete from Documents;");
	tx_3.commit();
	pqxx::work tx_4{ *c };
	tx_4.exec("ALTER SEQUENCE Words_id_seq RESTART WITH 1;");
	tx_4.exec("ALTER SEQUENCE Documents_id_seq RESTART WITH 1;");
	tx_4.commit();
}

bool DB::isEmpty()
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	pqxx::work tx{ *c };
	pqxx::result r = tx.exec("select exists (select 1 from words);");
	tx.commit();
	if (r[0][0].as<bool>())
	{
		return false;
	}
	return true;
}

bool DB::findURL(const std::string& url)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	pqxx::work tx{ *c };
	pqxx::result r = tx.exec("select id from Documents "
		"where document = '" + tx.esc(url) + "';");
	tx.commit();
	if (r.empty())
	{
		return false;
	}
	return true;
}

bool DB::findWord(const std::string& word)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	pqxx::work tx{ *c };
	pqxx::result r = tx.exec("select id from Words "
		"where word = '" + tx.esc(word) + "';");
	tx.commit();
	if (r.empty())
	{
		return false;
	}
	return true;
}
