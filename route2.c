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
#include <stdlib.h>
#include <string.h>

/**
 * @Author Kaylin Zaroukian, Jerry, Cody Krueger
 * @Date 14 OCT 2018
 * CIS 457 Data Comm
 * Project 2
 *
 * References:
 */

 u_short cksum(u_short* buf, int count) {
   register u_long sum = 0;
   while (count--) {
     sum += *buf++;
     if (sum & 0xFFFF0000) {
       // carry occured so wrap around
       sum &= 0xFFFF;
       sum++;
     }
   }
   return ~(sum & 0xFFFF);
 }

int main() {
  // packet socket appears to be eth1

  //TODO fix this later
  int f = 0;
  int packet_socket;
  int lo_socket;
  int eth0_socket;
  int eth1_socket;
  int eth3_socket;
  int packet_num = 0;

  // create timeout for arp replys
   struct timeval timeout;
   timeout.tv_sec = 1;
   timeout.tv_usec = 0;

  struct interface_mac_addresses {
    u_char router_mac_addr[10][6];
    struct in_addr router_ip_addr[10];
    char int_name[10][10];
    int total_sockets;
    u_int file_descriptors[10];
  };

  // queue of possible packets
  char packet_queue[15][1500];

  //struct
  // inital val
 FILE *file = fopen("r1-table.txt","r");

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

  struct routingTable{
    struct in_addr first_ip[10];
    //char prefix[10][2];
    int prefix[10];
    int prefix_bytes[10];
    struct in_addr second_ip[10];
    char name[10][10];
    u_int8_t table_length;
	};

  struct routingTable *table = (struct routingTable*)malloc(sizeof(struct routingTable));
  struct interface_mac_addresses *mac_addresses = (struct interface_mac_addresses*)malloc(sizeof(struct interface_mac_addresses));

  //have the list, loop over the list
  int i = 0;
  int r = 0;
  for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
    mac_addresses->total_sockets=i;


    if(strncmp(tmp->ifa_name, "r1",2) == 0) {
      //printf("Router ONE\n");
      file = fopen("r1-table.txt", "r");
      table->table_length = 4;
    }

    if(strncmp(tmp->ifa_name, "r2",2) == 0) {
      //printf("Router TWO\n");
      file = fopen("r2-table.txt", "r");
      table->table_length = 5;
    }

    if (tmp->ifa_addr->sa_family==AF_INET){
      struct sockaddr_in *r_ip_addr = (struct sockaddr_in *)tmp->ifa_addr;
      mac_addresses->router_ip_addr[r].s_addr = r_ip_addr->sin_addr.s_addr;
      //printf("IP From struct %lu\n", r_ip_addr->sin_addr.s_addr);
      //printf("actual IP %s\n", inet_ntoa(r_ip_addr->sin_addr));
      r++;

    }

    if(tmp->ifa_addr->sa_family==AF_PACKET){
      //printf("Interface: %s\n",tmp->ifa_name);

      // for eth1
      //create a packet socket on interface r?-eth1
      if(!strncmp(&(tmp->ifa_name[3]),"eth1",4)){
        printf("Creating Socket on interface %s\n",tmp->ifa_name);

        struct sockaddr_ll *r_mac_addr = (struct sockaddr_ll *)tmp->ifa_addr;
        //memcpy(router_mac_addr, r_mac_addr->sll_addr, 6);
        memcpy(mac_addresses->router_mac_addr[i],r_mac_addr->sll_addr,6);
        // mac_addresses->int_name[i] = "eth-1";
        memcpy(mac_addresses->int_name[i], "eth1", 10);

        //memcpy(mac_addresses->router_ip_addr[i], tmp->ifa_addr.sa_data, 4);
        //memcpy(mac_addresses->int_name[0], tmp->if)
        //memcpy(mac_addresses->router_ip_addr[i].s_addr, r_ip_addr.s_addr, 4);


        //printf("%s\n", r_mac_addr);

        packet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        //printf("PCKT SOCKET %d\n", packet_socket);
        mac_addresses->file_descriptors[i] = packet_socket;

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
        //memcpy(router_mac_addr2, r_mac_addr->sll_addr, 6);
        //mac_addresses->router_mac_addr[1] = r_mac_addr->sll_addr;
        memcpy(mac_addresses->router_mac_addr[i],r_mac_addr->sll_addr,6);
        //mac_addresses->int_name[i] = "eth-0";
        memcpy(mac_addresses->int_name[i],"eth0",10);
        //printf("ETH0 SOCKET %d\n", eth0_socket);
        mac_addresses->file_descriptors[i] = eth0_socket;
      //  memcpy(mac_addresses->router_ip_addr[i], tmp->ifa_addr.sa_data, 4);
      // struct sockaddr_in *r_ip_addr = (struct sockaddr_in *)tmp->ifa_addr;
      // //memcpy(mac_addresses->router_ip_addr[i].s_addr, r_ip_addr.s_addr, 4);
      // mac_addresses->router_ip_addr[i].s_addr = r_ip_addr->sin_addr.s_addr;

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
          // struct sockaddr_in *r_ip_addr = (struct sockaddr_in *)tmp->ifa_addr;
          // //memcpy(mac_addresses->router_ip_addr[i].s_addr, r_ip_addr.s_addr, 4);
          // mac_addresses->router_ip_addr[i].s_addr = r_ip_addr->sin_addr.s_addr;

          //memcpy(router_mac_addr3, r_mac_addr->sll_addr, 6);
          //mac_addresses->router_mac_addr[2] = r_mac_addr->sll_addr;
          memcpy(mac_addresses->router_mac_addr[i],r_mac_addr->sll_addr,6);
          //mac_addresses->int_name[i] = "eth-2";
          memcpy(mac_addresses->int_name[i],"eth2",10);
          mac_addresses->file_descriptors[i] = eth1_socket;

          if(eth1_socket<0){
            perror("socket");
            return 2;
          }

            if(bind(eth1_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
              perror("bind");
            }
          }

          // for eth3
          //create a packet socket on interface r?-eth3
          if(!strncmp(&(tmp->ifa_name[3]),"eth3",4)){
            printf("Creating Socket on interface %s\n",tmp->ifa_name);

            struct sockaddr_ll *r_mac_addr = (struct sockaddr_ll *)tmp->ifa_addr;
            //memcpy(router_mac_addr, r_mac_addr->sll_addr, 6);
            memcpy(mac_addresses->router_mac_addr[i],r_mac_addr->sll_addr,6);
            // mac_addresses->int_name[i] = "eth-1";
            memcpy(mac_addresses->int_name[i], "eth3", 10);

            eth3_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
            //printf("PCKT SOCKET %d\n", eth3_socket);
            mac_addresses->file_descriptors[i] = eth3_socket;

            if(eth3_socket<0){
              perror("socket");
              return 2;
            }

            if(bind(eth3_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
              perror("bind");
            }
          }
        }
      //printf("Loop number: %d\n",i);

      i++;
    }
  //
  FD_SET(packet_socket, &sockets);
  FD_SET(lo_socket, &sockets);
  FD_SET(eth0_socket, &sockets);
  FD_SET(eth1_socket, &sockets);
  FD_SET(eth3_socket, &sockets);

  int j = 0;
  size_t length = 0;
  ssize_t readFile;
  char *fileBuffer = NULL;
  char ip1[20][15];
  char ip2[20][15];
  char prefix[20][10];
  char interface[20][10];
  if (file < 0) {
    printf("ERROR\n");
  }


  while(((readFile = getline(&fileBuffer, &length, file)) != -1)){
    // printf("Entered loop\n");
    // printf("Return line of length: %zu: \n", readFile);

    // length of line with 2nd col populated
    if (readFile == 29) {

      //printf("Null\n");
      memcpy(ip1[j],fileBuffer, 8);
      memcpy(prefix[j], fileBuffer+9, 2);
      memcpy(ip2[j], fileBuffer+11, 9);
      memcpy(interface[j], fileBuffer+21, 7);

    } else {
      //printf("Not null\n");

      memcpy(ip1[j],fileBuffer, 8);
      memcpy(prefix[j], fileBuffer+9, 2);
      memcpy(ip2[j], fileBuffer+12, 1);
      memcpy(interface[j], fileBuffer+14, 7);

    }
    // printf("%s\n",fileBuffer);
    // printf("IP1 %s\n", ip1[j]);
    // printf("PRE %s\n", prefix[j]);
    // printf("IP2 %s\n", ip2[j]);
    // printf("INT %s\n", interface[j]);
    j++;
    length = j;
  }
  free(fileBuffer);
  fclose(file);

  //table->table_length = length;
  int k = 0;
  for (k = 0; k < table->table_length; k++) {
     // char test[strlen(ip1[k])];
     // memcpy(test, ip1[k], strlen(ip1[k]));
     inet_aton(ip1[k], &table->first_ip[k]);
    // printf("ATON %lu\n", table->first_ip[k]);
     //printf("sizeof %d\n", sizeof(ip2[k]));
     if (strncmp(ip2[k], "-", 1) != 0) {
      // printf("yes\n");
       inet_aton(ip2[k],&table->second_ip[k]);
     } else {
       inet_aton("0.0.0.0", &table->second_ip[k]);
     }
    // printf("no\n");

     //memcpy(table->prefix[k], prefix[k], 8);
     table->prefix[k] = atoi(prefix[k]);
     table->prefix_bytes[k] = (-1) << 32 - atoi(prefix[k]);
     memcpy(table->name[k], interface[k],8);

     printf("ENTRY #%d\n", k);
     printf("FIRST IP %s\n", inet_ntoa(table->first_ip[k]));
     printf("SECOND IP %s\n", inet_ntoa(table->second_ip[k]));
     printf("PREFIX %d\n", table->prefix[k]);
     printf("PREFIX BYTES %d\n", table->prefix_bytes[k]);
     printf("NAME %s\n", table->name[k]);
     printf("\n");
  }


  // struct interface_mac_addresses {
  //   u_char router_mac_addr[10][6];
  //   struct in_addr router_ip_addr[10];
  //   char int_name[10][10];
  //   int total_sockets;
  //   u_int file_descriptors[10];
  // };
  int y = 0;
  for(; y < mac_addresses->total_sockets; y++) {
    printf("Entry #: %d\n",y);
    printf("Name; %s\n", mac_addresses->int_name[y]);
    printf("Mac: %s\n", mac_addresses->router_mac_addr[y]);
    printf("IP: %s\n", inet_ntoa(*((struct in_addr *)&(mac_addresses->router_ip_addr[y].s_addr))));
  }

  //("Ready to recieve now\n");
  while(1){
    //printf("Entered while loop\n");
    fd_set tmp_set=sockets;
    select(FD_SETSIZE,&tmp_set,NULL,NULL,&timeout);

    int i;
    int len;
    int sock;
    struct sockaddr_ll recvaddr;
    char buf[1500];
    int recvaddrlen;
    int n = 0;
    for(i=0;i<FD_SETSIZE;i++) {
      if(FD_ISSET(i,&tmp_set)) {

        if (packet_num >= 15) {
          packet_num = 0;
        }
        //printf("FDISSET VAL %d\n", FD_ISSET(i,&tmp_set));
        int s = 0;
        int num = 0;
        u_char our_mac[6];
        // just as cautionary measure
        memcpy(our_mac,mac_addresses->router_mac_addr[0],6);
        // gets correct mac for our socket
        for(s=0;s<mac_addresses->total_sockets;s++) {
          if (i == mac_addresses->file_descriptors[s]) {
            memcpy(our_mac,mac_addresses->router_mac_addr[s],6);
            num = s;
          }
        }

        // printf("socket num %d\n", num);
        // printf("OUR socket %s\n", mac_addresses->int_name[num]);

        len = sizeof(tmp);

        // here we are receiving the inital packet
        n = recvfrom(i, buf, 1500,0,(struct sockaddr*)&recvaddr, &recvaddrlen);

        if(recvaddr.sll_pkttype==PACKET_OUTGOING)
          continue;
        //start processing all others
        printf("Got a %d byte packet\n", n);
        struct ether_header *eth_request  = (struct ether_header*)buf;
        printf("Ether type: %x\n", ntohs(eth_request->ether_type));

        // checks if packet is arp
        printf("Checking Ether Type\n");
        if (ntohs(eth_request->ether_type) == ETHERTYPE_ARP) {
          printf("\n \n \n");
          printf("ARP packet\n");

          //struct arp_header *arp_request = (struct arp_header*)(buf+14);
          struct ether_arp *arp_request = (struct ether_arp*)(buf+14);

          if (strncmp(inet_ntoa(*((struct in_addr *)&arp_request->arp_spa)), "0.0.0.0", 7) == 0) {
            // we want to drop the packet
            printf("all 0s\n");
            break;

          }

          if (strncmp(inet_ntoa(*((struct in_addr *)&arp_request->arp_tpa)), "0.0.0.0", 7) == 0) {
            // we want to drop the packet
            printf("all 0s\n");
            break;

          }

          if (arp_request->arp_op == ntohs(ARPOP_REQUEST)) {
            char reply_data[1500];
            memcpy(reply_data,buf,1500);

            struct ether_header *eth_reply = (struct ether_header*)reply_data;
            //struct arp_header *arp_reply = (struct arp_header*)(reply_data+14);
            struct ether_arp *arp_reply = (struct ether_arp*)(reply_data+14);

            // populates ethernet header on ARP reply
            memcpy(&(eth_reply->ether_dhost),&(eth_request->ether_shost),6);
          //  memcpy(&(eth_reply->ether_shost), &(eth_request->ether_dhost),6);
            memcpy(&(eth_reply->ether_shost), &(our_mac),6);
            //printf("%s\n",eth_request->ether_shost);
            memcpy(&(eth_reply->ether_type), &(eth_request->ether_type), 6);
          //  printf("Ethernet Header is set up\n");

            // populates ARP header on ARP reply
            //printf("Starting arp_reply\n");

            memcpy(&(arp_reply->ea_hdr), &(arp_request->ea_hdr), sizeof(arp_request->ea_hdr));
            //memcpy(&(arp_reply->arp_op), ARPOP_REPLY, sizeof(arp_request->ea_hdr.ea_hdr));
            arp_reply->arp_op = ntohs(ARPOP_REPLY);
            arp_request->arp_hrd = ntohs(1);
            arp_request->arp_pro = ntohs(0x800);
            arp_request->arp_hln = 6;
            arp_request->arp_pln = 4;

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
          } else if (arp_request->arp_op == ntohs(ARPOP_REPLY)) {
            // we are forwarding the packet now
              printf("Got an Arp Reply\n");

            // here we will get data from arp reply
            // are we assuming all packets are ip?



          }



        }

        printf("Checking If IP\n");
        if (ntohs(eth_request->ether_type) == ETHERTYPE_IP){
          // header for icmp request
          // print
          printf("ICMP Request\n");
          struct iphdr *ip_request = (struct iphdr*)(buf+sizeof(struct ether_header));

          if (strncmp(inet_ntoa(*((struct in_addr *)&ip_request->saddr)), "0.0.0.0", 7) == 0) {
            // we want to drop the packet
            printf("all 0s\n");
            break;

          }

          if (strncmp(inet_ntoa(*((struct in_addr *)&ip_request->daddr)), "0.0.0.0", 7) == 0) {
            // we want to drop the packet
            printf("all 0s\n");
            break;

          }

          // if (strncmp(inet_ntoa(*((struct in_addr *)&ip_request->daddr)),inet_ntoa(*((struct in_addr *)&ip_request->saddr)), 4) == 0) {
          //   // we want to drop the packet
          //   printf("we don't want to arp ourselves\n");
          //   break;
          //
          // }


          //u_short ip_len = ip_request->ihl * 4;
          //printf("Protocol # %d\n", ip_request->protocol);

          // may need to check something other than this
          if (ip_request->protocol==1 && ip_request->daddr == mac_addresses->router_ip_addr[num].s_addr) {
            //printf("IP header");
            // struct icmphdr *icmp_request = (struct icmphdr*)(buf + 34);
            printf("ICMP for us\n");
            struct icmp_header *icmp_request = (struct icmp_header*)(buf + 14 + 20);
            //printf("Request Header created\n");
            //printf("%d\n", sizeof(struct icmphdr));

          //  printf("IPCMP header");

            // replies for ICMP
            char reply_data[1514];
            memcpy(reply_data, buf, 1514);

            struct ether_header *eth_reply = (struct ether_header*)reply_data;
            struct iphdr *ip_reply = (struct iphdr*)(reply_data+sizeof(struct ether_header));
            struct icmp_header *icmp_reply = (struct icmp_header*)(reply_data + 14+20);

            // means ICMP ECHO for our router

          //  printf("Reply header \n");

            //populates the ethernet header
            //memcpy(&(eth_reply),&(eth_request),20);
            memcpy(&(eth_reply->ether_dhost),&(eth_request->ether_shost),6);
            memcpy(&(eth_reply->ether_shost), &(eth_request->ether_dhost),6);
            memcpy(&(eth_reply->ether_type), &(eth_request->ether_type), 2);
            //printf("Eth header\n");

            // populates the ip header
            memcpy(&(ip_reply->saddr), &(ip_request->daddr), 4);
            memcpy(&(ip_reply->daddr), &(ip_request->saddr), 4);
            //printf("IP header\n");

            // populates the icmp headers
            icmp_reply->type = 0;
            icmp_reply->code = 0;
            memcpy(&(icmp_reply->checksum), &(ip_request->check), 2);
            //printf("ICMP hdr\n");

            int x = send(i, reply_data, 98, 0);

            if (x < 0) {
              printf("ERROR sending ICMP ECHO Reply!\n");
              perror("Error sending ICMP ECHO reply");
              continue;
            }

            printf("ICMP Echo Reply packet sent\n");

        // this going to need to be a different else statement
        // but seems to work for now!
        } else {

             printf("Entered else block\n");
             // we are receiving an IPv4 packet
             // we will need to look at dst

             // here we will need to do a routing table look up to see where we are sending to
             int x = 0;
             char interface_name[10];
             char ip1[8];
             char ip2[8];

             // printf("This entry: %s has no arp reply\n", inet_ntoa(*((struct in_addr *)&(ip_request->saddr))));

             memcpy(ip1, inet_ntoa(*((struct in_addr *)&ip_request->daddr)), 8);

             // table index
             int index = -1;
             int socket = 0;
             struct in_addr *ip_to_send = (struct in_addr*)malloc(sizeof(struct in_addr));

             // we will need the ip addresses to send our arp request
             for(; x < table->table_length; x++) {
               //printf("IP request dst: %lu\n", ip_request->daddr);
               printf("Packet IP address %s\n", inet_ntoa(*((struct in_addr *)&ip_request->daddr )));

               //int val_16 = 0xFFFF0000;

               int val = 0xFFFFFF00;
               if(table->prefix[x] == 16) {
                 //printf("sweet 16\n");
                 val = 0xFFFF0000;
               }
               if(table->prefix[x] == 16 ) {
                 // if (table->first_ip[x].s_addr == (ip_request->daddr & 0xFFFF0000)) {
                 //   printf("match test\n");
                 // }
                 // if (strncmp(ip1,ip2,2) == 0) {
                 //
                 // }
                 memcpy(ip2, inet_ntoa(*((struct in_addr *)&table->first_ip[x].s_addr)),8);

                 printf("IP 1: %s\n", ip1);
                 printf("IP 2: %s\n", ip2);

                 if(strncmp(ip1,ip2,4) == 0) {
                   printf("match\n");
                   memcpy(interface_name, "rx-eth0", 10);
                   printf("INTERFACE is being reset by 16\n");

                   index = x;
                 }
                 //memcpy(&ip_to_send->s_addr, &table->second_ip[x].s_addr, 4);
               }

               // only works for 24 bits
               if(!((ip_request->daddr ^ table->first_ip[x].s_addr) & htonl(val)) && table->prefix[x] == 24){
                 index = x;
                 printf("INTERFACE name is being reset by 24\n");
                 //interface_name = table->name[x];
                 memcpy(interface_name,&table->name[x],10);
                 printf("found in table\n");
                 printf("table: %s\n", interface_name);
               }

             }

             // do we want to do this here?
             // wait till we have checksum working to uncomment this
             //ip_to_send->ttl -= 1;
             printf("Passed for loops\n");

             // if our index is > -1 then entry is in the table else
             // otherwise we want to drop the packet
             if (index > -1 && ip_request->ttl > 0 ) {
               printf("in our routing table");
               // send arp request

               int u = 0;

               // ip_request->ttl -= 1;
               // ip_request->check = cksum(buf, sizeof(buf));

               // forwarding socket
               int var = 0;
               if (index > -1) {
                 for(; u < mac_addresses->total_sockets; u++) {
                   //if (strncmp(mac_addresses->int_name[u], interface_name+3, 3) == 0) {
                   if (!strncmp(&(mac_addresses->int_name[u]),"eth",3) && atoi(mac_addresses->int_name[u]+3) == atoi(interface_name+6)) {
                     //printf("MATCH MAC ADDR\n");
                     var = u;
                     printf("\n\n\n");
                     printf("Router Info: \n");
                     printf("\n");
                     printf("Name: %s\n", mac_addresses->int_name[u]);
                     printf("IP address: %s\n", inet_ntoa(*(struct in_addr*)&mac_addresses->router_ip_addr[var].s_addr));

                     if (mac_addresses->file_descriptors[var] == packet_socket) {
                       printf("ETH 1 Socket\n");
                     }

                     if (mac_addresses->file_descriptors[var] == eth0_socket) {
                       printf("ETH 0 Socket\n");

                     }

                     if (mac_addresses->file_descriptors[var] == eth1_socket) {
                       printf("ETH 2 Socket\n");

                     }

                     if (mac_addresses->file_descriptors[var] == eth3_socket) {
                       printf("ETH 3 Socket\n");

                     }

                   }
                 }
               }

               printf("INTERFACE NAME: %s\n", interface_name);
               // if (table->prefix[index] == 24) {
               //   memcpy(&ip_to_send->s_addr, &mac_addresses->router_ip_addr[var].s_addr, 4);
               // }

               // add our packet to the array

               if(packet_num >= 15) {
                 packet_num = 0;
               } else {
                 packet_num++;
               }

               memcpy(packet_queue[packet_num], buf, 1500);

               // assuming we have a match we will now send an arp request
               char reply_data[1500];

               // we don't want to do this
               //memcpy(reply_data,buf,1500);

               // eth header here:
               struct ether_header *eth_arp_request = (struct ether_header*)reply_data;
               //struct arp_header *arp_reply = (struct arp_header*)(reply_data+14);
               struct ether_arp *arp_request = (struct ether_arp*)(reply_data+14);

               u_char broadcast_addr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

               // this is going to change to broadcast
               memcpy(&(eth_arp_request->ether_dhost),broadcast_addr,6);
             //  memcpy(&(eth_reply->ether_shost), &(eth_request->ether_dhost),6);
               memcpy(&(eth_arp_request->ether_shost),  &(mac_addresses->router_mac_addr[var]),6);
               //memcpy(&(eth_arp_request->ether_type), 0x06, 2);
               eth_arp_request->ether_type = ntohs(ETHERTYPE_ARP);

               printf("Ethernet Header is set up\n");

               // populates ARP header on ARP reply
               printf("Starting arp_request\n");

               struct arphdr *arphdr_eahdr = (struct arphdr*)malloc(sizeof(struct arphdr));
               memcpy(&(arp_request->ea_hdr), &(arphdr_eahdr), sizeof(arp_request->ea_hdr));
               //memcpy(&(arp_reply->arp_op), ARPOP_REPLY, sizeof(arp_request->ea_hdr.ea_hdr));
               arp_request->arp_op = ntohs(ARPOP_REQUEST);
               arp_request->arp_hrd = ntohs(1);
               arp_request->arp_pro = ntohs(0x800);
               arp_request->arp_hln = 6;
               arp_request->arp_pln = 4;
               // this may be wrong but double check
               //arp_reply->ea_hdr.arp_op=ARPOP_REPLY;

               memcpy(&(arp_request->arp_sha), &(mac_addresses->router_mac_addr[var]), 6);
               memcpy(&(arp_request->arp_spa), &(mac_addresses->router_ip_addr[var].s_addr), 4);
               memcpy(&(arp_request->arp_tha), broadcast_addr, 6);
               memcpy(&(arp_request->arp_tpa), &(ip_request->daddr), 4);

               // will need to send to correct socket
               int y = send(mac_addresses->file_descriptors[var], reply_data, 42, 0);

               if (y < 0) {
                 printf("ERROR sending ARP Request!\n");
                 perror("Error sending ARP request");
                 continue;
               }

               printf("ARP Request packet sent\n");
               free(arphdr_eahdr);

               // forwarding

               char arp_reply_data[1500];

               // lets block until we recive the arp reply with the mac address
               int hop_mac = 1;
               // may not need to do this?

               struct sockaddr_ll recvaddr;
               int recvrlen;
               int current_time;
               int b = 0;

               setsockopt(mac_addresses->file_descriptors[var], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

               while (hop_mac) {
                 current_time = recvfrom(mac_addresses->file_descriptors[var], arp_reply_data, 1500, 0, (struct sockaddr*) &recvaddr, recvrlen);
                 if (recvaddr.sll_pkttype == PACKET_OUTGOING) {
                   continue;
                 }
                 printf("blocking till we receive the packet\n");

                 if (current_time == -1) {
                   printf("no arp reply\n");
                   //break;
                 }
                 hop_mac = 0;
               }

               if (strncmp(inet_ntoa(*((struct in_addr *)&ip_request->saddr)), "0.0.0.0", 7) == 0) {
                 // we want to drop the packet
                 printf("all 0s\n");
                 //break;

               }

               if (strncmp(inet_ntoa(*((struct in_addr *)&ip_request->daddr)), "0.0.0.0", 7) == 0) {
                 // we want to drop the packet
                 printf("all 0s\n");
                 break;

               }

               // if (strncmp(inet_ntoa(*((struct in_addr *)&ip_request->daddr)),inet_ntoa(*((struct in_addr *)&ip_request->saddr)), 6) == 0) {
               //   // we want to drop the packet
               //   printf("we don't want to arp ourselves\n");
               //   break;
               //
               // }

               struct ether_header *block_eth_result = (struct ether_header *) arp_reply_data;
               printf("%02x\n", block_eth_result->ether_type);
               printf("recv val %d\n", current_time);

               if (ntohs(block_eth_result->ether_type) != ETHERTYPE_ARP) {
                 // this address is NOT in our routing table
                 printf("This entry: %s has no arp reply\n", inet_ntoa(*((struct in_addr *)&(ip_request->saddr))));
                 // create error packet
                 char error_hdr[1500];
                 memcpy(error_hdr,buf,1500);

                 printf("Getting seg fault somewhere\n");

                 struct ether_header *eth_error = (struct ether_header *) error_hdr;
                 struct iphdr *ip_error = (struct iphdr *) (error_hdr + 14);
                 struct icmp_header *icmp_error = (struct icmp_header *)(error_hdr + 14 + 20);

                 printf("Is it here?\n");


                 memcpy(&eth_error->ether_shost,our_mac, 6);
                 printf("ether_shost\n");
                 memcpy(&eth_error->ether_dhost, &eth_request->ether_shost, 6);
                 printf("ether_dhost\n");

                 //memcpy(&eth_error->ether_type, ntohs(ETHERTYPE_IP), 2);
                 eth_error->ether_type = ntohs(ETHERTYPE_IP);

                 printf("ether_type\n");

                 memcpy(&ip_error->saddr, &mac_addresses->router_ip_addr[num].s_addr, 4);
                 printf("ip s_addr\n");

                 memcpy(&ip_error->daddr, &ip_request->saddr, 4);
                 ip_error->protocol = 1;

                 printf("Maybe here?\n");


                 // icmp header
                 icmp_error->type = ICMP_DEST_UNREACH;
                 icmp_error->code = ICMP_HOST_UNREACH;

                 printf("Or here?\n");


                 // this is going to have to change once checksum works
                 icmp_error->checksum = ip_request->check;

                 int x = send(i, error_hdr, 98, 0);

                 if (x < 0) {
                   printf("ERROR sending ICMP HOST UNREACHABLE ERROR!\n");
                   perror("Error sending ICMP HOST UNREACHABLE ERROR");
                   continue;
               }
               //if (b == 1) {
                 //printf("%s\n" );

               } else {
                 // now we can forward?

                 // here we create a new ether header
                 printf("Forwarding the Packet!\n");
                 char forward_data[n];
                 memcpy(&forward_data, &buf, n);
                 struct ether_header *next_eth_reply = (struct ether_header *) arp_reply_data;
                 struct ether_header *new_eth_header = (struct ether_header *)forward_data;


                 //int h = 0;
                 // for (;h<mac_addresses->table_length;h++) {
                 //   if (strncmpmac_addresses)
                 // }
                 memcpy(new_eth_header->ether_dhost, next_eth_reply->ether_shost, 6);

                 // source may need to be the router
                 //memcpy(new_eth_header->ether_shost, mac_addresses->router_mac_addr[var], 6);
                 next_eth_reply->ether_type = ntohs(ETHERTYPE_IP);

                 printf("sending reply?");
                 // may need to change this
                // memcpy(&forward_data, &buf[0], 98);
                 //memcpy(&forward_data[0], &arp_reply_data, 14);

                 // size might be different
                 // don't use 98
                 send(mac_addresses->file_descriptors[var], forward_data, sizeof(forward_data), 0);

                 // implement a timeout for arp reply
               }



             } else if(index == -1) {
               // this address is NOT in our routing table
               printf("This entry: %s is not in our routing table\n", inet_ntoa(*((struct in_addr *)&(ip_request->saddr))));
               // create error packet
               char error_hdr[1500];
               memcpy(error_hdr,buf,1500);

               struct ether_header *eth_error = (struct ether_header *) error_hdr;
               struct iphdr *ip_error = (struct iphdr *) (error_hdr + 14);
               struct icmp_header *icmp_error = (struct icmp_header *)(error_hdr + 14 + 20);

               memcpy(&eth_error->ether_shost,our_mac, 6);
               memcpy(&eth_error->ether_dhost, &eth_request->ether_shost, 6);
               //memcpy(&eth_error->ether_type, ntohs(ETHERTYPE_IP), 2);
               eth_error->ether_type = ntohs(ETHERTYPE_IP);

               memcpy(&ip_error->saddr, &mac_addresses->router_ip_addr[num].s_addr, 4);
               memcpy(&ip_error->daddr, &ip_request->saddr, 4);
               ip_error->protocol = 1;

               // icmp header
               icmp_error->type = ICMP_DEST_UNREACH;
               icmp_error->code = ICMP_NET_UNREACH;

               // this is going to have to change once checksum works
               icmp_error->checksum = ip_request->check;

               int x = send(i, error_hdr, 98, 0);

               if (x < 0) {
                 printf("ERROR sending ICMP NET UNREACHABLE ERROR!\n");
                 perror("Error sending ICMP NET UNREACHABLE ERROR");
                 continue;
               }


             } else if (ip_request->ttl == 0) {
               // TIME to drop the packet
               // this address is NOT in our routing table
               printf("This packet: %s is dead\n", inet_ntoa(*((struct in_addr *)&(ip_request->saddr))));
               // create error packet
               char error_hdr[1500];
               memcpy(error_hdr,buf,1500);

               struct ether_header *eth_error = (struct ether_header *) error_hdr;
               struct iphdr *ip_error = (struct iphdr *) (error_hdr + 14);
               struct icmp_header *icmp_error = (struct icmp_header *)(error_hdr + 14 + 20);

               memcpy(&eth_error->ether_shost,our_mac, 6);
               memcpy(&eth_error->ether_dhost, &eth_request->ether_shost, 6);
               //memcpy(&eth_error->ether_type, ntohs(ETHERTYPE_IP), 2);
               eth_error->ether_type = ntohs(ETHERTYPE_IP);

               memcpy(&ip_error->saddr, &mac_addresses->router_ip_addr[num].s_addr, 4);
               memcpy(&ip_error->daddr, &ip_request->saddr, 4);
               ip_error->protocol = 1;

               // icmp header
               icmp_error->type = ICMP_TIME_EXCEEDED;
               icmp_error->code = ICMP_EXC_TTL;

               // this is going to have to change once checksum works
               icmp_error->checksum = ip_request->check;

               int x = send(i, error_hdr, 98, 0);

               if (x < 0) {
                 printf("ERROR sending ICMP TTL EXCEEDED ERROR!\n");
                 perror("Error sending ICMP TTL EXCEEDED ERROR");
                 continue;
               }
             }

          }
          }
           //else {
      //  }

      }
      //printf("For Loop #%d\n", i);
    }
    //printf("Exiting for loop\n");
    FD_CLR(i,&sockets);

  }
  //free the interface list when we don't need it anymore
  printf("Exiting routers\n");
  freeifaddrs(ifaddr);
  free(table);
  free(mac_addresses);
  //exit
  return 0;
}
