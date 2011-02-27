/*
 *      TConfig.cpp
 *      
 *      Copyright 2009 Florian <flo@localhost.localdomain>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "TConfig.h"
#include "myfuncs.h"

using namespace std;

TConfig::TConfig()
{
	
}

TConfig::TConfig(string f)
{
	if (!loadconfig(f)) throw;
}

bool TConfig::loadconfig(string f)
{
	int line_no=0;
	bool fail;
	int pos;
	string varname;
	string value;
	bool add;
	
	errorlines.clear();
	
	ifstream ifs;
	ifs.open (f.c_str());
	if (ifs.good())
	{
		string line;
		while (getline(ifs,line))
		{
			line_no++;
			
			if ((line=trim(removecomments(line)))!="")
			{
//				cout << "line #"<<line_no<<": ." << line << "." << endl;
				fail=false;
				if ((pos=findunquoted(line,"+=")) != string::npos)
				{
					varname=line.substr(0,pos);
					value=line.substr(pos+2);
					add=true;
				}
				else if ((pos=findunquoted(line,"=")) != string::npos)
				{
					varname=line.substr(0,pos);
					value=line.substr(pos+1);
					add=false;
				}
				else
				{
					fail=true;
				}
				
				if (!fail)
				{
					varname=lcase(trim(varname));
					value=trim(value);
					
					if ((varname!="") && (value!=""))
					{
						cout << "varname='"<<varname<<"', value='"<<value<<"'" << endl;
//						if (add) cout << "add is true" << endl; else cout << "add is false" << endl;
						
						if (is_correct(value))
						{
							value=remove_quotes(value);
							int where;
							where=findentry(varname);
							if (where==-1)	//ggf. anlegen?
							{
//								cout << "anlegen..." << endl;
								conf_ent tmp;
								tmp.name=varname;
								tmp.value="";
								tmp.is_string=false;
								entries.push_back(tmp);
								where=entries.size()-1;
							}
							
							if (!add) //=
							{
								entries[where].value=value;
								
								entries[where].is_string=false;
								for (int i=0; i<value.length(); i++)
									if (!isdigit(value[i]))
									{
										entries[where].is_string=true;
										break;
									}
							}
							else //+= : es werden NUR string-additionen durchgeführt! 2+2 ist nicht 4, sondern 22!
							{
								entries[where].value+=" "+value;
								entries[where].is_string=true;
							}
//							cout << entries[where].name << " == " << entries[where].value << "; (is_string="<<entries[where].is_string<<")" << endl;
						}
						else
						{
							fail=true;
						}
					}
					else
					{
						fail=true;
					}
				}
				
				if (fail)
				{
					//cout << "syntax error in line "<<line_no<<"in configuration file '"<<f<<"'" << endl;
					errorlines.push_back(line_no);
				}
			}
		}
		if (errorlines.empty()) return true; else return false;
	}
	else 	//could not open file o_O?
	{
		throw;
		return false;
	}
}

int TConfig::get_integer(string name)
{
	int pos=findentry(name);
	if (pos!=-1)
		if (entries[pos].is_string==false)
			return atoi(entries[pos].value.c_str());
		else
			return 0;
	else
		return 0;
}
string TConfig::get_string(string name)
{
	int pos=findentry(name);
	if (pos!=-1)
		return entries[pos].value;
	else
		return "";
}
bool TConfig::get_boolean(string name)
{
	int pos=findentry(name);
	if (pos!=-1)
	{
		string tmp=lcase(trim(entries[pos].value));
		return ((tmp=="yes") || (tmp=="on") || (tmp=="1") || (tmp=="true") || (tmp=="enabled"));
	}
	else
	{
		return false;
	}
}

bool TConfig::is_stored(string name)
{
	return (findentry(name)!=-1);
}
bool TConfig::is_string(string name)
{
	int pos=findentry(name);
	if (pos!=-1)
		return entries[pos].is_string;
	else
		return false;
}
bool TConfig::is_integer(string name)
{
	int pos=findentry(name);
	if (pos!=-1)
		return !entries[pos].is_string;
	else
		return false;
}
bool TConfig::is_boolean(string name)
{
	int pos=findentry(name);
	if (pos!=-1)
	{
		string tmp=trim(lcase(entries[pos].value));
		return ((tmp=="yes") || (tmp=="no") || (tmp=="1") || (tmp=="0") || (tmp=="on") || (tmp=="off") || (tmp=="enabled") || (tmp=="disabled") || (tmp=="true") || (tmp=="false"));
	}
	else
	{
		return false;
	}
}
		

int TConfig::findentry(string name)
{
	name=lcase(name);
	for (int i=0;i<entries.size();i++)
	{
		if (entries[i].name==name)
			return i;
	}
	return -1;
}

string TConfig::removecomments(string s)
{
	bool quoted=false;
	int i;
	
	for (i=0;i<s.length();i++)
	{
//		if ((!quoted) && (s[i]=='#')) break; //ALT
		if ((!quoted) && (s[i]=='#') && ((i==0) || (s[i-1]==' '))) break;
		if (s[i]=='"') quoted=!quoted;
	}
	
	return s.substr(0,i);
}

int TConfig::findunquoted(string haystack, string needle)
{
	bool quoted=false;
	
	for (int i=0;i<=haystack.length()-needle.length(); i++)
	{
		if (haystack[i]=='"') quoted=!quoted;
		if (!quoted)
			if (haystack.substr(i,needle.length())==needle)
				return i;
	}
	return string::npos;
}

bool TConfig::is_correct(string s)
{
	if ((s[0]=='"')^(s[s.length()-1]=='"')) return false;
	for (int i=1; i<s.length()-1; i++)
		if (s[i]=='"') return false;
	
	return true;
}

string TConfig::remove_quotes(string str)
{
	for (string::iterator it = str.begin(); it != str.end(); )
		if (*it=='"') 
			it = str.erase(it);
		else
			++it;
		
	//str.erase(string::remove(str.begin(), str.end(), 'a'), str.end());
	return str;
}

vector<int> TConfig::get_errors()
{
	return errorlines;
}

int TConfig::get_valid_integer(string name, int def)
{
	string path, leaf;
	if (name.rfind('.')==string::npos)
	{
		path=""; leaf=name;
	}
	else
	{
		path=name.substr(0,name.rfind('.')+1);
		leaf=name.substr(name.rfind('.')+1);
		
		while (!is_integer(path+leaf))
		{
			if (path.rfind('.',path.length()-2)==string::npos) 
			{
				path="";
				break;
			}
			else
			{
				path=path.substr(0,path.rfind('.',path.length()-2)+1);
			}
		}
	}
	
	if (!is_integer(path+leaf))
		return def;
	else
		return get_integer(path+leaf);
}

string TConfig::get_valid_string(string name, string def)
{
	string path, leaf;
	if (name.rfind('.')==string::npos)
	{
		path=""; leaf=name;
	}
	else
	{
		path=name.substr(0,name.rfind('.')+1);
		leaf=name.substr(name.rfind('.')+1);
		
		while (!is_stored(path+leaf))
		{
			if (path.rfind('.',path.length()-2)==string::npos) 
			{
				path="";
				break;
			}
			else
			{
				path=path.substr(0,path.rfind('.',path.length()-2)+1);
			}
			cout << "path="<<path<<endl;
		}
	}
	
	if (!is_stored(path+leaf))
		return def;
	else
		return get_string(path+leaf);
}

bool TConfig::get_valid_boolean(string name, bool def)
{
		string path, leaf;
	if (name.rfind('.')==string::npos)
	{
		path=""; leaf=name;
	}
	else
	{
		path=name.substr(0,name.rfind('.')+1);
		leaf=name.substr(name.rfind('.')+1);
		
		while (!is_boolean(path+leaf))
		{
			if (path.rfind('.',path.length()-2)==string::npos) 
			{
				path="";
				break;
			}
			else
			{
				path=path.substr(0,path.rfind('.',path.length()-2)+1);
			}
		}
	}
	
	if (!is_boolean(path+leaf))
		return def;
	else
		return get_boolean(path+leaf);
}
