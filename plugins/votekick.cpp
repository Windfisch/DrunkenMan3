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

#include <list>
#include <map>

#define TIMEOUT (5*60)
#define N_VOTES 5

struct vote_t
{
	string voter;
	string victim;
	time_t timestamp;
};

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=sizeof(list<vote_t> *); *conndefault=0; *chandefault=PFLAGS_EXEC_ONDEMAND | PFLAGS_EXEC_ONEVENT; *sessdefault=0;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, int reason)
{
	list<vote_t> *liste=* static_cast<list<vote_t>**>(context->data);
	if (liste==NULL)
	{
		liste=new list<vote_t>;
		*static_cast<list<vote_t>**>(context->data)=liste;
	}
	
	if (reason&PFLAGS_EXEC_ONDEMAND)
	{
		string voter, victim;
		voter=msg.origin;
		victim=ntharg(msg.content,2);
		
		list<vote_t>::iterator it;
		for (it=liste->begin(); it!=liste->end(); it++)
		{
			if ((ucase(voter)==ucase(it->voter)) && (ucase(victim)==ucase(it->victim))) //already there? only update timestamp
			{
				cout << "VOTEKICK: only updating timestamp" << endl;
				it->timestamp=time(NULL);
				break;
			}
		}
		if (it==liste->end())
		{
			cout << "VOTEKICK: adding new entry" << endl;
			vote_t tmp;
			tmp.voter=voter;
			tmp.victim=victim;
			tmp.timestamp=time(NULL);
			liste->push_back(tmp);
		}
		
		bool again;  //check timeouts
		do
		{
			cout << "VOTEKICK: checking for invalid entries..." << endl;
			again=false;
			for (it=liste->begin(); it!=liste->end(); it++)
			{
				if (!parent->get_parent()->isinchan(it->voter, parent->get_name()))
				{
					again=true;
					cout << "VOTEKICK: entry from "<<it->voter<<" for "<<it->victim<<" was deleted, because he isn't in that chan" << endl;
					liste->erase(it);
					break;
				}
				if (time(NULL) > it->timestamp+TIMEOUT) //TODO
				{
					again=true;
					cout << "VOTEKICK: entry from "<<it->voter<<" for "<<it->victim<<" timed out." << endl;
					liste->erase(it);
					break;
				}
			}
		} while (again);
		
		map<string,int> cnt; //count the votes per nick
		for (it=liste->begin(); it!=liste->end(); it++)
			cnt[ucase(it->victim)]++;
		
		map<string,int>::iterator it2; //kick them all =)
		for (it2=cnt.begin(); it2!=cnt.end(); it2++)
			if (it2->second>=N_VOTES) //TODO
			{
				parent->get_parent()->send("KICK "+parent->get_name()+" "+it2->first+" :seems they don't like you :P");
				for (it=liste->begin(); it!=liste->end(); it++)
					if (ucase(it->victim)==ucase(it2->first))
						it->timestamp=-TIMEOUT; //TODO
			}
	}
	else if (reason & PFLAGS_EXEC_ONEVENT)
	{
		
	}
}
