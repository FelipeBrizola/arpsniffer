all:
	gcc -o raw_eth_recv raw_eth_recv.c -Wall
	gcc -o raw_eth_send raw_eth_send.c -Wall

clean:
	rm -f raw_eth_recv raw_eth_send
