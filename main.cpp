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

#include <pwd.h>
#include <grp.h>

#include <cerrno>
using namespace std;


#include "myfuncs.h"
#include "TUserList.h"
#include "TPlugin.h"
#include "TSession.h"
#include "TChannel.h"
#include "TConnection.h"
#include "TConfig.h"
#include "main.h"

list<TPlugin*> plugins;	
TConfig config;
char default_comchar;


char comchar_filter(string cc, char fallback);



int main(int argc, char** argv)
{
	config.loadconfig("etc/drunkenman.conf"); //ignore syntax errors

	int conf_UID;
	if (!config.is_integer("UID"))
	{
		cout << "looking up UID for user '"<<config.get_string("UID")<<"'..."<< endl;
		struct passwd* pwd=getpwnam(config.get_string("UID").c_str());
		if (pwd)
		{
			conf_UID=pwd->pw_uid;
		}
		else
		{
			cout << "FATAL: failed to look up UID!"<<endl << "the program will exit now." << endl;
			exit(1);
		}
	}
	else
	{
		conf_UID=config.get_integer("UID");
	}
	
	int conf_GID;
	if (!config.is_integer("GID"))
	{
		cout << "looking up GID for group '"<<config.get_string("GID")<<"'..."<< endl;
		struct group* grp=getgrnam(config.get_string("GID").c_str());
		if (grp)
		{
			conf_GID=grp->gr_gid;
		}
		else
		{
			cout << "FATAL: failed to look up GID!"<<endl << "the program will exit now." << endl;
			exit(1);
		}
	}
	else
	{
		conf_GID=config.get_integer("GID");
	}
	
	//cout << "UID:" << conf_UID << ",  GID:"<<conf_GID<< endl;
	if (getgid()!=conf_GID)
	{
		if (setgid(conf_GID)!=0)
		{
			cout << "unable to set GID to " << conf_GID << endl;
			if (errno!=EPERM)
			{
				cout << "this is a fatal error" << endl <<"the program will exit now." << endl;
				exit(1);
			}
		}
	}

	if (getuid()!=conf_UID)
	{
		if (setuid(conf_UID)!=0)
		{
			cout << "unable to set UID to " << conf_UID << endl;
			if (errno!=EPERM)
			{
				cout << "this is a fatal error" << endl <<"the program will exit now." << endl;
				exit(1);
			}
		}
	}


	{
		struct passwd* pwd = getpwuid( getuid() );
		struct group* grp = getgrgid( getgid() );

		cout << "Running as user "<< (pwd?pwd->pw_name:"unknown") << "("<<getuid()<<"), group "<< (grp ? grp->gr_name : "unknown") << "("<<getgid()<<")"<<endl;
	}
	
	string conf_autoload=config.get_string("loadplugins");
	{
		string temp_conf_autoload=conf_autoload;
		string what;
		//while (getline(ifs,what))
		while ( (what=split(conf_autoload))!="")
		//if (what!="")
		{
			cout << "trying to load plugin '" << what << "'..." << endl;
			bool fail=false;
			
			list<TPlugin*>::iterator it;
			for (it=plugins.begin(); it!=plugins.end(); it++) //schauen, ob es schon geladen ist...
			{
				if (*it) //kein NULLpointer (sollte auch keiner sein, nur zur sicherheit...)
				{
					if (lcase((*it)->get_name())==lcase(what)) //schon geladen oO?
					{
						fail=true;
						break;
					}
				}
			}
			if (fail) // schon geladen!
			{
				cout << "ERROR: plugin '" << what << "' is already loaded!" << endl;
			}
			else // noch nicht geladen; wir können weitermachen										// L O A D   P L U G I N
			{
				TPlugin* tempptr;
				try //versuche, das plugin zu laden
				{
					tempptr=new TPlugin(what);
					plugins.push_back(tempptr);  //ab hier hatten wir erfolg!
					cout << "plugin '"<<what<< "' has been loaded successfully" << endl;
				}
				catch (...) //fehlgeschlagen?
				{
					cout << "ERROR: could not load plugin '" << what << "'!" << endl;
				}																					// done.
			}
		}
	}
	
	default_comchar=comchar_filter(config.get_string("comchar"),'!');
	
	list<TConnection*> connections;

	
	string conf_servers=config.get_string("connect");
	if (trim(conf_servers)=="")
	{
		cout << "no servers specified. exiting..." << endl;
		exit(1);
	}
	string temp_conf_servers=conf_servers;
	string temp_server, temp_net;
	while ((temp_server=split(temp_conf_servers))!="")
	{
		cout << "connecting to " << temp_server << "..." << endl;	
		temp_net=TConnection::get_networkname(temp_server);
		try
		{
			TConnection* temp=new TConnection(temp_server,config.get_valid_string(temp_net+".nick", "DrunkenMan")+" "+config.get_valid_string(temp_net+".altnicks", "DrunkenMan3 Drunken_Man"),comchar_filter(config.get_string(temp_net+".comchar"), default_comchar)); 
			connections.push_back (temp);
		}
		catch (...)
		{
			cout << "unable to connect to "<< temp_server <<"!" << endl;
		}
	}
	
	ircmessage msg;

	list<TConnection*>::iterator conn_it;
	TConnection* curr_conn;
	int cnt;

	while (true)
	{
		cnt=0;
		for (conn_it=connections.begin(); conn_it!=connections.end(); conn_it++)
		{
			cnt++;
			curr_conn=*conn_it;
			if (curr_conn->next_message()) 
			{
				msg=curr_conn->get_curr_msg();
				
				if (ucase(msg.command)=="PRIVMSG")			
				{
					if (msg.content[0]==curr_conn->comchar)
					{
						string cmd=msg.content.substr(1);
						if (match(cmd, "reload_config"))
						{
							if (curr_conn->ismaster(msg.origin_raw))
							{
								try
								{
									config.loadconfig("etc/drunkenman.conf");
									curr_conn->send("notice "+msg.origin+" :configuration reloaded.");
									
									default_comchar=comchar_filter(config.get_string("comchar"),'!');
									
									for (list<TConnection*>::iterator conn_it2=connections.begin(); conn_it2!=connections.end(); conn_it2++)
										(*conn_it2)->comchar=comchar_filter(config.get_string((*conn_it2)->get_networkname()+".comchar"), default_comchar);
								}
								catch (...)
								{
									curr_conn->send("notice "+msg.origin+" :reloading configuration failed!");
								}
							}
							else // !ismaster
							{
								curr_conn->send("notice "+msg.origin+" :sorry, but you aren't a master!");
							}
						}
						
						if (match(cmd, "reload_lists"))
						{
							if (curr_conn->ismaster(msg.origin_raw))
							{
								string temp_succ(""), temp_fail("");
								for (list<TConnection*>::iterator conn_it2=connections.begin(); conn_it2!=connections.end(); conn_it2++)
								{
									try
									{
										(*conn_it2)->loadlists();
										temp_succ=temp_succ+", "+(*conn_it2)->get_networkname();
									}
									catch (...)
									{
										temp_fail=temp_succ+", "+(*conn_it2)->get_networkname();
									}
								}
								if (temp_succ!="")
									curr_conn->send("notice "+msg.origin+" :successfully reloaded lists for "+temp_succ.substr(2));
								if (temp_fail!="")
									curr_conn->send("notice "+msg.origin+" :reloading lists failed for "+temp_fail.substr(2));
							}
							else // !ismaster
							{
								curr_conn->send("notice "+msg.origin+" :sorry, but you aren't a master!");
							}
						}
						
						if (match(cmd, "clear_logins"))
						{
							if (curr_conn->ismaster(msg.origin_raw))
							{
								string temp_succ(""), temp_fail("");
								for (list<TConnection*>::iterator conn_it2=connections.begin(); conn_it2!=connections.end(); conn_it2++)
									(*conn_it2)->logout_all();

								curr_conn->send("notice "+msg.origin+" :successfully cleared all login entries. note that also you have to re-login!");
							}
							else // !ismaster
							{
								curr_conn->send("notice "+msg.origin+" :sorry, but you aren't a master!");
							}
						}
						
						if (match(cmd,"connect"))
						{
							if (curr_conn->ismaster(msg.origin_raw))
							{
								//string server=msg.content.substr(9);
								string server=ntharg(cmd,2);
								curr_conn->send("notice "+msg.origin+" :trying to connect to "+server+"...");
								curr_conn->send("notice "+msg.origin+" :this may take a while. while connecting, the bot won't respond.");
								
								try
								{
									string temp_net=TConnection::get_networkname(server);
									TConnection* temp=new TConnection(server,config.get_valid_string(temp_net+".nick","DrunkenMan")+" "+config.get_valid_string(temp_net+".altnicks","DrunkenMan3 Drunken_Man"),comchar_filter(config.get_string(temp_net+".comchar"), default_comchar));
									connections.push_back (temp);
									curr_conn->send("notice "+msg.origin+" :connection to "+server+" was established successfully.");
								
								}
								catch (...)
								{
									curr_conn->send("notice "+msg.origin+" :connecting to "+server+" failed!");
								}
								
							}
							else // !ismaster
							{
								curr_conn->send("notice "+msg.origin+" :sorry, but you aren't a master!");
							}
						}
						
						if (match(cmd,"load"))
						{
							if (curr_conn->ismaster(msg.origin_raw))
							{
								//string what=msg.content.substr(6);
								string what=ntharg(cmd,2);
								cout << "trying to load plugin '" << what << "'..." << endl;
								bool fail=false;
								
								list<TPlugin*>::iterator it;
								for (it=plugins.begin(); it!=plugins.end(); it++) //schauen, ob es schon geladen ist...
								{
									if (*it) //kein NULLpointer (sollte auch keiner sein, nur zur sicherheit...)
									{
										if (lcase((*it)->get_name())==lcase(what)) //schon geladen oO?
										{
											fail=true;
											break;
										}
									}
								}
								if (fail) // schon geladen!
								{
									cout << "ERROR: plugin '" << what << "' is already loaded!" << endl;
									curr_conn->send("notice "+msg.origin+" :plugin '"+what+"' is already loaded!");
								}
								else // noch nicht geladen; wir können weitermachen										// L O A D   P L U G I N
								{
									TPlugin* tempptr;
									try //versuche, das plugin zu laden
									{
										tempptr=new TPlugin(what);
										plugins.push_back(tempptr);  //ab hier hatten wir erfolg!
										cout << "plugin '"<<what<< "' has been loaded successfully" << endl;
										curr_conn->send("notice "+msg.origin+" :plugin '"+what+"' has been loaded successfully.");
										
										curr_conn->addplugincontext(tempptr);
										
										//alle connections davon informieren, damit diese ihre contexts anpassen können
											//diese müssen auch alle ihrer channels informieren (selber zwecke)
									}
									catch (...) //fehlgeschlagen?
									{
										cout << "ERROR: could not load plugin '" << what << "'!" << endl;
										curr_conn->send("notice "+msg.origin+" :could not load plugin '"+what+"'!");
										curr_conn->send("notice "+msg.origin+" :see log for details");
									}																					// done.
								}
							}
							else // !ismaster
							{
								curr_conn->send("notice "+msg.origin+" :sorry, but you aren't a master!");
							}
						}
						if (match(cmd,"unload"))
						{
							if (curr_conn->ismaster(msg.origin_raw))
							{
								//string what=msg.content.substr(8);
								string what=ntharg(cmd,2);
								cout << "..." << endl;
								bool fail=true;
								//ggf. entladen
								list<TPlugin*>::iterator it;
								for (it=plugins.begin(); it!=plugins.end(); it++) //schauen, ob es schon geladen ist...
								{
									if (*it) //kein NULLpointer (sollte auch keiner sein, nur zur sicherheit...)
									{
										if (lcase((*it)->get_name())==lcase(what)) //schon geladen?
										{
											fail=false;
											break;
										}
									}
								}
								
								if (fail) //nicht geladen!
								{
									cout << "ERROR: plugin '" << what << "' is not loaded!" << endl;
									curr_conn->send("notice "+msg.origin+" :plugin '"+what+"' is not loaded!");
								}
								else //geladen :)?																		// U N L O A D   P L U G I N
								{
									cout << "removing plugincontext..." <<endl;
									curr_conn->removeplugincontext( distance(plugins.begin(),it) );
									delete *it;
									plugins.erase(it);
									curr_conn->send("notice "+msg.origin+" :plugin '"+what+"' has been unloaded successfully.");
								}																						// done.
							}
							else // !ismaster
							{
								curr_conn->send("notice "+msg.origin+" :sorry, but you aren't a master!");
							}
						}
					}
				}
			}
			
			curr_conn->check_timeout();
			if (!curr_conn->valid())
			{
				string temp=curr_conn->get_connectname();
				string temp_net=curr_conn->get_networkname();
				int nfails=0;
				bool retry=!(curr_conn->dontreconnect);
				cout << "ERROR: connection for '" << temp << "' has been closed!" << endl;
				delete curr_conn;
				curr_conn=0;
				
				if (retry)
					while (curr_conn==0)
					{
						cout << "trying to connect to '" << temp << "'... " << endl;
						try
						{
							curr_conn=new TConnection(temp, config.get_valid_string(temp_net+".nick", "DrunkenMan")+" "+config.get_valid_string(temp_net+".altnicks","DrunkenMan3 Drunken_Man"),comchar_filter(config.get_string(temp_net+".comchar"), default_comchar));
							cout << "success!" << endl;
							break;
						}
						catch(...)
						{
							cout << "failed." << endl;
							curr_conn=0;
							nfails++;
						}
						if (nfails>=3) break;
						usleep (5000000);
					}
					
				if (curr_conn==0)
				{
					if (retry)
						cout << "[ERROR]  unable to reconnect to '" << temp << "'!" << endl;
					else
						cout << "connection '" << temp << "' has been closed successfully." << endl;
					
					connections.erase(conn_it);
					break;
				}
				else
				{
					*conn_it=curr_conn;
				}
			}
			curr_conn->check_session_timeouts();
			curr_conn->exec_plugins(plugins);
		}
		
		if (cnt==0)
		{
			cout << "last connection has been closed :(" << endl;
			cout << "program will exit now." << endl << endl;
			exit (1);
		}
	}
}


char comchar_filter(string cc, char fallback)
{
	if (cc.length()!=1)
		return fallback;
	else
		return cc[0];
}
