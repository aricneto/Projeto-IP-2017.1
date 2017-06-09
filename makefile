serverFile=main/gameServer
clientFile=main/gameClient
serverName=gameServer
clientName=gameClient

all: compServer compClient

server: compServer runServer

client: compClient runClient

compClient:
	gcc -o gameClient main/gameClient.c lib/client.c

compServer:
	gcc -o gameServer main/gameServer.c lib/server.c

runClient:
	./$(clientName)

runServer:
	./$(serverName)