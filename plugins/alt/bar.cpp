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

#include "../TPluginParentLight.h"
#include "../mytypes.h"

extern "C" int init()
{
	return 0; //wir brauchen keinen kontext!
}

extern "C" void bar (plugincontext* context, ircmessage msg, TPluginParentLight* parent)
{
	cout << "context is "<<context<<endl;
	cout << "message is: "<<msg.origin<<"|"<<msg.command<<"|"<<msg.params<<"|"<<msg.content<<"|"<<endl;
	parent->pluginsend ("hier bin ich!");
	bool temp;
	temp=parent->ismaster("flo|linux");
	if (temp) parent->pluginsend ("hallo meister");
	
	parent->pluginsend ("und du bist " + parent->get_name() + "..?");
}
