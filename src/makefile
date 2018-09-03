# GCC version needs 4.7+(over 4.7)
#
OBJS    = html_element.o html_analysys.o test/assert.o test_main.o
CXX     = g++
CXXFLAGS  = -Wall $(DEBUG) -std=c++11 -pthread

test_main: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o test_main

clean:
	$(RM) $(OBJS)
	$(RM) test_main
