/*
 *      foo.cpp
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

using namespace std;

#include "../TPluginParent.h"

//typedef void (*pluginfunc)(void* context, ircmessage msg);
//typedef int (*plugininitfunc)();

struct plugincontext
{
	int flags; 	//when it is executed, etc
	bool exec_now;
	void* data; //pointer to an area in the RAM where the plugin can write to
	void* functions[10];	//array of pointers to various functions:
							//											say: (in case of chans: PRIVMSG chan; in case of nicks: PRIVMSG nick)
							//											send: send plain text to connection
};

struct ircmessage
{
	string origin;
	string command;
	string content;
	string params;
	
	bool operator! () { return ((origin=="")&&(command=="")&&(content=="")&&(params=="")); }
	operator bool ()  { return ((origin!="")||(command!="")||(content!="")||(params!="")); }
};


extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	
	return 0; //wir brauchen keinen kontext!
}

extern "C" void foo (plugincontext* context, ircmessage msg, TPluginParent* parent)
{
	cout << "context is "<<context<<endl;
	cout << "message is: "<<msg.origin<<"|"<<msg.command<<"|"<<msg.params<<"|"<<msg.content<<"|"<<endl;
	parent->pluginsend ("hier bin ich!");
}
