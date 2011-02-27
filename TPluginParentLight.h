#ifndef _TPLUGINPARENTLIGHT_H_
#define _TPLUGINPARENTLIGHT_H_

#include <string>

using namespace std;

#include "mytypes.h"
#include "myfuncs.h"

#include "TConnectionInterface.h"

#include "TPlugin.h"
#include "TUserList.h"

class TPluginParentLight
{
	public:
		virtual void say (string what)=0;
		TConnectionInterface* get_parent(){return parent;}
		virtual string get_name()=0; //channame, bei sessions ownername
		virtual int get_type()=0;
	protected:
		TConnectionInterface* parent;
};
#endif
