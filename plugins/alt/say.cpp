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
#include "../myfuncs.h"

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=0; *conndefault=0; *chandefault=PFLAGS_EXEC_ONDEMAND | PFLAGS_EXEC_ONCREATE; *sessdefault=PFLAGS_EXEC_ONDEMAND | PFLAGS_EXEC_ONCREATE;
}

extern "C" void say (plugincontext* context, ircmessage msg, TPluginParentLight* parent, int reason)
{
	//if (ucase(msg.command)=="PRIVMSG")
	if (reason&PFLAGS_EXEC_ONDEMAND)
		parent->pluginsay (msg.origin+"> "+msg.content+"!");
		
	if (reason&PFLAGS_EXEC_ONCREATE)
		parent->pluginsay ("welcome!");
}
