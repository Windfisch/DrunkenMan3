all:
	for i in *.cpp; do make $${i%.cpp}.so; done

%.so: %.cpp
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ ../myfuncs.cpp 

ops.so: ../TUserList.o

clean:
	rm *.so *.o

.PHONY: clean all
