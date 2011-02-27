#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <dlfcn.h>

#include "myfuncs.h"

using namespace std;
string ucase (string str)
{
	string temp=str;
	for (int i=0;i<temp.length();i++) temp[i]=toupper(temp[i]);
	return temp;
}

string lcase (string str)
{
	string temp=str;
	for (int i=0;i<temp.length();i++) temp[i]=tolower(temp[i]);
	return temp;
}

string trim (string str)
{
	string temp=str;
	int i;
	for (i=0;i<temp.length(); i++)
		if (temp[i]!=' ')
			break;
			
	if (i==temp.length()) return "";
	
	temp=temp.substr(i);
	
	for (i=temp.length()-1; i>=0; i--)
		if (temp[i]!=' ')
			break;
	
	temp=temp.substr(0,i+1);
	return temp;		
}

string ltrim (string str)
{
	int i;
	for (i=0;i<str.length(); i++)
		if (str[i]!=' ')
			break;
			
	if (i==str.length())
		return "";
	else
		return str.substr(i);
}

string rtrim (string str)
{
	int i;
	
	for (i=str.length()-1; i>=0; i--)
		if (str[i]!=' ')
			break;
	
	return str.substr(0,i+1);
}



string split (string& in)
{
	in=ltrim(in);
	
	string out;
	
	int i;
	i=in.find(' ');
	
	if (i==string::npos)
	{
		out=in;
		in="";
		return out;
	}
	
	out=in.substr(0,i);  //01234   89;
	for (;i<in.length();i++)
		if (in[i]!=' ') break;
		
	if (i==in.length())
		in="";
	else
		in=in.substr(i);
		
	return out;
}

bool match(string what, string key)
{
	if (what.substr(0,key.length())==key) 
	{
		if (what.length()>key.length())
		{
			if (what.substr(key.length(),1)==" ")
				return true;
		}
		else //keine parameter
		{
			return true;
		}
	}
	return false;
}

string ntharg(string str, int n)
{
	for (int i=1;i<n;i++)
		split(str);
	return split(str);
}

string IntToString(int i)
{
    ostringstream temp;
    temp << i;
    return temp.str();
}

string get_hostmask(string nick1)
{
	if (nick1.find('!',0)==string::npos)
		return "";
	else
		return nick1.substr(nick1.find('!',0)+1);
}

string cut_nick (string nick1)
{
	if (nick1.find('!',0)==string::npos)
		return nick1;
	else
		return nick1.substr(0,nick1.find('!',0));
}


list<ircmode_t> parsemodes (string params)
{
	string modes=ntharg(params,2);
	int param_no=3; //start with second param
	bool plus=true;
	list<ircmode_t> l;
	ircmode_t tmp;
	
	for (int i=0;i<modes.length();i++)
	{
		if (modes[i]=='+')
			plus=true;
		else if (modes[i]=='-')
			plus=false;
		else
		{
			switch (modes[i])
			{
				case 'q':
				case 'a':
				case 'o':
				case 'h':
				case 'v':
				case 'l':
				case 'b':
				case 'k':
					tmp.param=ntharg(params,param_no);
					param_no++;
					break;

				case 'p':
				case 's':
				case 'i':
				case 't':
				case 'n':
				case 'm':
					tmp.param="";
					break;

				default:
					cout << "WARNING: unknown modechar '"<<modes[i]<<"'!" << endl;
					tmp.param="";
					break;
			}

			if (plus)
				tmp.mode="+";
			else
				tmp.mode="-";
			
			tmp.mode+=modes[i];

			l.push_back(tmp);
		}
	}
	
	return l;
}

int numchanperm (string m)
{
	if (m=="")
		return 0;
	
	switch(m[0])
	{
		case 'v': return 1;
		case 'h': return 2;
		case 'o': return 3;
		case 'a': return 4;
		case 'q': return 5;
		default: //WTF?
			return 0;
	}
}
