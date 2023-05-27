# Variables del Makefile
CC = g++
CFLAGS = -std=c++11 -Wall -g
LDFLAGS = -pthread

# Archivos fuente
SERVER_SOURCE = server.cpp
CLIENT_SOURCE = client.cpp

# Nombres de los ejecutables
SERVER_EXECUTABLE = servidor
CLIENT_EXECUTABLE = cliente

all: $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE)

$(SERVER_EXECUTABLE): $(SERVER_SOURCE)
	$(CC) $(CFLAGS) $(SERVER_SOURCE) -o $(SERVER_EXECUTABLE) $(LDFLAGS)

$(CLIENT_EXECUTABLE): $(CLIENT_SOURCE)
	$(CC) $(CFLAGS) $(CLIENT_SOURCE) -o $(CLIENT_EXECUTABLE) $(LDFLAGS)

clean:
	rm -f $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE)



