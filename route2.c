#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

/**
 * @Author Kaylin Zaroukian, Jerry, Cody Krueger
 * @Date 14 OCT 2018
 * CIS 457 Data Comm
 * Project 2
 *
 * References:
 */

int checksumCalculated(char *buffer, size_t len) {
  size_t i;
  size_t sum;
  for(i = 0; i < len; i++) {
    //sum += (int) buffer[i];
    sum += (unsigned int) buffer[i];
    // decides when to wrap
    if (sum & 0xFFFF0000) {
      sum &= 0xFFFF;
      sum++;
    }
  }
  // makes sure checksum is only 16 bytes
  //uint16_t finalSum = (uint16_t) sum;
  printf("Checksum prior to: %zu\n", sum);
  // gets 1s compliment
  return ~(sum & 0xFFFF);
}

int main() {
  // packet socket appears to be eth1
  int packet_socket;
  int lo_socket;
  int eth0_socket;
  int eth1_socket;
  u_char router_mac_addr[6];
  u_char router_mac_addr2[6];
  u_char router_mac_addr3[6];

  // set of sockets
  fd_set sockets;
  FD_ZERO(&sockets);

  struct icmp_header {
    u_int8_t type;
    u_int8_t code;
    u_int16_t checksum;
  };

  struct ifaddrs *ifaddr, *tmp;
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs");
    return 1;
  }
  //have the list, loop over the list
  int i = 0;
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    i++;

    if(tmp->ifa_addr->sa_family==AF_PACKET){
      printf("Interface: %s\n",tmp->ifa_name);

      // for eth1
      //create a packet socket on interface r?-eth1
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
        printf("Creating Socket on interface %s\n",tmp->ifa_name);

        struct sockaddr_ll *r_mac_addr = (struct sockaddr_ll *)tmp->ifa_addr;
        memcpy(router_mac_addr, r_mac_addr->sll_addr, 6);
        printf("%s\n", r_mac_addr);

        packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

        if(packet_socket<0){
          perror("socket");
          return 2;
        }

        if(bind(packet_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
          perror("bind");
        }
      }

      // eth 0
      if(!strncmp(&(tmp->ifa_name[3]),"eth0",4)){
        printf("Creating Socket on interface %s\n",tmp->ifa_name);

        eth0_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

        struct sockaddr_ll *r_mac_addr = (struct sockaddr_ll *)tmp->ifa_addr;
        memcpy(router_mac_addr2, r_mac_addr->sll_addr, 6);

        if(eth0_socket<0){
          perror("socket");
          return 2;
        }


        if(bind(eth0_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
          perror("bind");
        }
      }

          // eth2
          if(!strncmp(&(tmp->ifa_name[3]),"eth2",4)){
          printf("Creating Socket on interface %s\n",tmp->ifa_name);

          eth1_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

          struct sockaddr_ll *r_mac_addr = (struct sockaddr_ll *)tmp->ifa_addr;
          memcpy(router_mac_addr3, r_mac_addr->sll_addr, 6);

          if(eth1_socket<0){
            perror("socket");
            return 2;
          }

            if(bind(eth1_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
              perror("bind");
            }
          }
        }
      printf("Loop number: %d\n",i);
    }
  //
  FD_SET(packet_socket, &sockets);
  FD_SET(lo_socket, &sockets);
  FD_SET(eth0_socket, &sockets);
  FD_SET(eth1_socket, &sockets);

  printf("Ready to recieve now\n");
  while(1){
    printf("Entered while loop\n");
    fd_set tmp_set=sockets;
    select(FD_SETSIZE,&tmp_set,NULL,NULL,NULL);

    int i;
    int len;
    int sock;
    struct sockaddr_ll recvaddr;
    char buf[1500];
    int recvaddrlen;
    int n = 0;
    for(i=0;i<FD_SETSIZE;i++) {
      if(FD_ISSET(i,&tmp_set)) {
        len = sizeof(tmp);
        n = recvfrom(i, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);
        if(recvaddr.sll_pkttype==PACKET_OUTGOING)
          continue;
        //start processing all others
        printf("Got a %d byte packet\n", n);
        struct ether_header *eth_request  = (struct ether_header*)buf;
        printf("Ether type: %d\n", ntohs(eth_request->ether_type));

        // checks if packet is arp
        if (ntohs(eth_request->ether_type) == ETH_P_ARP) {
          printf("ARP packet\n");

          //struct arp_header *arp_request = (struct arp_header*)(buf+14);
          struct ether_arp *arp_request = (struct ether_arp*)(buf+14);

          char reply_data[1500];
          memcpy(reply_data,buf,1500);

          struct ether_header *eth_reply = (struct ether_header*)reply_data;
          //struct arp_header *arp_reply = (struct arp_header*)(reply_data+14);
          struct ether_arp *arp_reply = (struct ether_arp*)(reply_data+14);

          u_char our_mac[6];

          if(i == eth0_socket) {
            //our_mac = router_mac_addr2;
            memcpy(our_mac, router_mac_addr2, 6);
          }
          else if(i == eth1_socket) {
            memcpy(our_mac, router_mac_addr, 6);

          } else {
            memcpy(our_mac, router_mac_addr3, 6);
          }

          // populates ethernet header on ARP reply
          memcpy(&(eth_reply->ether_dhost),&(eth_request->ether_shost),6);
        //  memcpy(&(eth_reply->ether_shost), &(eth_request->ether_dhost),6);
          memcpy(&(eth_reply->ether_shost), &(our_mac),6);
          memcpy(&(eth_reply->ether_type), &(eth_request->ether_type), 6);
          printf("Ethernet Header is set up\n");

          // populates ARP header on ARP reply
          printf("Starting arp_reply\n");

          memcpy(&(arp_reply->ea_hdr), &(arp_request->ea_hdr), sizeof(arp_request->ea_hdr));
          //memcpy(&(arp_reply->arp_op), ARPOP_REPLY, sizeof(arp_request->ea_hdr.ea_hdr));
          arp_reply->arp_op = ntohs(ARPOP_REPLY);
          // this may be wrong but double check
          //arp_reply->ea_hdr.arp_op=ARPOP_REPLY;

          memcpy(&(arp_reply->arp_sha), our_mac, 6);
          memcpy(&(arp_reply->arp_spa), &(arp_request->arp_tpa), 4);
          memcpy(&(arp_reply->arp_tha), &(arp_request->arp_sha), 6);
          memcpy(&(arp_reply->arp_tpa), &(arp_request->arp_spa), 4);

          int x = send(i, reply_data, 42, 0);

          if (x < 0) {
            printf("ERROR sending ARP Reply!\n");
            perror("Error sending ARP reply");
            continue;
          }

          printf("ARP Reply packet sent\n");

        }
        if (ntohs(eth_request->ether_type) == ETHERTYPE_IP){
          // header for icmp request
          printf("ICMP Request\n");
          struct iphdr *ip_request = (struct iphdr*)(buf+sizeof(struct ether_header));
          u_short ip_len = ip_request->ihl * 4;
          // struct icmphdr *icmp_request = (struct icmphdr*)(buf + 34);
          struct icmp_header *icmp_request = (struct icmp_header*)(buf + 14 + ip_len);
          printf("Request Header created\n");
          printf("%d\n", sizeof(struct icmphdr));

          // replies for ICMP
          char reply_data[1514];
          memcpy(reply_data, buf, 1514);

          struct ether_header *eth_reply = (struct ether_header*)reply_data;
          struct iphdr *ip_reply = (struct iphdr*)(reply_data+sizeof(struct ether_header));
          struct icmp_header *icmp_reply = (struct icmp_header*)(reply_data + 14+20);

          printf("Reply header \n");

          //populates the ethernet header
          //memcpy(&(eth_reply),&(eth_request),20);
          memcpy(&(eth_reply->ether_dhost),&(eth_request->ether_shost),6);
          memcpy(&(eth_reply->ether_shost), &(eth_request->ether_dhost),6);
          memcpy(&(eth_reply->ether_type), &(eth_request->ether_type), 2);
          printf("Eth header\n");

          // populates the ip header
          memcpy(&(ip_reply->saddr), &(ip_request->daddr), 4);
          memcpy(&(ip_reply->daddr), &(ip_request->saddr), 4);
          printf("IP header\n");

          // populates the icmp headers
          icmp_reply->type = 0;
          icmp_reply->code = 0;
          memcpy(&(icmp_reply->checksum), &(ip_request->check), 2);
          printf("ICMP hdr\n");

          int x = send(i, reply_data, 98, 0);

          if (x < 0) {
            printf("ERROR sending ICMP ECHO Reply!\n");
            perror("Error sending ICMP ECHO reply");
            continue;
          }

          printf("ICMP Echo Reply packet sent\n");

        }

      }
    }
    printf("Exiting for loop\n");
    FD_CLR(i,&sockets);

  }
  //free the interface list when we don't need it anymore
  printf("Exiting routers\n");
  freeifaddrs(ifaddr);
  //exit
  return 0;
}
