#ifndef _MYFUNCS_H_
#define _MYFUNCS_H_

#include <string>
#include <list>

using namespace std;

string ucase (string str);
string lcase (string str);
string trim (string str);
string rtrim (string str);
string ltrim (string str);
string split (string& str);
string ntharg(string str, int n);
bool match(string what, string key);

string IntToString(int i);

string get_hostmask(string nick1);
string cut_nick (string nick1);

struct ircmode_t
{
  string mode;
  string param;
};

list<ircmode_t> parsemodes (string params);
int numchanperm (string m);

#endif
