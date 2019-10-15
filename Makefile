#######################
# Makefile
#######################

# source object target
SOURCE := RemoteTcpServer.cpp 
TARGET := RemoteTcpServer.out
OBJS := RemoteTcpServer.o CRemoteTcpServer.o

# compile and lib parameter
CC      := g++
LDFLAGS := -L/usr/local/lib
#LIBS    := -lmodbus -pthread
DEFINES :=
#INCLUDE := -I/usr/local/include/modbus
CFLAGS  := 
CXXFLAGS:= 

# link
$(TARGET):$(OBJS)
	@$(CC) -std=c++11 -g $(CFLAGS) $^ $(LDFLAGS) $(LIBS)  -o $@

# compile
RemoteTcpServer.o:RemoteTcpServer.cpp CRemoteTcpServer.o
	@$(CC) -std=c++11 -c -g $(CFLAGS) RemoteTcpServer.cpp -o RemoteTcpServer.o
CRemoteTcpServer.o:CRemoteTcpServer.h CRemoteTcpServer.cpp
	@$(CC) -std=c++11 -c -g $(CFLAGS) CRemoteTcpServer.cpp -o CRemoteTcpServer.o

.PHONY:clean
clean:
	@rm main.out $(OBJS) 

