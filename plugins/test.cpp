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

#include "../TConnectionInterface.h"
#include "../TPluginParentLight.h"
#include "../mytypes.h"
#include "../myfuncs.h"
#include "../TConfigReadOnly.h"

int x;

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=0; *conndefault=PFLAGS_EXEC_ONANYEVENT; *chandefault=0; *sessdefault=0;//return 0; //wir brauchen keinen kontext!
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, TConfigReadOnly& config, int reason)
{
	cout << config.is_integer("UID") << endl;
	cout << config.get_string("UID") << endl;
}
