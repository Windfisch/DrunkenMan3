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

using namespace std;

#include <iostream>
#include <string>

using namespace std;

#include "../TConnectionInterface.h"
#include "../TPluginParentLight.h"
#include "../TUserList.h"
#include "../mytypes.h"
#include "../myfuncs.h"

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=sizeof(char)+sizeof(string *)+sizeof(TUserList *)+sizeof(int); *conndefault=0; *chandefault=PFLAGS_EXEC_ONDEMAND; *sessdefault=PFLAGS_EXEC_ONDEMAND;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, int reason)
{
	int* whoiscnt=(int*) ( (TUserList**) ( (string**) ( (char*)  context->data +1) +1) +1);
	if (reason&PFLAGS_EXEC_ONDEMAND)
	{
		if (ucase(msg.command)=="PRIVMSG")
		{
			if (*((char*)(context->data)) != 0)
			{
				parent->say("sorry, try again later.");
			}
			else
			{
				string chan=ntharg(msg.content,2);
				if (trim(chan)=="")
				{
					parent->say("you have to specify a channel name!");
				}
				else
				{
//					parent->say ("/names "+chan+"...");
					parent->get_parent()->send("names "+chan+NEWLINE);
					context->flags|=PFLAGS_EXEC_ONANYEVENT;
					*((char*)(context->data)) = 1;
					string* chanptr=new string(chan);
					
					string** cptrdest;
					cptrdest= (string**) ((char*) context->data  +  1);
					
					*cptrdest=chanptr;
					
					TUserList** lptr= (TUserList**) ((string**)(((char*) context->data+1))+1);
					
					*lptr=new TUserList;
					cout << "lptr=" << lptr <<"/" << *lptr << endl;
					
					
					*whoiscnt=0;
//					parent->say ("exiting commandmode");
					//((string**)((char*)context->data+1))=chanptr;
					//*((string*) ((char*)context->data + 1))=chanptr;
				}
			}
		}
	}	
	
	if (reason&PFLAGS_EXEC_ONANYEVENT)
	{
		TUserList** lptr= (TUserList**) ((string**)(((char*) context->data+1))+1);
		TUserList* liste=*lptr;
		string name;
		name=   **  ((string**)((char*)context->data+1));
		int numcmd=atoi(msg.command.c_str());
		if (numcmd==353)
		{
			if (lcase(msg.params.substr(msg.params.rfind(' ')+1))==lcase(name))
			{
				string temp;
				string temp2=msg.content;
				while ((temp=split(temp2))!="")
				{
					if ((temp[0]=='+') || (temp[0]=='@'))
						temp=temp.substr(1);
					liste->addtolist(temp);
					parent->get_parent()->send("whois "+temp+NEWLINE);
					(*whoiscnt)++;
				}
			}
		}
		if (numcmd==366)
		{
			parent->say ("fertig empfangen...");
			*((char*)(context->data)) = 2;
		}
		
		if (numcmd==311)
		{
//			parent->say ("whoisantwort für "+ntharg(msg.params,2)+": "+ntharg(msg.params,4));
			if (liste->isinlist(ntharg(msg.params,2)))
			{
				liste->edit (ntharg(msg.params,2),ntharg(msg.params,4));
				(*whoiscnt)--;
			}
		}
		
		//cout << "DEBUG: " << *whoiscnt << "," << *((char*)(context->data)) << endl;
		
		if ((*whoiscnt==0)&&(*((char*)(context->data))==2))
		{
			parent->say ("fertig! YAY!");
			*((char*)(context->data))=0;
			
/*			list<string> nicks = liste->give_list();
			list<string> add = liste->give_additional();
			
			list<string>::iterator it1,it2;
			it1=nicks.begin();
			it2=add.begin();*/
			
			
			
		}

		
	}
}
