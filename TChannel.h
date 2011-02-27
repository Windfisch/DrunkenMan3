#ifndef _TCHANNEL_H_
#define _TCHANNEL_H_

#include "mytypes.h"
#include "myfuncs.h"

#include "TPluginParent.h"

#include "TPlugin.h"

#include "TUserList.h"
#include "TConnection.h"
class TConnection;

class TChannel : public TPluginParent
{
	public:
		TChannel(string channame, TConnection* parent_);
		~TChannel();
		void interpret_message (ircmessage msg);
		virtual string get_name();
		virtual void say (string what);
		void show_users ();
		void exec_plugins(list<TPlugin*> plugins);
		void addplugincontext(TPlugin* plugin);
		TUserList get_users();
		string get_modes();
		string get_topic();
		
		virtual int get_type();
		
	private:
		TUserList users;
		string name;
		string chanmodes;
		string chantopic;
		bool receiving_names_list;
};

#endif
