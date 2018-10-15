#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

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

// may want to use all u_shorts
  struct arp_header{
    u_short mac_addr;
    u_short ip_addr;
    u_char mac_addr_len;
    u_char ip_addr_len;
    u_short op;
    u_char src_mac[6];
    u_char src_ip[4];
    u_char dst_mac[6];
    u_char dst_ip[4];
  };

  // set of sockets
  fd_set sockets;
  FD_ZERO(&sockets);

  //get list of interface addresses. This is a linked list. Next
  //pointer is in ifa_next, interface name is in ifa_name, address is
  //in ifa_addr. You will have multiple entries in the list with the
  //same name, if the same interface has multiple addresses. This is
  //common since most interfaces will have a MAC, IPv4, and IPv6
  //address. You can use the names to match up which IPv4 address goes
  //with which MAC address.
  struct ifaddrs *ifaddr, *tmp;
  if(getifaddrs(&ifaddr)==-1){
    perror("getifaddrs");
    return 1;
  }
  //have the list, loop over the list
  int i = 0;
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    i++;
    //Check if this is a packet address, there will be one per
    //interface.  There are IPv4 and IPv6 as well, but we don't care
    //about those for the purpose of enumerating interfaces. We can
    //use the AF_INET addresses in this list for example to get a list
    //of our own IP addresses
    if(tmp->ifa_addr->sa_family==AF_PACKET){
      printf("Interface: %s\n",tmp->ifa_name);

      // for eth1
      //create a packet socket on interface r?-eth1
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
        printf("Creating Socket on interface %s\n",tmp->ifa_name);

        //create a packet socket
        //AF_PACKET makes it a packet socket
        //SOCK_RAW makes it so we get the entire packet
        //could also use SOCK_DGRAM to cut off link layer header
        //ETH_P_ALL indicates we want all (upper layer) protocols
        //we could specify just a specific one

        struct sockaddr_ll *r_mac_addr = (struct sockaddr_ll *)tmp->ifa_addr;
        memcpy(router_mac_addr, r_mac_addr->sll_addr, 6);

        packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

        if(packet_socket<0){
          perror("socket");
          return 2;
        }

        //Bind the socket to the address, so we only get packets
        //recieved on this specific interface. For packet sockets, the
        //address structure is a struct sockaddr_ll (see the man page
        //for "packet"), but of course bind takes a struct sockaddr.
        //Here, we can use the sockaddr we got from getifaddrs (which
        //we could convert to sockaddr_ll if we needed to)
        if(bind(packet_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
          perror("bind");
        }
      }

      // for lo
      if(!strncmp(&(tmp->ifa_name[2]),"lo",2)){
        printf("Creating Socket on interface %s\n",tmp->ifa_name);

        //create a packet socket
        //AF_PACKET makes it a packet socket
        //SOCK_RAW makes it so we get the entire packet
        //could also use SOCK_DGRAM to cut off link layer header
        //ETH_P_ALL indicates we want all (upper layer) protocols
        //we could specify just a specific one
        lo_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

        if(lo_socket<0){
          perror("socket");
          return 2;
        }

        //Bind the socket to the address, so we only get packets
        //recieved on this specific interface. For packet sockets, the
        //address structure is a struct sockaddr_ll (see the man page
        //for "packet"), but of course bind takes a struct sockaddr.
        //Here, we can use the sockaddr we got from getifaddrs (which
        //we could convert to sockaddr_ll if we needed to)
        if(bind(lo_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
          perror("bind");
        }
      }

      // eth 0
      if(!strncmp(&(tmp->ifa_name[3]),"eth0",4)){
        printf("Creating Socket on interface %s\n",tmp->ifa_name);

        //create a packet socket
        //AF_PACKET makes it a packet socket
        //SOCK_RAW makes it so we get the entire packet
        //could also use SOCK_DGRAM to cut off link layer header
        //ETH_P_ALL indicates we want all (upper layer) protocols
        //we could specify just a specific one
        eth0_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

        if(eth0_socket<0){
          perror("socket");
          return 2;
        }

        //Bind the socket to the address, so we only get packets
        //recieved on this specific interface. For packet sockets, the
        //address structure is a struct sockaddr_ll (see the man page
        //for "packet"), but of course bind takes a struct sockaddr.
        //Here, we can use the sockaddr we got from getifaddrs (which
        //we could convert to sockaddr_ll if we needed to)
        if(bind(eth0_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
          perror("bind");
        }
      }

          // eth2
          if(!strncmp(&(tmp->ifa_name[3]),"eth2",4)){
          printf("Creating Socket on interface %s\n",tmp->ifa_name);

          //create a packet socket
          //AF_PACKET makes it a packet socket
          //SOCK_RAW makes it so we get the entire packet
          //could also use SOCK_DGRAM to cut off link layer header
          //ETH_P_ALL indicates we want all (upper layer) protocols
          //we could specify just a specific one
          eth1_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

          if(eth1_socket<0){
            perror("socket");
            return 2;
          }

            //Bind the socket to the address, so we only get packets
            //recieved on this specific interface. For packet sockets, the
            //address structure is a struct sockaddr_ll (see the man page
            //for "packet"), but of course bind takes a struct sockaddr.
            //Here, we can use the sockaddr we got from getifaddrs (which
            //we could convert to sockaddr_ll if we needed to)
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
  //
  // typedef struct {
  //   uint16_t type;
  //   uint16_t code;
  //   uint16_t checksum;
  // } icmp_header;


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

        // header for arp
        // not sure if we need both of these
        // struct ether_arp *arp_hdr;
        // arp_hdr = (struct ether_arp*)(buf + 14);

        // we are receiving all arp packets
        // checks to see which packets are arp
        struct ether_header *eth_request  = (struct ether_header*)buf;

        if (ntohs(eth_request->ether_type) == ETH_P_ARP) {
          printf("ARP packet\n");

          struct arp_header *arp_request = (struct arp_header*)(buf+14);

          char reply_data[1500];

          struct ether_header *eth_reply = (struct ether_header*)reply_data;
          struct arp_header *arp_reply = (struct arp_header*)(reply_data+14);

          // populates ethernet header on ARP reply
          memcpy(&(eth_reply->ether_dhost),&(eth_request->ether_shost),6);
          memcpy(&(eth_reply->ether_shost), &(eth_request->ether_dhost),6);
          memcpy(&(eth_reply->ether_type), &(eth_request->ether_type), 6);
          printf("Ethernet Header is set up\n");

          // populates ARP header on ARP reply
          printf("Starting arp_reply\n");
          arp_reply->mac_addr = arp_request->mac_addr;
          arp_reply->ip_addr = arp_request->ip_addr;
          printf("continuing arp reply\n");
          // should be 6
          arp_reply->mac_addr_len = arp_request->mac_addr_len;
          // should be 4
          arp_reply->ip_addr_len = arp_request->ip_addr_len;
          // should be 2
          arp_reply->op = htons(2);
          printf("continuing arp reply, 1st 5 fields established\n");


          // sets source mac address
          int j = 0;
          int k = 0;
          // establishes src and dst mac addresses
          while(j < 6) {
            arp_reply->src_mac[i] = router_mac_addr[i];
            arp_reply->dst_mac[i] = arp_request->src_mac[i];
            j++;
            printf("continuing arp reply, mac addr\n");
          }

          while(k < 4){
            arp_reply->src_ip[i] = arp_request->dst_ip[i];
            arp_reply->dst_ip[i] = arp_request->src_ip[i];
            printf("continuing arp reply, ip addr\n");
            k++;

          }
          printf("Arp header established\n");

          int x = send(i, reply_data, 42, 0);

          if (x < 0) {
            printf("ERROR sending ARP Reply!\n");
            perror("Error sending ARP reply");
            continue;
          }

          printf("ARP Reply packet sent\n");

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
