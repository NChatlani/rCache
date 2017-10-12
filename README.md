# rCache

rCache is a class project completed for CMS 330 (System Software Principles). It is an implementation of a remote In-Memory caching service, 
similar to the system *memcached*. rCache consists of a client-server architecture that services a hash table. The server is multithreaded
and runs a pool of worker threads that process hash table operations. A buffer accepts new connections and delivers them to the worker
threads.
