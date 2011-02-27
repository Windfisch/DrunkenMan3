#ifndef _TUSERLIST_H_
#define _TUSERLIST_H_
#include <algorithm>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <dlfcn.h>

using namespace std;

#include "mytypes.h"
#include "myfuncs.h"

class TUserList
{
	public:
		TUserList();
		TUserList(string file);
		~TUserList();
		
		void loadfromfile(string file);
		void savetofile();
		void savetofile(string file);
		void removefromlist(string word);
		void addtolist (string word);
		void addtolist (string word,string add);
		void edit (string word, string add);
		string get_info(string word);
		bool isinlist(string word);
		void show_list();
		void clear();
		map<string,string> give_list();
		
	private:
		string filename;
		map<string,string> entry;
};

#endif
