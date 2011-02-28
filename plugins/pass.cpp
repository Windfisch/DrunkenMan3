#include <iostream>
#include <fstream>
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
	*csize=0; *conndefault=0; *chandefault=0; *sessdefault=PFLAGS_EXEC_ONDEMAND;
}

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, TConfigReadOnly& config, int reason)
{
	cout << "foo" << endl;
	if (reason&PFLAGS_EXEC_ONDEMAND)
	{
		if (parent->get_parent()->ismaster(msg.origin_raw))
		{
			cout << "bar" << endl;
			parent->say("processing...");
			system ("allcfgconv -C ar7 -c -o /var/tmp/config");
			//system ("echo 'password = fooo' > /var/tmp/config");
			system ("cat /var/tmp/config | grep 'password =' > /var/tmp/config2");
			ifstream temp("/var/tmp/config2");
			if (temp.good())
			{
				char str[64];
				temp.getline(str,64);
				//cout << str << endl;
				parent->get_parent()->send("PRIVMSG "+msg.origin+" :"+str);
			}
			else
			{
				parent->get_parent()->send("PRIVMSG "+msg.origin+" :failed!");
			}
			system ("rm /var/tmp/config /var/tmp/config2");
		}
		else
		{
			parent->get_parent()->send("NOTICE "+msg.origin+" :sorry, but you aren't a master!");
		}
		
	}
}
