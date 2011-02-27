#include <ctime>
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
#include "TSession.h"

extern list<TPlugin*> plugins;

TSession::TSession(string nickname, TConnection* parent_)
{
	nick=nickname;
	cout << "new session: "<< nick<<endl;
	parent=parent_;
	
	ircmessage curr_msg;
	curr_msg.origin=""; curr_msg.content=""; curr_msg.command=""; curr_msg.params="";
	
	for (list<TPlugin*>::iterator it=plugins.begin(); it!=plugins.end(); it++)
	{
		TPluginParent::hiddenaddplugincontext((*it)->get_default_flags_for_sessions(), (*it)->get_context_size());
		if ((contexts.rbegin())->flags & PFLAGS_EXEC_ONCREATE)
			(*it)->execute( &(*(contexts.rbegin())) , curr_msg, this, PFLAGS_EXEC_ONCREATE);
	}
}

TSession::~TSession()
{
	cout << "session for "<<nick<<" ends."<<endl;
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

void TSession::interpret_message(ircmessage msg)
{
	if (ucase(msg.command)=="PRIVMSG")	//wir kriegen eine nachricht?
		if (ucase(msg.params)==ucase(parent->get_nick())) //keine channelnachricht?
			if (ucase(msg.origin_raw)==ucase(nick))  //vom besitzer dieser session?
				msg_for_us=true;					//dann ist sie für uns!
				
	if (ucase(msg.command)=="NOTICE")	//wir kriegen eine notice?
		if (ucase(msg.params)==ucase(parent->get_nick())) //keine channelnachricht?
			if (ucase(msg.origin_raw)==ucase(nick))  //vom besitzer dieser session?
				msg_for_us=true;					//dann ist sie für uns!
				
	if (ucase(msg.command)=="NICK")	//jemand benennt sich einfach so um?
		if (ucase(msg.origin_raw)==ucase(nick)) //der besitzer dieser session benennt sich um?
			{
				nick=msg.content+"!"+get_hostmask(msg.origin_raw);
				msg_for_us=true;	//das ist einer erwähnung würdig, oder?
			}
	
	if (msg_for_us) lastevent=time(NULL);
	
}

void TSession::exec_plugins(list<TPlugin*> plugins)
{
	int reason;
	ircmessage curr_msg=static_cast<TConnection*>(parent)->get_curr_msg();
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

bool TSession::valid(){
	return (time(NULL)-lastevent < 10); //600); //timeout
}

string TSession::get_name()
{
	cout << "sessGETNAME" << nick << endl;
	return nick;
}
void TSession::addplugincontext(TPlugin* plugin){TPluginParent::hiddenaddplugincontext(plugin->get_default_flags_for_sessions(), plugin->get_context_size());}

int TSession::get_type() {return TYPE_SESS;}

void TSession::say (string what)
{
	parent->send("PRIVMSG "+cut_nick(get_name())+" :"+what);
}
