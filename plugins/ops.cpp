#include <iostream>
#include <string>
#include <map>

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
	map<string,string> tmp;
	
	if (reason&PFLAGS_EXEC_ONDEMAND)
	{
		string str("");
		tmp = parent->get_parent()->get_channel_users(parent->get_name()).give_list();
		for (map<string,string>::iterator it=tmp.begin(); it!=tmp.end(); it++)
			if (numchanperm(it->second)>=2)
				if (ucase(it->first)!="CHANSERV")
					str+=it->first+" ";
		parent->say(str);
	}	
}
