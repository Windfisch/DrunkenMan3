#include <iostream>
#include <string>

using namespace std;

#include "../TPluginParentLight.h"
#include "../mytypes.h"
#include "../myfuncs.h"

extern "C" void init(int* csize, int* conndefault, int* chandefault, int* sessdefault)
{
	*csize=0; *conndefault=PFLAGS_EXEC_ONANYEVENT | PFLAGS_EXEC_ONCREATE; *chandefault=0; *sessdefault=0;
}

extern "C" void display2 (plugincontext* context, ircmessage msg, TPluginParentLight* parent, int reason)
{
	//if (ucase(msg.command)=="PRIVMSG")
	if (reason&PFLAGS_EXEC_ONANYEVENT)
		cout << msg.origin << ": " << msg.command << " " << msg.params << ": " << msg.content << endl;
		
	if (reason&PFLAGS_EXEC_ONCREATE)
		cout << endl << "!!!!!!!!!!!!!!connected.!!!!!!!!!!!!!!" << endl << endl;
}
