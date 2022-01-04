CC   = g++
LD   = g++

.PHONY: all clean

all: DS user

DS: DS.o

user: user.o

DS.o: ServerUDP.cpp ServerUDP.hpp
	$(CC) $(CFLAGS) -o DS.o -c ServerUDP.cpp

user.o: user.cpp user.hpp
	$(CC) $(CFLAGS) -o user.o -c user.cpp

clean:
	@echo Cleaning...
	rm -f fs/*.o *.o user DS *.txt

rmusers:
	@echo Cleaning Users...
		rm -r USERS

rmgroups:
	@echo Cleaning Groups...
		rm -r GROUPS