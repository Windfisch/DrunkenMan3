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
	*csize=0; *conndefault=0; *chandefault=PFLAGS_EXEC_ONEVENT; *sessdefault=0;
}

extern "C" void test (plugincontext* context, ircmessage msg, TPluginParentLight* parent, int reason)
{
	if (reason&PFLAGS_EXEC_ONEVENT)
		if (parent->get_parent()->ismaster(msg.origin))
			parent->say (ucase("hallo meister!"));
}
