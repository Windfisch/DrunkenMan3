#include <cerrno>
#include <cstdlib>
#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <dlfcn.h>
#include <ctime>

using namespace std;
#include "TConnection.h"


extern list<TPlugin*> plugins;


TConnection::TConnection(string server, string nicklist, char comchar_)
{
	dontreconnect=false;
	parent=this;
	newline="\r\n";
	comchar=comchar_;

	sock=-1;
	buf=new char[BUFLEN];
	for (int i=0;i<BUFLEN;i++) buf[i]=0;
	
	try
	{
		connect (&server);
		register_client(nicklist);
		loadlists();
	}
	catch (...)
	{
		cout << "grml" << endl;
		throw;
	}
		
	
	ircmessage curr_msg;
	curr_msg.origin=""; curr_msg.content=""; curr_msg.command=""; curr_msg.params="";
	
	for (list<TPlugin*>::iterator it=plugins.begin(); it!=plugins.end(); it++)
	{
		TPluginParent::hiddenaddplugincontext((*it)->get_default_flags_for_connections(), (*it)->get_context_size());
		if (contexts.rbegin()->flags & PFLAGS_EXEC_ONCREATE)
			(*it)->execute( &(*(contexts.rbegin())) , curr_msg, this, PFLAGS_EXEC_ONCREATE);
	}
	
}
TConnection::~TConnection()
{
	cout << "executing all plugins with the ONDESTROY flag set..." << endl;
	list<TPlugin*>::iterator it2=plugins.begin();
	
	ircmessage curr_msg=static_cast<TConnection*>(parent)->get_curr_msg();
		
	for (list<plugincontext>::iterator it=contexts.begin(); it!=contexts.end(); it++)
	{
		if (it->flags & PFLAGS_EXEC_ONDESTROY)
			(*it2)->execute( &(*it) , curr_msg, this, PFLAGS_EXEC_ONDESTROY);
			
		it2++;
	}

	delete []buf;
}

void TConnection::exec_plugins(list<TPlugin*> plugins)
{
	int reason;
	list<TPlugin*>::iterator it2=plugins.begin();
	for (list<plugincontext>::iterator it=contexts.begin(); it!=contexts.end(); it++)
	{
		reason=0;

		if ((it->flags) & PFLAGS_EXEC_ALWAYS)
			reason|=PFLAGS_EXEC_ALWAYS;
		
		if (((it->flags) & PFLAGS_EXEC_ONEVENT) && (curr_msg) )
			reason|=PFLAGS_EXEC_ONEVENT;
			
		if (((it->flags) & PFLAGS_EXEC_ONANYEVENT) && (curr_msg) )
			reason|=PFLAGS_EXEC_ONANYEVENT;
		
		if (reason) 
			(*it2)->execute( &(*it) , curr_msg, this, reason);
				
		it2++;
	}
	for (list<TChannel*>::iterator it=chans.begin();it!=chans.end();it++)
		(*it)->exec_plugins(plugins);
	
	for (list<TSession*>::iterator it=sessions.begin(); it!=sessions.end(); it++)
		(*it)->exec_plugins(plugins);
}
void TConnection::addplugincontext(TPlugin* plugin)
{
	TPluginParent::hiddenaddplugincontext(plugin->get_default_flags_for_connections(), plugin->get_context_size());
	
	for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
		(*it)->addplugincontext(plugin);

	for (list<TSession*>::iterator it=sessions.begin(); it!=sessions.end(); it++)
		(*it)->addplugincontext(plugin);
		
	
}

void TConnection::removeplugincontext(int x)
{
	TPluginParent::removeplugincontext(x);

	for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
		(*it)->removeplugincontext(x);
		
	for (list<TSession*>::iterator it=sessions.begin(); it!=sessions.end(); it++)
		(*it)->removeplugincontext(x);
}

void TConnection::connect(string* server)
{
	sock=socket(AF_INET,SOCK_STREAM,0);
	if (sock<=0) 
	{
		sock=-1;
		throw 1;
	}
	
	hent=gethostbyname(server->c_str());
	if (hent)
	{
		addr.sin_family=AF_INET;
		addr.sin_port=htons(6667);
		addr.sin_addr.s_addr=*((uint32_t*)hent->h_addr_list[0]);
		
		if (::connect( sock,(struct sockaddr *)&addr,sizeof(addr))==0) 
		{
			//cout << "GOOD:    we're connected to " << hent->h_name << " (" << inet_ntoa(addr.sin_addr) << ")" << endl;
			sname=*server;
			connectname=*server;
			last_recv=time(NULL);
			expecting_pong=false;
		}
		else
		{
			disconnect();
			throw 3;
			//cout << "FATAL:   connecting to " << hent->h_name << " (" << inet_ntoa (addr.sin_addr) << ") failed!         program ends.\n";
		}
	}
	else
	{
		disconnect();
		throw 2;
	}
}

void TConnection::disconnect()
{
	if (sock>0)
	{
		close(sock);
		sock=-1;
	}
}

string TConnection::getline()
{
	fd_set sockset;
	struct timeval tv;
	int retval;
	int size;

	char* pos;
	char* temp;
	int len;
	
	string line="";
	
	if (valid()) //nur empfangen, wenn die verbindung noch steht!
	{
		tv.tv_sec=0; tv.tv_usec=10000; 	//timeout setzen: 10msec warten...
		FD_ZERO (&sockset);				//sockset leeren
		FD_SET(sock,&sockset);				//und füllen
		retval=select (sock+1,&sockset,NULL,NULL,&tv);
		
		if (retval>0) 					//neue daten da?
		{
			last_recv=time(NULL);
			expecting_pong=false;
			len=strlen(buf);
			if ((BUFLEN-1-len)>0)	//wenn noch platz im buffer frei ist:
			{
				size=recv(sock, buf+len ,  BUFLEN-1-len  , 0);	//empfangen ab \0 im puffer (\0 wird überschrieben!)
				if (size<=0)
				{
					cout << "ERROR:    connection for '" << sname << "' has been closed!" << endl;
					disconnect();
				}
				buf[size+len]=0;							//aber weiter hinten wieder angehängt!
			}																	//aber höchstens so viel wie der buffer packt!
		}
		else         					//nix oder fehler?
		{
			if (retval==-1)
			{
				cout << "ERROR:   error using select ("<<retval<<")"<<endl;
				disconnect();
			}
		}
	}
	
	
	if (pos=strchr(buf,'\n')) //we have \n or \r\n
  {
    if ((*(pos-1))=='\r')     //we have \r\n
      newline="\r\n";
    else                      //we have \n
      newline="\n";
  }
  else if (pos=strchr(buf,'\r')) //we have \r
    newline="\r";
  //else //no newline? will be handled below


	if ((pos=strstr(buf,newline.c_str())))	//ist da irgendwo ein newline im buffer?
	{
		//an pos ist ein NEWLINE!
		len=pos-buf; //also: die ersten len zeichen sind eine zeile
		line=string(buf,len); //nach line kopieren
		temp=buf-1; len+=newline.length();  //und diese zeile aus buf entfernen
		do { temp++; temp[0]=temp[len];  } while (temp[len]); 
	}
	else //kein newline?
	{
		if (strlen(buf)>=BUFLEN-1) //kein newline, aber trotzdem daten im puffer und puffer voll oO? das kann nur dann sein, wenn der buffer zu klein ist!
		{
			cout << "ERROR:   buffer isn't empty but there's no NEWLINE!\n         discarding the buffer (was: " << buf << ")..." << endl;
			buf[0]='\0';
		}
	}

	return line;
}

void TConnection::register_client(string nicks)
{
	nick="";
	send ("USER DrunkenMan foo bar :DrunkenMan by Florian Jung");
	
	nicks+=" ";
	srandom(time(NULL));  //notfallnick
	for (int i=0;i<8;i++)	
		nicks+=char( random ()%26 +  'a' );

	nicks+=" ";
	
	string curr_nick;
	int pos,oldpos;
	oldpos=0;
	pos=nicks.find (' ',0);
	string answer;
	ircmessage ans;
	int numcmd;
	while (pos!=string::npos)
	{
		curr_nick=nicks.substr(oldpos,pos-oldpos);
		
		send ("NICK "+curr_nick);
		while (true)
		{
			while ( (!next_message()) && valid() ) usleep (10000);
			if (valid()==false) //verbindung verloren?
				throw 99; 
			
			ans=curr_msg;
			numcmd=atoi(ans.command.c_str());
			if (numcmd==1) //success
			{
				nick=curr_nick; 
				sname=ans.origin; //this message always is sent from the server we're on
				break;
			}
			
			if ((numcmd>=431)&&(numcmd<=439)) //fail
			{
				break;
			}
		}
		
		if (nick!="") break;
		
		oldpos=pos+1;
		pos=nicks.find (' ',oldpos);
	}
	
	if (nick=="")
		throw 1;
	
}

void TConnection::send_raw (string line)
{
	int len;
	if (valid())
	{
		cout << "SEND:    "<<line<<flush;
		len=line.length();
		if (line.substr(len-newline.length(),newline.length())!=newline)
			printf ("\nWARNING: line to send doesn't end with NEWLINE!\n");
		
		::send (sock,line.c_str(),len,0);
	}
}

void TConnection::send (string line)
{
	if (line.substr(line.length()-newline.length(),newline.length())!=newline)
		line=line+newline;
	send_raw(line);
}

ircmessage TConnection::parseline (string line)
{
	ircmessage temp;
	int foundpos;
	int pos;
	
	pos=0;
	if (line.substr(0,1)==":")
	{
		foundpos=line.find (' ',0);
		temp.origin=line.substr(1,foundpos-1);
		pos=foundpos+1;
	}
	
	foundpos=line.find(' ',pos);
	if (foundpos==string::npos)
	{
		temp.command=line;
	}
	else
	{
		temp.command=line.substr(pos,foundpos-pos);
		pos=foundpos+1;
		if (line.substr(pos,1)==":")
		{
			temp.content=line.substr(pos+1);
		}
		else
		{
			foundpos=line.find(" :",pos);
			if (foundpos!=string::npos)
			{
				temp.params=line.substr(pos,foundpos-pos);
				temp.content=line.substr(foundpos+2);
			}
			else
			{
				temp.params=line.substr(pos);
			}
		}
	}
	temp.origin_raw=temp.origin;
	temp.origin=cut_nick(temp.origin);
	
	
	return temp;
}


void TConnection::interpret_message(ircmessage msg) //ggf. neue sessions öffnen, channellisten anpassen, wenn wir joinen/parten/gekickt werden
{
	curr_msg=msg;
	msg_for_us=false;
	if (curr_msg) msg_for_us=true;
			
	if (ucase(msg.command)=="PING")
		send("PONG :" + msg.content);
		
	if (ucase(msg.command)=="PRIVMSG")
	{
		if (ucase(msg.params)==ucase(nick)) //a message? for me :)?
		{
			bool newsession=true;	
			for (list<TSession*>::iterator it=sessions.begin(); it!=sessions.end(); it++) //check if this user has already a session
				if (ucase((*it)->get_name())==ucase(msg.origin_raw))
				{
					newsession=false;
					break;
				}
				
			if (newsession) //do we have to create a new session for msg.origin_raw?
				sessions.push_back(new TSession(msg.origin_raw,this)); //create it and push it to the list
		}
		
		if (msg.content[0]==comchar)
		{
			if (users.isinlist(msg.origin))
			{
				string info=users.get_info(msg.origin);
				if (match(msg.content.substr(1),"login")) //bitte um login?
				{
					string pass="";
					int pos=msg.content.find(' ');
					if (pos==string::npos)
						pass="";
					else
						pass=msg.content.substr(pos+1);
					
					if (pass==info) //richtiges passwort?
					{
						if (!users_li.isinlist(msg.origin))
						{
							users_li.addtolist(msg.origin, msg.origin_raw);
							cout << "user "<<msg.origin<<" is now logged in." << endl;
							send ("NOTICE "+msg.origin+" :you are now logged in as user.");
						}
						else
						{
							cout << "user "<<msg.origin<<" has re-logged-in" << endl;
							send ("NOTICE "+msg.origin+" :you are now logged in as user.");
							send ("NOTICE "+msg.origin+" :you were previously logged in as "+users_li.get_info(msg.origin));
							users_li.edit(msg.origin, msg.origin_raw);
						}
					}
					else
					{
						send ("NOTICE "+msg.origin+" :wrong password!");
					}
				}
				
				if (match(msg.content.substr(1),"logout"))
				{
					if (users_li.isinlist(msg.origin))
					{
						users_li.removefromlist(msg.origin);
						cout << "user "<<msg.origin<<" has been logged out." << endl;
						send ("NOTICE "+msg.origin+" :you have been logged out.");
					}
					else
					{
						send ("NOTICE "+msg.origin+" :you aren't logged in!");
					}
				}
			}


			if (masters.isinlist(msg.origin))
			{
				string info=masters.get_info(msg.origin);
				if (match(msg.content.substr(1),"masterlogin")) //bitte um login?
				{
					string pass="";
					int pos=msg.content.find(' ');
					if (pos==string::npos)
						pass="";
					else
						pass=msg.content.substr(pos+1);
					
					if (pass==info) //richtiges passwort?
					{
						if (!masters_li.isinlist(msg.origin))
						{
							masters_li.addtolist(msg.origin, msg.origin_raw);
							cout << "master "<<msg.origin<<" is now logged in." << endl;
							send ("NOTICE "+msg.origin+" :you are now logged in as master.");
						}
						else
						{
							cout << "master "<<msg.origin<<" has re-logged-in" << endl;
							send ("NOTICE "+msg.origin+" :you are now logged in as master.");
							send ("NOTICE "+msg.origin+" :you were previously logged in as "+masters_li.get_info(msg.origin));
							masters_li.edit(msg.origin, msg.origin_raw);
						}
					}
					else
					{
						send ("NOTICE "+msg.origin+" :wrong password!");
					}
				}
				
				if (match(msg.content.substr(1),"masterlogout"))
				{
					if (masters_li.isinlist(msg.origin))
					{
						masters_li.removefromlist(msg.origin);
						cout << "master "<<msg.origin<<" has been logged out." << endl;
						send ("NOTICE "+msg.origin+" :you have been logged out.");
					}
					else
					{
						send ("NOTICE "+msg.origin+" :you aren't logged in!");
					}
				}
			}
		}


	
	}		
	
	if (ucase(msg.command)=="JOIN")
		if (ucase(msg.origin)==ucase(nick))
			chans.push_back(new TChannel(msg.params,this));
		
	if (ucase(msg.command)=="PART")
		if (ucase(msg.origin)==ucase(nick))
			for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
				if (ucase((*it)->get_name())==ucase(msg.params))
				{
					delete *it;
					chans.erase (it);
					break;
				}
	
	if (ucase(msg.command)=="KICK")
	{	
		string where;
		string victim;
		where=msg.params.substr(0,msg.params.find(' ',0));
		victim=msg.params.substr(msg.params.find(' ',0)+1);
		if (ucase(victim)==ucase(nick))
			for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
				if (ucase(where)==ucase((*it)->get_name()))
				{
					delete *it;
					chans.erase(it);
					break;
				}
	}
	
	if (ucase(msg.command)=="NICK")
	{
		if (ucase(msg.origin)==ucase(nick)) //we changed our nick?
		{
			nick=msg.content;
		}
		else //someone else changed his nick?
		{
			if (ucase(msg.origin)!=ucase(msg.content)) //check lists only when the nick changed _really_ (not drunkenman to DrunkenMan)
			{
				if (isuser(msg.origin_raw)) //was logged in?
				{
					send("NOTICE "+msg.content+" :You still were logged in as '"+msg.origin+"'. Logging you out...");
					cout << "logging out user '"<<msg.origin<<"' because he changed nick" << endl;
					users_li.removefromlist(msg.origin);
				}
				if (ismaster(msg.origin_raw)) //was logged in?
				{
					send("NOTICE "+msg.content+" :You still were logged in as master '"+msg.origin+"'. Logging you out...");
					cout << "logging out master '"<<msg.origin<<"' because he changed nick" << endl;
					masters_li.removefromlist(msg.origin);
				}
			}
		}
	}
	
	if (ucase(msg.command)=="QUIT")
	{
		if (isuser(msg.origin_raw)) //was logged in?
		{
			cout << "logging out user '"<<msg.origin<<"' because he quit" << endl;
			users_li.removefromlist(msg.origin);
		}		
		if (ismaster(msg.origin_raw)) //was logged in?
		{
			cout << "logging out master '"<<msg.origin<<"' because he quit" << endl;
			masters_li.removefromlist(msg.origin);
		}		
	}
	
	for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
		(*it)->interpret_message(msg);
		
	for (list<TSession*>::iterator it=sessions.begin(); it!=sessions.end(); it++)
		(*it)->interpret_message(msg);
}

void TConnection::logout_all()
{
	cout << "clearing login-entries for " << get_networkname() << "..." << endl;
	users_li.clear();
	masters_li.clear();
}

string TConnection::get_nick()
{
	return nick;
}

string TConnection::get_name()
{
//	return ""; //todo: oder servername?
//cout << "connGETNAME" << nick << endl;
	return sname;
}

/*TConnection* TConnection::get_parent()
{
	return this;
}*/

/*void TConnection::pluginsend(string what)
{
	send(what+NEWLINE);
}*/

void TConnection::zerocurrmsg()
{
	curr_msg.origin=""; curr_msg.params=""; curr_msg.content=""; curr_msg.command="";
	TPluginParent::zerocurrmsg();  //funktion der elternklasse aufrufen
	
	for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
		(*it)->zerocurrmsg();
		
	for (list<TSession*>::iterator it=sessions.begin(); it!=sessions.end(); it++)
		(*it)->zerocurrmsg();
}

void TConnection::loadlists()
{
	users.loadfromfile("./etc/"+get_networkname()+"/users.txt");
	masters.loadfromfile("./etc/"+get_networkname()+"/masters.txt");
}

void TConnection::savelists()
{
	users.savetofile("./etc/"+get_networkname()+"/users.txt");
	masters.savetofile("./etc/"+get_networkname()+"/masters.txt");
}


void TConnection::adduser(string nick, string pass)
{
	users.addtolist(nick, pass);
}

void TConnection::deluser(string nick)
{
	users.removefromlist(nick);
}

bool TConnection::isuser(string nick)
{
	if (nick.find('!')==string::npos)
	{
		cout << "WARNING: called isuser() with stripped nick instead of raw_nick. returning false" << endl;
		return false;
	}
	
	if ((users_li.isinlist(cut_nick(nick))) && (users_li.get_info(cut_nick(nick))!=nick)) //still logged in, but wrong hostmask?
	{
		cout << "removing '"<<cut_nick(nick)<<"' from list of logged in users due to a hostmask mismatch" << endl;
		users_li.removefromlist(cut_nick(nick));
	}

	return (users_li.get_info(cut_nick(nick))==nick);
}

void TConnection::addmaster(string nick, string pass)
{
	masters.addtolist(nick, pass);
}

void TConnection::delmaster(string nick)
{
	masters.removefromlist(nick);
}

bool TConnection::ismaster(string nick)
{
	if (nick.find('!')==string::npos)
	{
		cout << "WARNING: called ismaster() with stripped nick instead of raw_nick. returning false" << endl;
		return false;
	}

	if ((masters_li.isinlist(cut_nick(nick))) && (masters_li.get_info(cut_nick(nick))!=nick)) //still logged in, but wrong hostmask?
	{
		cout << "removing '"<<cut_nick(nick)<<"' from list of logged in masters due to a hostmask mismatch" << endl;
		masters_li.removefromlist(cut_nick(nick));
	}

	return (masters_li.get_info(cut_nick(nick))==nick);
}

TUserList TConnection::get_masterlist()
{
	return masters;
}

TUserList TConnection::get_userlist()
{
	return users;
}

TUserList TConnection::get_channel_users (string chan)
{
	for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
		if (ucase((*it)->get_name()) == ucase(chan))
			return (*it)->get_users();
}

bool TConnection::isinchan(string nick,string chan)
{
	return (get_channel_users(chan).isinlist(nick));
}

ircmessage TConnection::get_curr_msg() { return curr_msg; }

bool TConnection::next_message()
{
	zerocurrmsg();
	string temp=getline();
	
	if (temp!="") cout << "@" << sname << ": " << temp << endl;
	
	if (!temp.empty())
	{
		interpret_message(parseline(temp));
		return true;
	}
	else
	{
		return false;
	}
}

void TConnection::check_session_timeouts()
{
	bool again=false;
	do
	{
		again=false;
		for (list<TSession*>::iterator it=sessions.begin(); it!=sessions.end(); it++)
			if (!(*it)->valid())
			{
				delete (*it); //das objekt hinterm iterator löschen
				sessions.erase(it); //und den pointer aus der liste entfernen
				again=true;
				break;
			}
	}
	while (again);
}

bool TConnection::valid()
{
	return (sock!=-1);
}

//string TConnection::get_ournick() {return nick;}

void TConnection::check_timeout()
{
	if ((time(NULL)>last_recv+60*5) && (!expecting_pong)) //es ist zeit für nen ping
	{
		send ("ping "+sname);  //ping
		last_ping=time(NULL);
		expecting_pong=true;
	}
	if ((time(NULL)>last_ping+30) && (expecting_pong)) //wir haben gepingt und keinen pong erhalten?
	{
		cout << "ping timeout." << endl;
		disconnect();
	}
}

string TConnection::get_networkname(string s_name)
{
	int pos;
	pos=s_name.rfind('.');
	if (pos!=string::npos)
		pos=s_name.rfind('.',pos-1);
	if (pos!=string::npos)
		return s_name.substr(pos+1);
	else
		return s_name;
}

string TConnection::get_networkname()
{
	return get_networkname(sname);
}

string TConnection::get_connectname()
{
	return connectname;
}

string TConnection::get_channel_topic(string chan)
{
	for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
		if (ucase((*it)->get_name()) == ucase(chan))
			return (*it)->get_topic();
}

string TConnection::get_channel_modes(string chan)
{
	for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
		if (ucase((*it)->get_name()) == ucase(chan))
			return (*it)->get_modes();
}

void TConnection::quit(string reason)
{
	send ("QUIT :"+reason);
	dontreconnect=true;
}

int TConnection::get_type() {return TYPE_CONN;}

void TConnection::communicate(string subject, void *data)
{
	deliver_message(subject, data);
	
	for (list<TChannel*>::iterator it=chans.begin(); it!=chans.end(); it++)
	  (*it)->deliver_message(subject, data);
	
	for (list<TSession*>::iterator it=sessions.begin(); it!=sessions.end(); it++)
	  (*it)->deliver_message(subject, data);
}

void TConnection::say (string what) //TODO: vlt an alle botmasters senden?
{
	//parent->send("PRIVMSG "+get_name()+" :"+what+NEWLINE);
	cout << "DEBUG: say doesn't work for TConnection!" << endl;
}
