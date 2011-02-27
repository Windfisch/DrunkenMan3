/* TODO
 * timeout per config
 * anzahl der nötigen votes per config
 * anzahl evtl relativ? in % der aktiven user? oder in % der gesamtuser?
 */

/*
 *      say.cpp
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
#include "../TUserList.h"
#include "../mytypes.h"
#include "../myfuncs.h"

#include <map>

#define TIMEOUT 4

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=sizeof(map<string, time_t> *); *conndefault=0; *chandefault=PFLAGS_EXEC_ONEVENT; *sessdefault=0;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, TConfigReadOnly& config, int reason)
{
	map<string, time_t> *liste=*static_cast<map<string, time_t>**>(context->data);
	if (liste==NULL)
	{
		liste=new map<string, time_t>;
		*static_cast<map<string, time_t>**>(context->data)=liste;
	}
	
	if (reason&PFLAGS_EXEC_ONEVENT)
	{
		if (ucase(msg.command)=="KICK")
		{
			(*liste) [lcase(ntharg(msg.params,2))]=time(NULL);
		}
		if (ucase(msg.command)=="JOIN")
		{
			if (liste->find(lcase(msg.origin))!=liste->end())
				if (time(NULL) < (*liste)[lcase(msg.origin)] +TIMEOUT )
				{
					parent->get_parent()->send("mode "+parent->get_name()+" +b "+msg.origin);
					parent->get_parent()->send("kick "+parent->get_name()+" "+msg.origin+" :do not autorejoin");
				}
		}
		
		map<string, time_t>::iterator it, it2;
		for (it=liste->begin(); it!=liste->end();)
		{
			if (time(NULL) > it->second +TIMEOUT)
			{
				cout << "erasing entry for " << it->first << "..." << endl;
				it2=it;
				it++;
				liste->erase(it2);
			}
			else
				it++;
		}
	}
}
