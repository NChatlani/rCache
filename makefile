all:
	
	gcc -c -fpic rcache_client_lib.c -Wall -Werror
	gcc -shared -o librcache.so rcache_client_lib.o
	gcc -L. -o client rcache_client.c -Wall -Werror -lrcache
	
	gcc -Wall -o rcache_server rcache_server.c -l pthread
	
	