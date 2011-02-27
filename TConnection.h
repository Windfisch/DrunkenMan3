#ifndef _TCONNECTION_H_
#define _TCONNECTION_H_

#include "mytypes.h"
#include "myfuncs.h"

#include "TConnectionInterface.h"
#include "TPluginParent.h"

#include "TPlugin.h"

#include "TSession.h"
#include "TChannel.h"

class TSession;
class TChannel;

class TConnection : public TPluginParent, public TConnectionInterface
{
	public:
		TConnection(string server, string nicklist, char comchar_);
		~TConnection();
		
		virtual void send (string line);
		virtual void send_raw (string line);
		static ircmessage parseline (string line);
//		static string cut_nick (string nick1);
		virtual string get_nick();
		
		void exec_plugins(list<TPlugin*> plugins);
		
		virtual string get_name();
		virtual string get_networkname();
		static string get_networkname(string s_name);
		string get_connectname();
		
		//virtual string get_ournick();
		
		//virtual void pluginsend(string what);
		//virtual TConnection* get_parent();
		
		void addplugincontext(TPlugin* plugin);
		void removeplugincontext(int x);
		void zerocurrmsg();
		
		void loadlists();
		void savelists();
		
		virtual void addmaster(string nick, string pass);
		virtual void adduser(string nick, string pass);
		
		virtual void delmaster(string nick);
		virtual void deluser(string nick);
		
		virtual bool ismaster(string nick);
		virtual bool isuser(string nick);
		virtual bool isinchan(string nick,string chan);
		
		virtual TUserList get_masterlist();
		virtual TUserList get_userlist();
		virtual TUserList get_channel_users(string chan);

		virtual string get_channel_topic(string chan);
		virtual string get_channel_modes(string chan);
		
		virtual void quit(string reason);
		
		ircmessage get_curr_msg();
		//string get_curr_msg();
		bool next_message();
		
		void check_session_timeouts();
		
		void logout_all();
		
		bool valid();
		void check_timeout();
		
		virtual int get_type();
		
		virtual void communicate(string subject, void *data);
		virtual void say (string what);
		
		bool dontreconnect;
		char comchar;

	private:
		void connect(string* server);
		void disconnect ();
		void register_client(string nicks);
		
		string getline();
		void interpret_message(ircmessage msg);
		
		int sock;
		struct sockaddr_in addr;
		struct hostent* hent;
		char* buf;
		string nick;
		list<TChannel*> chans;
		list<TSession*> sessions;
		TUserList masters, masters_li;
		TUserList users, users_li;
		string sname;
		string connectname;
		ircmessage curr_msg;
		time_t last_recv;
		time_t last_ping;
		bool expecting_pong;
		string newline;
};


#endif
