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
	*csize=0; *conndefault=0; *chandefault=PFLAGS_EXEC_ONDEMAND; *sessdefault=0;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, int reason)
{
	if (reason&PFLAGS_EXEC_ONDEMAND)
		if (ucase(msg.command)=="PRIVMSG")
			if (parent->get_parent()->isuser(msg.origin))
			{
				list<string> foo=parent->get_parent()->get_channel_users(parent->get_name()).give_list();
				for (list<string>::iterator it=foo.begin(); it!=foo.end(); it++)
					parent->say ("hello, "+*it+"!");
			}
			else
			{
				parent->say ("sorry, you aren't a user, "+msg.origin+"!");
			}
}
