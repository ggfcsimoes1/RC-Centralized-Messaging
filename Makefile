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

clean: #Removes .o files and output files from testing
	@echo Cleaning...
	rm -f fs/*.o *.o user DS *.txt

rmusers: #Removes "USERS" folder
	@echo Cleaning Users...
		rm -r USERS

rmgroups: #Removes "GROUPS" folder
	@echo Cleaning Groups...
		rm -r GROUPS

builddir:
	@echo Rebuilding dir...
	./user < SCR/script_06.txt
	./user < SCR/script_09.txt
	./user < SCR/script_14.txt