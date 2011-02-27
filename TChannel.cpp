#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <dlfcn.h>
using namespace std;
#include "TChannel.h"

extern list<TPlugin*> plugins;

TChannel::TChannel (string channame,TConnection* parent_)
{
	name=channame;
	cout << "new TChannel ("<< channame << "," << parent_ << ")" << endl;
	
	parent=parent_;

	receiving_names_list=false;
	
	ircmessage curr_msg;
	curr_msg.origin=""; curr_msg.content=""; curr_msg.command=""; curr_msg.params="";
	
	for (list<TPlugin*>::iterator it=plugins.begin(); it!=plugins.end(); it++)
	{
		TPluginParent::hiddenaddplugincontext((*it)->get_default_flags_for_channels(), (*it)->get_context_size());
		if (contexts.rbegin()->flags & PFLAGS_EXEC_ONCREATE)
			(*it)->execute( &(*(contexts.rbegin())) , curr_msg, this, PFLAGS_EXEC_ONCREATE);
	}
	
	parent->send("mode "+name+NEWLINE);
}
TChannel::~TChannel ()
{
	cout << "~TChannel (" << name << ")" << endl;
	
	cout << "executing all plugins with the ONDESTROY flag set..." << endl;
	list<TPlugin*>::iterator it2=plugins.begin();
	
	ircmessage curr_msg=static_cast<TConnection*>(parent)->get_curr_msg();
		
	for (list<plugincontext>::iterator it=contexts.begin(); it!=contexts.end(); it++)
	{
		if (it->flags & PFLAGS_EXEC_ONDESTROY)
			(*it2)->execute( &(*it) , curr_msg, this, PFLAGS_EXEC_ONDESTROY);
			
		it2++;
	}
}

void TChannel::show_users()
{
	list<string>::iterator it;
	cout << "---------userlist von "<<name<<" -------" << endl;
	users.show_list();	
	cout << endl << "---------------------------------------------------" << endl;
		
}

void TChannel::interpret_message (ircmessage msg)
{
//	cout << "chanmodes are " << chanmodes << endl;
//	cout << "topic is " << chantopic << endl;
//	show_users();

	string temp;
	
	if (  ucase(msg.command)=="JOIN" )
		if (ucase(msg.content)==ucase(name))
		{
			msg_for_us=true;
			users.addtolist(msg.origin);
		}
		
	if (ucase(msg.command)=="PART")
		if (ucase(msg.params)==ucase(name))
		{
			msg_for_us=true;
			users.removefromlist(msg.origin);
		}
		
	if (ucase(msg.command)=="NICK")
		if (users.isinlist(msg.origin))
		{
			msg_for_us=true;
			users.addtolist(msg.content, users.get_info(msg.origin)); //infos übernehmen!
			users.removefromlist(msg.origin);
		}
		
	if (ucase(msg.command)=="QUIT")
		if (users.isinlist(msg.origin))
		{
			msg_for_us=true;
			users.removefromlist(msg.origin);
		}
		
	if (ucase(msg.command)=="KICK")	//1. param: where?;  2. param: who?
		if (ucase(ntharg(msg.params,1))==ucase(name))
		{
			msg_for_us=true;
			users.removefromlist(ntharg(msg.params,2));
		}
	
	if (ucase(msg.command)=="PRIVMSG")
		if (ucase(msg.params)==ucase(name))
			msg_for_us=true;
			
	if (ucase(msg.command)=="NOTICE")
		if (ucase(msg.params)==ucase(name))
			msg_for_us=true;
			
	if (ucase(msg.command)=="MODE")
	{
		if (ucase(ntharg(msg.params,1))==ucase(name))
			{
				msg_for_us=true;

				list<ircmode_t> l=parsemodes(msg.params);
				for (list<ircmode_t>::iterator it=l.begin(); it!=l.end(); it++) //TODO
				{
					string what=it->mode;
					string who=it->param;

					string hasmode;
					switch (it->mode[1])
					{
						case 'q':
						case 'a':
						case 'o':
						case 'h':
						case 'v':
							hasmode=users.get_info(it->param);
							if (it->mode[0]=='+')  //added a mode?
							{
								if (numchanperm(it->mode.substr(1)) > numchanperm(hasmode)) //added a better mode than the user previously had? if not, ignore.
									hasmode=it->mode[1];
							}
							else // removed a mode?
							{
								if (it->mode.substr(1)==hasmode) //removed the mode a user has? better and "worse" modes can be ignored.
								{
									if (it->mode[1]=='v') //removed voice? then the result is "nothing"
									{
										hasmode="";
									}
									else //removed something different than voice?
									{
										parent->send ("names "+name+NEWLINE);
									}
								}
							}
							users.edit(it->param,hasmode);
							break;
						 case 'b': //ignore; do not send a /mode message
							break;
						default:
							parent->send ("mode "+name+NEWLINE);
							break;
					}
				}
					
			}
				
	}
	
	if (ucase(msg.command)=="TOPIC")
		if (ucase(msg.params)==ucase(name))
			chantopic=msg.content;
	
	int pos,pos2;
	string where;
	switch (atoi(msg.command.c_str()))
	{
		case 353:	if (lcase(msg.params.substr(msg.params.rfind(' ')+1))==lcase(name))
					{
						if (!receiving_names_list)
						{
							users.clear();
							receiving_names_list=true;
						}
						string temp;
						string temp2=msg.content;
						while ((temp=split(temp2))!="")
						{
							string modechar="";

							switch(temp[0])
							{
								case '*': modechar="q"; break;
								case '!': modechar="a"; break;
								case '@': modechar="o"; break;
								case '%': modechar="h"; break;
								case '+': modechar="v"; break;
							}
							if (modechar!="") //i.e., if there was a *!@%+
								temp=temp.substr(1);
								
							users.addtolist(temp, modechar);
						}
					}
					break;
		case 366:	if (lcase(msg.params.substr(msg.params.rfind(' ')+1))==lcase(name)) 
						receiving_names_list=false;
					break;
		case 324:	if (lcase(ntharg(msg.params,2))==lcase(name))  //antwort auf /mode?
						chanmodes=ntharg(msg.params,3);
					break;
		case 332:	if (lcase(ntharg(temp,2))==lcase(name))  //topic-antwort für uns?
						chantopic=msg.content;
					break;
		case 331:	if (lcase(ntharg(temp,2))==lcase(name))  //notopic-antwort für uns?
						chantopic="";
					break;
	}
	
	if (msg_for_us)
		cout << "in " << name << ": " << msg.origin << " " << msg.command << " " << msg.params << " :" << msg.content << endl;
}


void TChannel::exec_plugins(list<TPlugin*> plugins)
{
	ircmessage curr_msg=static_cast<TConnection*>(parent)->get_curr_msg();
	
	int reason;	
	
	list<TPlugin*>::iterator it2=plugins.begin();
	for (list<plugincontext>::iterator it=contexts.begin(); it!=contexts.end(); it++)
	{
		reason=0;

		if (((it->flags) & PFLAGS_EXEC_ONDEMAND) && (curr_msg) && (msg_for_us) )//ist ondemand an und ham wir ne nachricht bekommen?
			if (ucase(curr_msg.command)=="PRIVMSG")								//wars eine privmsg?
				if (curr_msg.content[0]==static_cast<TConnection*>(parent)->comchar)  //ist sie ein aufruf?
					if (match(curr_msg.content.substr(1),(*it2)->get_name()))	//ist sie an uns gerichtet?  //TODO: wenn cmd geändert wird, z.B. dass <nick>, foo statt !foo auch geht, ändern!!!
						reason|=PFLAGS_EXEC_ONDEMAND;
			
		if (((it->flags) & PFLAGS_EXEC_ONEVENT) && (curr_msg) && (msg_for_us) )
			reason|=PFLAGS_EXEC_ONEVENT;

		if (((it->flags) & PFLAGS_EXEC_ONANYEVENT) && (curr_msg) )
			reason|=PFLAGS_EXEC_ONANYEVENT;
		
		if ((it->flags) & PFLAGS_EXEC_ALWAYS)
			reason|=PFLAGS_EXEC_ALWAYS;
			
		if (reason) 
			(*it2)->execute( &(*it) , curr_msg, this, reason);
				
		it2++;
	}
}


TUserList TChannel::get_users(){return users;}

string TChannel::get_name()
{
		return name;
}

void TChannel::addplugincontext(TPlugin* plugin){TPluginParent::hiddenaddplugincontext(plugin->get_default_flags_for_channels(), plugin->get_context_size());}

string TChannel::get_modes() { return chanmodes; }
string TChannel::get_topic() { return chantopic; }

int TChannel::get_type() {return TYPE_CHAN;}

void TChannel::say (string what) 
{
	parent->send("PRIVMSG "+get_name()+" :"+what);
}
