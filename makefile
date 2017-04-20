main:
	gcc -Wall -pthread -O2 -o bin/hitter hitter.c

clean:
	rm -f hitter 
