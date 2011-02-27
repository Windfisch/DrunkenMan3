#include <ctime>
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

#include "TPluginParent.h"
#include "TConnection.h"

extern list<TPlugin*> plugins;

void TPluginParent::hiddenaddplugincontext(int flags,int csize)
{
	cout << "adding a new plugincontext..." << endl;
	
	plugincontext temp;
	char* cptr;
	temp.flags=flags;
	
	if (csize>0)
		try
		{
			cptr=new char[csize];
			for (int i=0; i<csize; i++) cptr[i]=0; //context nullen
		}
		catch (...)
		{
			cout << "PANIC: unable to create context data!" << endl; //todo: reaktion
			cout << "HINT:  this should   N E V E R   happen! you don't have even "<<csize<<"bytes RAM free!" << endl;
			cout << "       consider rebooting your machine as soon as possible!" << endl;
			cptr=NULL;
		}
	else
		cptr=NULL;
		
	temp.data=static_cast<void*> (cptr);
	
	contexts.push_back(temp); //pushen... und fertig :)	
}
		

void TPluginParent::removeplugincontext(int x)
{
	cout << "removing " << x << "th plugincontext" << endl;
	list<plugincontext>::iterator temp;
	temp=contexts.begin();
	advance(temp,x);
	
	list <TPlugin*>::iterator it=plugins.begin();
	advance(it,x);
	ircmessage curr_msg;
	curr_msg.origin=""; curr_msg.content=""; curr_msg.command=""; curr_msg.params="";
	if (temp->flags & PFLAGS_EXEC_ONREMOVE)
		(*it)->execute( &(*temp) , curr_msg, this, PFLAGS_EXEC_ONREMOVE);
	
	if (temp->data)
		delete [] static_cast<char*> (temp->data);
	contexts.erase(temp);
}

void TPluginParent::zerocurrmsg()
{
	//curr_msg.origin=""; curr_msg.content=""; curr_msg.params=""; curr_msg.command="";
	msg_for_us=false;
}

TPluginParent::~TPluginParent()
{
	cout << "destroying all contexts for object..." << endl;
	list<TPlugin*>::iterator it2=plugins.begin();
	
	for (list<plugincontext>::iterator it=contexts.begin(); it!=contexts.end(); it++)
	{
		if (it->data)
			delete [] static_cast<char*> (it->data);
			
		it2++;
	}
}

void TPluginParent::deliver_message(string subject, void *data)
{
	list<TPlugin*>::iterator it2=plugins.begin();
	for (list<plugincontext>::iterator it=contexts.begin(); it!=contexts.end(); it++)
	{
		if (it->flags & PFLAGS_RECV_MESSAGES)
			(*it2)->push_message( &(*it) , subject, data, this);
			
		it2++;
	}
}
