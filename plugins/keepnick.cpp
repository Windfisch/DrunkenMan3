#include <iostream>
#include <string>

using namespace std;

#include "../TConnectionInterface.h"
#include "../TPluginParentLight.h"
#include "../TUserList.h"
#include "../mytypes.h"
#include "../myfuncs.h"
#include "../TConfigReadOnly.h"


#define WAITING_NS_ID 1
#define WAITING_NS_GHOST 2
#define NONE 0



extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=1; *conndefault=PFLAGS_EXEC_ONCREATE | PFLAGS_EXEC_ONEVENT; *chandefault=0; *sessdefault=0;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, TConfigReadOnly& config, int reason)
{
	bool check=false;
	char *state=(char*) context->data;
	
	if (reason & PFLAGS_EXEC_ONCREATE)
		check=true;
	
	if (reason & PFLAGS_EXEC_ONEVENT)
	{
		if (config.get_valid_boolean(parent->get_parent()->get_networkname() + ".paranoidnick", false))
			if (ucase(msg.command)=="NICK")
				if (ucase(msg.content)==ucase(parent->get_parent()->get_nick())) //our nick got changed? i don't want that!  //content weil wir NACH dem updaten gerufen werden
					check=true;
		
		switch (*state)
		{
			case WAITING_NS_ID:
				if ((msg.command=="401") && (ucase(ntharg(msg.params,2))=="NICKSERV")) //no such nick/channel. no nickserv on that server?
				{
					cout << "no nickserv" << endl;
					parent->get_parent()->send("NICK "+config.get_string(parent->get_parent()->get_networkname() + ".nick"));
					*state=NONE;
				}
				else if ((msg.command=="NOTICE") && (ucase(msg.origin)=="NICKSERV")) //a notice from nickserv?
				{
					cout << "nickserv answered!" << endl;
					parent->get_parent()->send("PRIVMSG nickserv :ghost "+config.get_string(parent->get_parent()->get_networkname() + ".nick"));
					*state=WAITING_NS_GHOST;
				}
				break;
			case WAITING_NS_GHOST:
				if ((msg.command=="401") && (ucase(ntharg(msg.params,2))=="NICKSERV")) //no such nick/channel. no nickserv on that server?
				{
					cout << "no nickserv" << endl;
					parent->get_parent()->send("NICK "+config.get_string(parent->get_parent()->get_networkname() + ".nick"));
					*state=NONE;
				}
				else if ((msg.command=="NOTICE") && (ucase(msg.origin)=="NICKSERV")) //a notice from nickserv?
				{
					cout << "nickserv answered!" << endl;
					parent->get_parent()->send("NICK "+config.get_string(parent->get_parent()->get_networkname() + ".nick"));
					*state=NONE;
				}
				break;
				
		}
	}

	if (check)
	{
		if (ucase(parent->get_parent()->get_nick())!=ucase(config.get_string(parent->get_parent()->get_networkname() + ".nick")))
		{
			cout << "that's not the nick we want! trying to change it" << endl;
			
			parent->get_parent()->send("PRIVMSG nickserv :identify "+config.get_string(parent->get_parent()->get_networkname() + ".nick")+" "+config.get_string(parent->get_parent()->get_networkname() + ".pass"));
			*state=WAITING_NS_ID;
		}
	}
}
