all:
	g++ -o server server.cpp ../shared/client.cpp ../shared/client.h ../shared/utils.cpp ../shared/utils.h -lpthread -luuid

clean:
	$(RM) server
	$(RM) client.h.gch
	$(RM) utils.h.gch
	$(RM) commands.h.gch
