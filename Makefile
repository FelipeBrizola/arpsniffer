all:
	gcc -o arpsniffer arpsniffer.c -Wall

clean:
	rm -f arpsniffer
