#ifndef _TCONFIGREADONLY_H_
#define _TCONFIGREADONLY_H_

#include <iostream>
#include <string>

using namespace std;

class TConfigReadOnly
{
	public:
		virtual int get_integer(string name)=0;
		virtual string get_string(string name)=0;
		virtual bool get_boolean(string name)=0;

		virtual int get_valid_integer(string name, int def)=0;
		virtual string get_valid_string(string name, string def)=0;
		virtual bool get_valid_boolean(string name, bool def)=0;
		
		virtual bool is_stored(string name)=0;
		
		virtual bool is_string(string name)=0;
		virtual bool is_integer(string name)=0;
		virtual bool is_boolean(string name)=0;
};
#endif
