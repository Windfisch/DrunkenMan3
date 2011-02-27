#ifndef _TPLUGINPARENT_H_
#define _TPLUGINPARENT_H_

#include "mytypes.h"
#include "myfuncs.h"

#include "TPlugin.h"
#include "TPluginParentLight.h"

class TConnection;

class TPluginParent : public TPluginParentLight
{
	public: 
		~TPluginParent();
		virtual void addplugincontext(TPlugin* plugin)=0;
		void removeplugincontext(int x);
		void zerocurrmsg();
		void deliver_message(string subject, void *data);
		
	protected:
		void hiddenaddplugincontext(int flags, int csize);
		list<plugincontext> contexts;
		bool msg_for_us;
};

#endif
