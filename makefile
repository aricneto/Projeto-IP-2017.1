serverFile=main/gameServer
clientFile=main/gameClient
serverName=gameServer
clientName=gameClient

all: compServer compClient

server: compServer runServer

client: compClient runClient

test: compTest runTest

compClient:
	gcc -o gameClient main/gameClient.c lib/client.c -lncurses

compTest:
	gcc -o gameClientTest main/gameClient.c lib/client.c -lncurses

compServer:
	gcc -o gameServer main/gameServer.c lib/server.c -lncurses -pthread

runClient:
	./$(clientName)

runTest:
	./$(clientName)Test

runServer:
	./$(serverName)