sv_node: classes.o header.o sv_node.o iniparser.o dictionary.o strlib.o store.o uihandler.o search.o get.o delete.o 
	g++ -o sv_node -lsocket -lnsl classes.o  header.o sv_node.o iniparser.o dictionary.o strlib.o store.o uihandler.o search.o get.o delete.o -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib -lcrypto -lsocket

header.o: header.h header.cpp
	g++ -c -Wall header.cpp -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib

sv_node.o: header.h sv_node.cpp
	g++ -c -Wall sv_node.cpp -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib

iniparser.o: iniparser.h iniparser.c
	g++ -c -Wall iniparser.h iniparser.c

dictionary.o: dictionary.h dictionary.c
	g++ -c -Wall dictionary.h dictionary.c

strlib.o: strlib.c
	g++ -c -Wall strlib.c strlib.h

store.o: header.h store.cpp
	g++ -c -Wall store.cpp -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib

uihandler.o: header.h uihandler.cpp
	g++ -c -Wall uihandler.cpp -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib

classes.o: header.h classes.cpp
	g++ -c -Wall classes.cpp -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib

search.o: header.h search.cpp
	g++ -c -Wall search.cpp -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib

get.o: header.h get.cpp
	g++ -c -Wall get.cpp -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib

delete.o: header.h delete.cpp
	g++ -c -Wall delete.cpp -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib

clean:
	rm -f *.o sv_node *.*~ *.h.gch