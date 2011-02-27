
#include "TUserList.h"

TUserList::TUserList()
{
	clear();
	filename="";
}
TUserList::TUserList(string file)
{
	filename="";
	loadfromfile(file);
}

TUserList::~TUserList()
{
}

map<string,string> TUserList::give_list()
{
	return entry;
}

void TUserList::show_list()
{
	map<string,string>::iterator it;

	for (it=entry.begin(); it!=entry.end(); it++)
		cout << it->first << ": " << it->second << endl;	
}
void TUserList::loadfromfile(string file)
{
	string temp, temp2;
	
	ifstream ifs (file.c_str(), ifstream::in);
	if (ifs.good())
	{
		clear();
		while (getline(ifs,temp) && getline(ifs,temp2))
			entry[temp]=temp2;

		filename=file;
	}
	ifs.close();
}

bool TUserList::isinlist(string word)
{
	for (map<string,string>::iterator it=entry.begin(); it!=entry.end(); it++)
		if (ucase(it->first)==ucase(word))
			return true;
	return false;
}

void TUserList::addtolist(string word, string add)
{
	if (!isinlist(word))
		entry[word]=add;
}

void TUserList::addtolist(string word)
{
	addtolist (word,"");
}

void TUserList::removefromlist(string word)
{
	for (map<string,string>::iterator it=entry.begin(); it!=entry.end(); it++)
		if (ucase(it->first)==ucase(word))
		{
			entry.erase(it);
			break;
		}
}

void TUserList::savetofile()
{
	savetofile(filename);
}

void TUserList::savetofile(string file)
{
	map<string,string>::iterator it;
	
	ofstream ofs (file.c_str());
	if (ofs.good())
	{
		for (it=entry.begin(); it!=entry.end(); it++)
			ofs << it->first << endl << it->second << endl;
		
		ofs.close();
	}
}

void TUserList::clear()
{
	entry.clear();
}

void TUserList::edit(string word, string add)
{
	for (map<string,string>::iterator it=entry.begin(); it!=entry.end(); it++)
		if (ucase(it->first)==ucase(word))
		{
			it->second=add;
			break;
		}
}

string TUserList::get_info(string word)
{
	return entry[word];
}
