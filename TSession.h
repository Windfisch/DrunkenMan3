#ifndef _TSESSION_H_
#define _TSESSION_H_

#include "mytypes.h"
#include "myfuncs.h"

#include "TPluginParent.h"

#include "TPlugin.h"

#include "TConnection.h"
class TConnection;

class TSession : public TPluginParent
{
	public:
		TSession(string nickname,TConnection* parent_);
		~TSession();
		void interpret_message (ircmessage msg);
		virtual string get_name();
		virtual void say (string what);
		void addplugincontext(TPlugin* plugin);
		void exec_plugins(list<TPlugin*> plugins);
		bool valid();
		
		virtual int get_type();

	private:
		string nick;
		time_t lastevent;
};

#endif
