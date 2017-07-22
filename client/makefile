all:
	g++ -o client client.cpp client.h commands.cpp commands.h ../shared/server.cpp ../shared/server.h ../shared/utils.cpp ../shared/utils.h ../shared/fileTransfer.cpp ../shared/fileTransfer.h -lpthread -lX11 -luuid -lssl -lcrypto

clean:
	$(RM) client
	$(RM) client.g.gch
	$(RM) utils.h.gch
	$(RM) server.h.gch
	$(RM) commands.h.gch