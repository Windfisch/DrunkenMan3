#ifndef _TCONFIG_H_
#define _TCONFIG_H_
/*
 *      TConfig.h
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
#include <string>
#include <vector>

using namespace std;

#include "TConfigReadOnly.h"

class TConfig : public TConfigReadOnly
{
	public:
		TConfig();
		TConfig(string f);
		bool loadconfig(string f);
		
		virtual int get_integer(string name);
		virtual string get_string(string name);
		virtual bool get_boolean(string name);
		
		virtual int get_valid_integer(string name, int def);
		virtual string get_valid_string(string name, string def);
		virtual bool get_valid_boolean(string name, bool def);
		
		virtual bool is_stored(string name);
		
		virtual bool is_string(string name);
		virtual bool is_integer(string name);
		virtual bool is_boolean(string name);
		
		vector<int> get_errors();
	private:
		struct conf_ent
		{
			string name;
			string value;
			bool is_string;
		};
		vector<conf_ent> entries;
		vector<int> errorlines;
		
		int findentry(string name);
		
		static string remove_quotes(string str);
		static string removecomments(string s);
		static int findunquoted(string haystack, string needle);
		static bool is_correct(string s);
};
#endif
