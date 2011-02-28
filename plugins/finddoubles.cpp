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
#include <sstream>

#include "../TConnectionInterface.h"
#include "../TPluginParentLight.h"
//#include "../TUserList.h"
#include <vector>
#include "../mytypes.h"
#include "../myfuncs.h"

struct whoisinfo
{
	string who;
	string host;
	bool proc;
};

inline std::string tostring(int x)
 {
   std::ostringstream o;
   if (!(o << x))
     throw;
   return o.str();
 } 

vector<whoisinfo>::iterator whereinlist(vector<whoisinfo>* l, string s)
{
	for (vector<whoisinfo>::iterator i=l->begin();i!=l->end();i++)
		if (  lcase( i->who ) == lcase(s)  )
			return i;
			
	return l->end();
}

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=sizeof(char)+sizeof(string *)+sizeof(vector<whoisinfo> *)+sizeof(int); *conndefault=0; *chandefault=PFLAGS_EXEC_ONDEMAND; *sessdefault=PFLAGS_EXEC_ONDEMAND;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, int reason)
{
	int* whoiscnt=(int*) ( (vector<whoisinfo> **) ( (string**) ( (char*)  context->data +1) +1) +1);
	if (reason&PFLAGS_EXEC_ONDEMAND)
	{
		if (ucase(msg.command)=="PRIVMSG")
		{
			if (*((char*)(context->data)) != 0)
			{
				if (lcase(trim(ntharg(msg.content,2)))=="abort")
				{
					parent->say ("aborted for channel "+**  ((string**)((char*)context->data+1)));

					string ** cptrdest= (string**) ((char*) context->data  +  1);
					if (*cptrdest)
						delete (*cptrdest);
					*cptrdest=NULL;
					
					vector<whoisinfo> **lptr= (vector<whoisinfo> **) ((string**)(((char*) context->data+1))+1);					
					*lptr=new vector<whoisinfo>;
					if (*lptr)
						delete (*lptr);
					*lptr=NULL;
					
					*((char*)(context->data))=0;
					
					context->flags=PFLAGS_EXEC_ONDEMAND;
				}
				else
				{
					parent->say("sorry, try again later.");
				}
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
					
					vector<whoisinfo> **lptr= (vector<whoisinfo> **) ((string**)(((char*) context->data+1))+1);
					
					*lptr=new vector<whoisinfo>;
					
					*whoiscnt=0;
				}
			}
		}
	}	
	
	if (reason&PFLAGS_EXEC_ONANYEVENT)
	{
		vector<whoisinfo>** lptr= (vector<whoisinfo>**) ((string**)(((char*) context->data+1))+1);
		vector<whoisinfo>* liste=*lptr;
		
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
					//liste->addtolist(temp);
					
					whoisinfo structtemp;
					structtemp.who=temp;
					structtemp.host="";
					structtemp.proc=false;
					liste->push_back(structtemp);
					
					parent->get_parent()->send("whois "+temp+NEWLINE);
					(*whoiscnt)++;
				}
			}
		}
		if (numcmd==366)
		{
//			parent->say ("fertig empfangen...");
			*((char*)(context->data)) = 2;
		}
		
		if (numcmd==311)
		{
//			parent->say ("whoisantwort für "+ntharg(msg.params,2)+": "+ntharg(msg.params,4));
			
			vector<whoisinfo>::iterator pos;
			
			if ((pos=whereinlist(liste, ntharg(msg.params,2)))!=liste->end())
			{
				pos->host=ntharg(msg.params,4);
			//	liste->edit (ntharg(msg.params,2),ntharg(msg.params,4));
				(*whoiscnt)--;
			}
		}
		
		//cout << "DEBUG: " << *whoiscnt << "," << *((char*)(context->data)) << endl;
		
		if ((*whoiscnt==0)&&(*((char*)(context->data))==2))
		{
			int sz=liste->size();
			int cnt;
			
			string saytemp;
			bool shared=false;
			
			for (int i=0; i<sz; i++)
			{
				saytemp=(*liste)[i].who;
				cnt=1;
				if (   (*liste)[i].proc == false )
				{
					for (int j=i+1; j<sz; j++)
					{
						if (  (*liste)[i].host == (*liste)[j].host )
						{
							saytemp+=", "+(*liste)[j].who;
							cnt++;
							(*liste)[i].proc=true;
						}
					}
				}
				if (cnt>1)
				{
					parent->say(tostring(cnt)+" people are sharing the hostmask "+(*liste)[i].host+": "+saytemp);
					shared=true;
				}
			}
			
			
			if (!shared)
				parent->say("there were no people sharing a hostmask in " + (**  ((string**)((char*)context->data+1)))+".");


			string ** cptrdest= (string**) ((char*) context->data  +  1);
			if (*cptrdest)
				delete (*cptrdest);
			*cptrdest=NULL;
			
			vector<whoisinfo> **lptr= (vector<whoisinfo> **) ((string**)(((char*) context->data+1))+1);					
			*lptr=new vector<whoisinfo>;
			if (*lptr)
				delete (*lptr);
			*lptr=NULL;
					
			*((char*)(context->data))=0;
			
			context->flags=PFLAGS_EXEC_ONDEMAND;
		}

		
	}
}

