#ifndef _TCONNECTIONINTERFACE_H_
#define _TCONNECTIONINTERFACE_H_

#include <string>
#include "TUserList.h"
//#include "mytypes.h"
//#include "myfuncs.h"

class TConnectionInterface  // : public TPluginParent
{
	public:
		virtual void send (string line)=0;
		
		virtual string get_nick()=0; //unser nick!
				
		virtual string get_name()=0; //name des servers
		virtual string get_networkname()=0; //name des netzwerks
		
		virtual void addmaster(string nick,string pass)=0;
		virtual void adduser(string nick,string pass)=0;
		
		virtual void delmaster(string nick)=0;
		virtual void deluser(string nick)=0;
		
		virtual bool ismaster(string nick)=0;
		virtual bool isuser(string nick)=0;
		virtual bool isinchan(string nick,string chan)=0;
		
		virtual TUserList get_masterlist()=0;
		virtual TUserList get_userlist()=0;
		virtual TUserList get_channel_users(string chan)=0;
		
		virtual string get_channel_topic(string chan)=0;
		virtual string get_channel_modes(string chan)=0;
		
		virtual void quit(string reason)=0;
		
		virtual void communicate(string subject, void *data)=0;
};


#endif
