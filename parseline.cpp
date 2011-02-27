//      parseline.cpp
//      
//      Copyright 2010 Unknown <flo@arch>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include <iostream>
#include <string>
#include "mytypes.h"


ircmessage parseline (string line)
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
	//temp.origin=cut_nick(temp.origin);
	
	
	return temp;
}


int main(int argc, char** argv)
{
  char foo[1000];
  string l;
  ircmessage m;
	cin.getline(foo, sizeof(foo));
  l=foo;
//  cout << l;
  m=parseline(l);
  cout << "origin:  " << m.origin << endl;
  cout << "command: " << m.command << endl;
  cout << "content: " << m.content << endl;
  cout << "params:  " << m.params << endl;
  
	return 0;
}
