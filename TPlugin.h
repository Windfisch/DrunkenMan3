#ifndef _TPLUGIN_H_
#define _TPLUGIN_H_

#include "mytypes.h"
#include "myfuncs.h"


class TPlugin
{
	public:
		TPlugin(string pluginname);
		~TPlugin();
		void execute(plugincontext* context,ircmessage msg, TPluginParent* parent, int reason);
		void push_message(plugincontext* context, string subject, void* data, TPluginParent* parent);
		string get_name();
		int get_context_size();
		int get_default_flags_for_channels();
		int get_default_flags_for_connections();
		int get_default_flags_for_sessions();
		
	private:
		int default_flags_for_channels;
		int default_flags_for_connections;
		int default_flags_for_sessions;
		
		int context_size; //in bytes
		string name;
		void* handle;
		pluginfunc func;
		pluginrecvfunc recv_msg;
};
#endif
