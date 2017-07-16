all:
	g++ -o client client.cpp server.cpp utils.cpp commands.cpp client.h commands.h utils.h server.h -lpthread -lX11

clean:
	$(RM) client
	$(RM) client.g.gch
	$(RM) utils.h.gch
	$(RM) server.h.gch
	$(RM) commands.h.gch
