/* TODO
 * 470 forwarding to another channel implementieren
 * evtl timeout implementieren
 * aus config folgendes lesen:
 *   rejoinen?
 *   wie lange warten?
 * 
 */

#include <iostream>
#include <string>
#include <list>

using namespace std;

#include "../TConnectionInterface.h"
#include "../TPluginParentLight.h"
#include "../TUserList.h"
#include "../mytypes.h"
#include "../myfuncs.h"
#include "../TConfigReadOnly.h"


#define FLAGS_INVITE 1
#define FLAGS_UNBAN  2

struct entry_t
{
	char state;
	char flags;
	int cnt;
	int max;
	time_t last;
	string name;
};

int calc_wait (int cnt);

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=sizeof(list<entry_t>*);
	*conndefault=PFLAGS_EXEC_ONEVENT; *chandefault=0; *sessdefault=0;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, TConfigReadOnly& config, int reason)
{
	list<entry_t> *liste=*static_cast<list<entry_t> **>(context->data);
	if (liste==0)
	{
		liste=new list<entry_t>;
		*static_cast<list<entry_t> **>(context->data)=liste;
	}
	
	if (reason&PFLAGS_EXEC_ONEVENT)
	{
		if ((msg.command=="KICK") && (lcase(ntharg(msg.params,2))==lcase(parent->get_parent()->get_nick()))) //wir wurden gekickt? sauerei!
		{
			cout << "we got kicked from " << ntharg(msg.params,1) << endl;
			if (config.get_valid_boolean(parent->get_parent()->get_networkname() + "." + ntharg(msg.params,1) + ".rejoin", false))
			{
				cout << "  trying to rejoin" << endl;
					
				entry_t tmp;
				tmp.state=1;
				tmp.flags=0;
				tmp.last=time(NULL)+1; //fake time to wait 1 second ONCE
				tmp.cnt=0;
				tmp.max=config.get_valid_integer(parent->get_parent()->get_networkname() + "." + ntharg(msg.params,1) + ".rejoin_tries", 10);
				tmp.name=ntharg(msg.params,1);
				liste->push_back(tmp);

				context->flags |= PFLAGS_EXEC_ALWAYS;
			}
		}
	}
	
	if (reason&PFLAGS_EXEC_ALWAYS)
	{
		for (list<entry_t>::iterator it=liste->begin(); it!=liste->end(); it++)
		{
			switch (it->state)
			{
				case 1:
					if (time(NULL)>(it->last+calc_wait(it->cnt)))
					{
						parent->get_parent()->send("join "+it->name);
						it->state=2;
					}
					break;
				case 2: //waiting for JOIN answer			
					if ((ucase(msg.command)=="JOIN") && (ucase(msg.origin)==ucase(parent->get_parent()->get_nick())))
					{
						it->state=99;
					}
					else
					{
						if ((lcase(ntharg(msg.params,1))==lcase(parent->get_parent()->get_nick())) && (lcase(ntharg(msg.params, 2))==lcase(it->name)))
						{
							int numcmd=atoi(msg.command.c_str());
							switch (numcmd)
							{
								case 471: //channel is full
									it->last=time(NULL);
									it->state=1;
									it->cnt++;
									break;
								case 473: //inviteonly
									if (!(it->flags&FLAGS_INVITE)) //try inviting via chanserv only once. if it fails once, it will fail forever!
									{
										it->flags|=FLAGS_INVITE;
										parent->get_parent()->send("PRIVMSG ChanServ :invite "+it->name);
										it->state=3;
									}
									else
									{
										it->last=time(NULL);
										it->state=1;
										it->cnt++;
									}
									break;
								case 474: //banned
									if (!(it->flags&FLAGS_UNBAN)) //try unbanning only once.
									{
										it->flags|=FLAGS_UNBAN;
										parent->get_parent()->send("PRIVMSG ChanServ :unban "+it->name);
										it->state=3;
									}
									else
									{
										it->last=time(NULL);
										it->state=1;
										it->cnt++;
									}
									break;

								case 475: //bad key, no such chan, too many chans. fatal.
								case 403:
								case 405:
									it->state=99;
									break;
							}
						}
					}
					break;
				
				case 3: //waiting for CHANSERV answer
					if ( ((msg.command=="401") && (ucase(ntharg(msg.params,2))=="CHANSERV")) || ((msg.command=="NOTICE") && (ucase(msg.origin)=="CHANSERV")) )
					{
						//answer arrived or no chanserv?
						it->last=time(NULL);
						it->state=1;
					}

					break;
			}
		}
		
		bool temp=true;
		while (temp)
		{
			temp=false;
			for (list<entry_t>::iterator it=liste->begin(); it!=liste->end(); it++)
			{
				if ((it->state==99) || ((it->max!=0) && (it->cnt > it->max)))
				{
					cout << "cleanup: removing entry for channel " << it->name << "..." << endl;
					liste->erase(it);
					temp=true;
					break;
				}
			}
		}
		
	}
}

int calc_wait (int cnt)
{
	int tmp;
	
	tmp=1 << cnt;
	if (tmp>30)
		tmp=30;
		
	return tmp;
}
