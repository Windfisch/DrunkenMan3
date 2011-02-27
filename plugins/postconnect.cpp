#include <iostream>
#include <string>

using namespace std;

#include "../TConnectionInterface.h"
#include "../TPluginParentLight.h"
#include "../TUserList.h"
#include "../mytypes.h"
#include "../myfuncs.h"
#include "../TConfigReadOnly.h"

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=0; *conndefault=PFLAGS_EXEC_ONCREATE; *chandefault=0; *sessdefault=0;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, TConfigReadOnly& config, int reason)
{
	if (reason&PFLAGS_EXEC_ONCREATE)
	{
		cout << parent->get_parent()->get_networkname() << endl;
		cout << config.get_string(parent->get_parent()->get_networkname() + ".join") << endl;
		//evtl aus datei lesen?
		parent->get_parent()->send ("join #DrunkenMan" NEWLINE);
		
		if (config.is_string(parent->get_parent()->get_networkname()+".pass"))
			parent->get_parent()->send("PRIVMSG NickServ :identify "+config.get_string(parent->get_parent()->get_networkname()+".nick")+" "+config.get_string(parent->get_parent()->get_networkname()+".pass"));

	}
}
