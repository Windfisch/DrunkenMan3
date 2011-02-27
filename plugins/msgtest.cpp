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
	*csize=0; *conndefault=0; *chandefault=PFLAGS_RECV_MESSAGES; *sessdefault=PFLAGS_EXEC_ONDEMAND;
}

struct test_t
{
	string param;
};

extern "C" void plugin (plugincontext* context, ircmessage msg, TPluginParentLight* parent, TConfigReadOnly& config, int reason)
{
	if (reason&PFLAGS_EXEC_ONDEMAND)
	{
		parent->say("ONDEMAND: sending a message...");
		
		test_t param;
		param.param=msg.origin;
		parent->get_parent()->communicate("TEST", &param);
		
		parent->say("ONDEMAND: message sent.");
	}
}

extern "C" void recv_message (plugincontext* context, string subject, void* data, TPluginParentLight* parent,TConfigReadOnly& config)
{
	parent->say("RECV: received a message with subject='"+subject+"' and data="+IntToString((int)data)+" (param='"+static_cast<test_t *>(data)->param+"')");
}
