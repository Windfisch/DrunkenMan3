#ifndef _MYTYPES_H_
#define _MYTYPES_H_
#define NEWLINE "\r\n"
#define BUFLEN 1024

#define PFLAGS_EXEC_ALWAYS 1
#define PFLAGS_EXEC_ONEVENT 2
#define PFLAGS_EXEC_ONDEMAND 4
#define PFLAGS_EXEC_ONANYEVENT 8
#define PFLAGS_EXEC_ONCREATE 16
#define PFLAGS_EXEC_ONREMOVE 32
#define PFLAGS_EXEC_ONDESTROY 64
#define PFLAGS_RECV_MESSAGES 128

#define TYPE_CONN 0
#define TYPE_CHAN 1
#define TYPE_SESS 2

//#include "TPluginParent.h"
class TPluginParent;
//class TConfigReadOnly;
#include "TConfigReadOnly.h"

struct plugincontext
{
	int flags; 	//when it is executed, etc
	void* data; //pointer to an area in the RAM where the plugin can write to
};

struct ircmessage
{
	string origin;
	string origin_raw;
	string command;
	string content;
	string params;
	
	bool operator! () { return ((origin=="")&&(command=="")&&(content=="")&&(params=="")); }
	operator bool ()  { return ((origin!="")||(command!="")||(content!="")||(params!="")); }
};

const ircmessage NULLMSG;

typedef void (*pluginfunc)(void* context, ircmessage msg, TPluginParent* parent, TConfigReadOnly& config, int reason);
typedef void (*pluginrecvfunc)(plugincontext* context, string subject, void* data, TPluginParent* parent, TConfigReadOnly& config);
typedef void (*plugininitfunc)(int* csize, int* conndefault, int* chandefault, int* sessdefault);

#endif
