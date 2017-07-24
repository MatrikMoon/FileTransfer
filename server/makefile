all:
	g++ -o server server.cpp ../shared/client.cpp ../shared/client.h ../shared/fileTransfer.cpp ../shared/fileTransfer.h ../shared/utils.cpp ../shared/utils.h -lpthread -luuid -lssl -lcrypto -std=c++11 -Wformat=0

clean:
	$(RM) server
	$(RM) client.h.gch
	$(RM) utils.h.gch
	$(RM) commands.h.gch
