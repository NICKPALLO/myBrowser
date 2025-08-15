#include "dataBase.h"




DB::DB(std::string Host, std::string Port, std::string Dbname, std::string User, std::string Password) : host(Host), port(Port), dbname(Dbname), user(User), password(Password)
{
	c = new pqxx::connection("host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + password);
	createTables();
}
void DB::createTables()
{
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
	pqxx::work tx{ *c };
	//tx.exec("SET CLIENT_ENCODING TO 'WIN1251';");
	tx.exec("insert into Documents(document) values ('" + tx.esc(url) + "')");
	tx.commit();
}

void DB::addWord(const std::string& word)
{
	pqxx::work tx{ *c };
	//tx.exec("SET CLIENT_ENCODING TO 'WIN1251';");
	tx.exec("insert into Words(word) values ('" + tx.esc(word) + "')");
	tx.commit();
}

void DB::addRelevance(const std::string& url, const std::string& word, const int relevance)
{
	pqxx::work tx{ *c };
	tx.exec("insert into relevants values ("
		"(select id from documents where document = '" + tx.esc(url) + "'),"
		"(select id from words where word = '" + tx.esc(word) + "'),"+ std::to_string(relevance) + ");");
	tx.commit();

}

int DB::getRelevance(const std::string& url)
{
	pqxx::work tx{ *c };
	pqxx::result r = tx.exec("select SUM(relevance) from relevants r "
		"where r.document_id = (select id from documents where document = '"+ tx.esc(url) +"');");
	tx.commit();
	return r.at(0, 0).as<int>();
}

void DB::deleteAll()
{
	pqxx::work tx{ *c };
	tx.exec("delete from Relevants;");
	tx.exec("delete from Words;");
	tx.exec("delete from Documents;");
	tx.exec("ALTER SEQUENCE Words_id_seq RESTART WITH 1;");
	tx.exec("ALTER SEQUENCE Documents_id_seq RESTART WITH 1;");
	tx.commit();
}

bool DB::FindURL(const std::string& url)
{
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
