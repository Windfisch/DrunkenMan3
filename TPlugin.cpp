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

using namespace std;
#include "TPlugin.h"
#include "main.h"

#include "TConfig.h"
#include "TConfigReadOnly.h"

TPlugin::TPlugin(string pluginname)
{
	string path;
	plugininitfunc initfunc;
	
	path="./plugins/"+lcase(pluginname)+".so";
	handle=dlopen(path.c_str(),RTLD_NOW);
	if (!handle)
	{
		cout << "file '" << "./plugins/"+lcase(pluginname)+".so" << "' not found. " << dlerror() << endl;
		throw 1;
	}
	
	func=(pluginfunc)dlsym(handle, "plugin");
	if (func==NULL)
	{
		cout << "symbol 'plugin' not found."<< endl;
		dlclose(handle); handle=0;
		throw 2;
	}
		
	recv_msg=(pluginrecvfunc)dlsym(handle, "recv_message");
	if (recv_msg==NULL)
	{
		cout << "symbol 'recv_message' not found. this is not an error."<< endl;
	}
		
	initfunc=(plugininitfunc)dlsym(handle,"init");
	if (initfunc==NULL)
	{
		cout << "symbol 'init' not found." << endl;
		dlclose(handle); handle=0;
		throw 3;
	}
		
	(*initfunc) (&context_size, &default_flags_for_connections,&default_flags_for_channels,&default_flags_for_sessions);
	
	name=pluginname;
}

TPlugin::~TPlugin()
{
	if (handle) 
	{
		cout << "unloading '"<<name<<"'..." << endl;
		dlclose(handle);
	}
}

void TPlugin::execute(plugincontext* context,ircmessage msg,TPluginParent* parent, int reason)
{
	(*func) (context, msg, parent, config, reason);
}

void TPlugin::push_message(plugincontext* context, string subject, void* data, TPluginParent* parent)
{
	if (recv_msg)
		(*recv_msg) (context, subject, data, parent, config);
	else
		cout << "WARNING: tried to call a recv_msg for plugin '"<<name<<"', but it's not defined!" << endl;
}

int TPlugin::get_context_size(){return context_size;}
int TPlugin::get_default_flags_for_connections() {return default_flags_for_connections;}
int TPlugin::get_default_flags_for_channels() {return default_flags_for_channels;}
int TPlugin::get_default_flags_for_sessions() {return default_flags_for_sessions;}
string TPlugin::get_name(){return name;}
