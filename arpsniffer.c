#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

#define BUFFER_SIZE 1600
#define ETHERTYPE 0x0806
#define ETHERNET_HEADER_SIZE 14 // bytes
#define ETHERNET_PADDING_SIZE 18 // bytes

int main(int argc, char *argv[])
{
	int fd;
	unsigned char buffer[BUFFER_SIZE];
	struct ifreq ifr;
	char ifname[IFNAMSIZ];

	if (argc != 2) {
		printf("Usage: %s iface\n", argv[0]);
		return 1;
	}
	strcpy(ifname, argv[1]);

	/* Cria um descritor de socket do tipo RAW */
	fd = socket(PF_PACKET,SOCK_RAW, htons(ETH_P_ALL));
	if(fd < 0) {
		perror("socket");
		exit(1);
	}

	/* Obtem o indice da interface de rede */
	strcpy(ifr.ifr_name, ifname);
	if(ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
		perror("ioctl");
		exit(1);
	}

	/* Obtem as flags da interface */
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0){
		perror("ioctl");
		exit(1);
	}

	/* Coloca a interface em modo promiscuo */
	ifr.ifr_flags |= IFF_PROMISC;
	if(ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
		perror("ioctl");
		exit(1);
	}

	printf("Esperando pacotes ... \n");
	while (1) {
		unsigned char sender_mac[6];
		unsigned char target_mac[6];
		unsigned char sender_ip[4];
		unsigned char target_ip[4];

		unsigned char hdw_type[2];
		unsigned char protocol_type[2];
		unsigned char hdw_size[1];
		unsigned char protocol_size[1];
		unsigned char opcode[2];

		short int ethertype;

		int offset = 0;

		/* Recebe pacotes */
		if (recv(fd,(char *) &buffer, BUFFER_SIZE, 0) < 0) {
			perror("recv");
			close(fd);
			exit(1);
		}

		int arp_buffer_size = sizeof(buffer) / 8 - (ETHERNET_HEADER_SIZE + ETHERNET_PADDING_SIZE);		

		unsigned char arp_buffer[arp_buffer_size];

		memset(arp_buffer, 0, sizeof(arp_buffer));

		// obtem apenas arp
		memcpy(arp_buffer, buffer + ETHERNET_HEADER_SIZE, arp_buffer_size );

		memcpy(&ethertype, buffer+sizeof(sender_mac)+sizeof(target_mac), sizeof(ethertype));
		ethertype = ntohs(ethertype);

		if (ethertype == ETHERTYPE) {		

			memcpy(hdw_type, arp_buffer, sizeof(hdw_type));
			offset += sizeof(hdw_type);

			memcpy(protocol_type, arp_buffer + offset, sizeof(protocol_type));
			offset += sizeof(protocol_type);

			memcpy(hdw_size, arp_buffer + offset, sizeof(hdw_size));
			offset += sizeof(hdw_size);

			memcpy(protocol_size, arp_buffer + offset, sizeof(protocol_size));
			offset += sizeof(protocol_size);

			memcpy(opcode, arp_buffer + offset, sizeof(opcode));
			offset += sizeof(opcode);

			memcpy(sender_mac, arp_buffer + offset, sizeof(sender_mac));
			offset +=  sizeof(sender_mac);

			memcpy(sender_ip, arp_buffer + offset, sizeof(sender_ip));
			offset += sizeof(sender_ip);

			memcpy(target_mac, arp_buffer + offset, sizeof(target_mac));
			offset += sizeof(target_mac);

			memcpy(target_ip, arp_buffer + offset, sizeof(target_ip));


			printf("HARDWARE TYPE:  %02x %02x\n", hdw_type[0], hdw_type[1]);
			printf("PROTOCOL TYPE:  %02x %02x\n", protocol_type[0], protocol_type[1]);
			printf("HARDWARE SIZE:  %02x\n", hdw_size[0]);
			printf("PROTOCOL SIZE:  %02x\n", protocol_size[0]);
			printf("OPCODE:  %02x %02x\n", opcode[0], opcode[1]);

			printf("SENDER MAC:  %02x:%02x:%02x:%02x:%02x:%02x\n",sender_mac[0], sender_mac[1], sender_mac[2], sender_mac[3], sender_mac[4], sender_mac[5]);
			printf("SENDER IP: %03d.%03d.%03d.%03d\n",sender_ip[0], sender_ip[1], sender_ip[2], sender_ip[3]);

			printf("TARGET MAC:  %02x:%02x:%02x:%02x:%02x:%02x\n",target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5]);
			printf("TARGET IP: %03d.%03d.%03d.%03d\n",target_ip[0], target_ip[1], target_ip[2], target_ip[3]);


			printf("\n");

		}

	}

	close(fd);
	return 0;
}
