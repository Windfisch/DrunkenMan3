CXX=g++
CFLAGS=-Wall
CXXFLAGS=$(CFLAGS)
LDFLAGS=-ldl

OBJ=main.o TChannel.o TConnection.o TPluginParent.o TUserList.o myfuncs.o TConfig.o TPlugin.o TSession.o
BIN=drunkenman

DEPENDFILE = .depend


SRC = $(OBJ:%.o=%.cpp)

all: $(BIN)


$(BIN): $(OBJ)
	$(CXX) $(CFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	

depend dep: $(SRC)
	$(CC) -MM $(SRC) > $(DEPENDFILE)

-include $(DEPENDFILE)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<


.PHONY: clean dep depend

clean:
	rm -f $(OBJ) $(BIN)
	


