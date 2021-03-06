CC = gcc -std=c99
CCFLAGS = -Wall -O3

SERVER= server
CLIENT= client

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS)
	$(CC) -o $(SERVER) $(CCFLAGS)  $(SERVER).c -lpthread
	$(CC) -o $(CLIENT) $(CCFLAGS)  $(CLIENT).c -lpthread

$(GIT_HOOKS):
	scripts/install-git-hooks

debug: $(GIT_HOOKS)
	$(CC) -o $(SERVER) $(CCFLAGS) -g $(SERVER).c -lpthread
	$(CC) -o $(CLIENT) $(CCFLAGS) -g $(CLIENT).c -lpthread

server:
	$(CC) -p $(SERVER) $(CCFLAGS) -g $(SERVER).c -lpthread

client:
	$(CC) -o $(CLIENT) $(CCFLAGS) -g $(CLIENT).c -lpthread

clean:
	rm -rf $(SERVER) $(CLIENT)
