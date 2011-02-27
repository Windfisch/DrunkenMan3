#include <iostream>
#include "TConfig.h"

using namespace std;

int main()
{
  TConfig config;

  config.loadconfig("test.conf");
  
  cout << config.get_valid_string("foo.bar.baz.str","default") << endl;
  cout << config.get_valid_integer ("foo.bar.baz.int",1337) << endl;
  cout << config.get_valid_boolean("foo.bar.baz.bool",false) << endl;
}
