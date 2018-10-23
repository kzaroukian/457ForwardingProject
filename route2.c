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
  int eth3_socket;
  int packet_num = 0;

  struct interface_mac_addresses {
    u_char router_mac_addr[10][6];
    struct in_addr router_ip_addr[10];
    char int_name[10][10];
    int total_sockets;
    u_int file_descriptors[10];
  };

  // queue of possible packets
  char packet_queue[15][1500];
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
      printf("Router ONE\n");
      file = fopen("r1-table.txt", "r");
      table->table_length = 4;
    }

    if(strncmp(tmp->ifa_name, "r2",2) == 0) {
      printf("Router TWO\n");
      file = fopen("r2-table.txt", "r");
      table->table_length = 5;
    }

    if (tmp->ifa_addr->sa_family==AF_INET){
      struct sockaddr_in *r_ip_addr = (struct sockaddr_in *)tmp->ifa_addr;
      mac_addresses->router_ip_addr[r].s_addr = r_ip_addr->sin_addr.s_addr;
      printf("IP From struct %lu\n", r_ip_addr->sin_addr.s_addr);
      printf("actual IP %s\n", inet_ntoa(r_ip_addr->sin_addr));
      r++;

    }

    if(tmp->ifa_addr->sa_family==AF_PACKET){
      printf("Interface: %s\n",tmp->ifa_name);

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
        printf("PCKT SOCKET %d\n", packet_socket);
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
        printf("ETH0 SOCKET %d\n", eth0_socket);
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
            printf("PCKT SOCKET %d\n", eth3_socket);
            mac_addresses->file_descriptors[i] = packet_socket;

            if(eth3_socket<0){
              perror("socket");
              return 2;
            }

            if(bind(eth3_socket,tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
              perror("bind");
            }
          }
        }
      printf("Loop number: %d\n",i);

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
    printf("Entered loop\n");
    printf("Return line of length: %zu: \n", readFile);

    // length of line with 2nd col populated
    if (readFile == 29) {

      printf("Null\n");
      memcpy(ip1[j],fileBuffer, 8);
      memcpy(prefix[j], fileBuffer+9, 2);
      memcpy(ip2[j], fileBuffer+11, 9);
      memcpy(interface[j], fileBuffer+21, 7);

    } else {
      printf("Not null\n");

      memcpy(ip1[j],fileBuffer, 8);
      memcpy(prefix[j], fileBuffer+9, 2);
      memcpy(ip2[j], fileBuffer+12, 1);
      memcpy(interface[j], fileBuffer+14, 7);

    }
    printf("%s\n",fileBuffer);
    printf("IP1 %s\n", ip1[j]);
    printf("PRE %s\n", prefix[j]);
    printf("IP2 %s\n", ip2[j]);
    printf("INT %s\n", interface[j]);
    j++;
    length = j;
  }
  free(fileBuffer);
  fclose(file);

  //table->table_length = length;
  int k = 0;
  for (k = 0; k < table->table_length-1; k++) {
     // char test[strlen(ip1[k])];
     // memcpy(test, ip1[k], strlen(ip1[k]));
     inet_aton(ip1[k], &table->first_ip[k]);
     printf("ATON %lu\n", table->first_ip[k]);
     printf("sizeof %d\n", sizeof(ip2[k]));
     if (strncmp(ip2[k], "-", 1) != 0) {
       printf("yes\n");
       inet_aton(ip2[k],&table->second_ip[k]);
     } else {
       inet_aton("0.0.0.0", &table->second_ip[k]);
     }
     printf("no\n");

     //memcpy(table->prefix[k], prefix[k], 8);
     table->prefix[k] = atoi(prefix[k]);
     table->prefix_bytes[k] = (-1) << 32 - atoi(prefix[k]);
     memcpy(table->name[k], interface[k],8);

     printf("FIRST IP %s\n", inet_ntoa(table->first_ip[k]));
     printf("SECOND IP %s\n", inet_ntoa(table->second_ip[k]));
     printf("PREFIX %d\n", table->prefix[k]);
     printf("PREFIX BYTES %d\n", table->prefix_bytes[k]);
     printf("NAME %s\n", table->name[k]);
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
    printf("Name; %s\n", mac_addresses->int_name[y]);
    printf("Mac: %s\n", mac_addresses->router_mac_addr[y]);
    printf("IP: %s\n", inet_ntoa(*((struct in_addr *)&(mac_addresses->router_ip_addr[y].s_addr))));
  }

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

        if (packet_num >= 15) {
          packet_num = 0;
        }
        printf("FDISSET VAL %d\n", FD_ISSET(i,&tmp_set));
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

        printf("socket num %d\n", num);
        printf("OUR socket %s\n", mac_addresses->int_name[num]);

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
            printf("%s\n",eth_request->ether_shost);
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
          } else if (arp_request->arp_op == ntohs(ARPOP_REPLY)) {
            // we are forwarding the packet now

          }



        }
        if (ntohs(eth_request->ether_type) == ETHERTYPE_IP){
          // header for icmp request
          printf("ICMP Request\n");
          struct iphdr *ip_request = (struct iphdr*)(buf+sizeof(struct ether_header));
          u_short ip_len = ip_request->ihl * 4;
          printf("Protocol # %d\n", ip_request->protocol);
          if (ip_request->protocol==1 && ip_request->daddr == mac_addresses->router_ip_addr[num].s_addr) {
            printf("IP header");
            // struct icmphdr *icmp_request = (struct icmphdr*)(buf + 34);
            struct icmp_header *icmp_request = (struct icmp_header*)(buf + 14 + ip_len);
            printf("Request Header created\n");
            printf("%d\n", sizeof(struct icmphdr));

            printf("IPCMP header");

            // replies for ICMP
            char reply_data[1514];
            memcpy(reply_data, buf, 1514);

            struct ether_header *eth_reply = (struct ether_header*)reply_data;
            struct iphdr *ip_reply = (struct iphdr*)(reply_data+sizeof(struct ether_header));
            struct icmp_header *icmp_reply = (struct icmp_header*)(reply_data + 14+20);

            // means ICMP ECHO for our router

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

        } else {
             printf("Entered else block\n");
             // we are receiving an IPv4 packet
             // we will need to look at dst

             // here we will need to do a routing table look up to see where we are sending to
             int x = 0;
             char interface_name[10];
             int index = 0;

             // we will need the ip addresses to send our arp request
             for(; x < table->table_length; x++) {
               printf("IP request dst: %lu\n", ip_request->daddr);
               printf("IP data %s\n", inet_ntoa(*((struct in_addr *)&ip_request->daddr )));

               //int val_16 = 0xFFFF0000;
               int val = 0xFFFFFF00;
               if(table->prefix[x] == 16) {
                 printf("sweet 16\n");
                 val = 0xFFFF0000;
               }
               if(table->prefix[x] == 16) {
                 memcpy(interface_name, "rx-eth0", 10);
                 index = x;
               }

               if(!((ip_request->daddr ^ table->first_ip[x].s_addr) & htonl(val))){
                 index = x;
                 //interface_name = table->name;
                 memcpy(interface_name,&table->name[x],10);
                 printf("found in table\n");
                 printf("table: %s\n", interface_name);
               }

             }
             printf("Passed for loops\n");



             int u = 0;
             int var = 0;
             if (strlen(interface_name) > 1) {
               for(; u < mac_addresses->total_sockets; u++) {
                 if (strncmp(mac_addresses->int_name[u], interface_name+3, 3) == 0) {
                   //printf("MATCH MAC ADDR\n");
                   var = u;
                 }
               }
             }


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
             memcpy(&(eth_arp_request->ether_shost), &(our_mac),6);
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

             memcpy(&(arp_request->arp_sha), our_mac, 6);
             memcpy(&(arp_request->arp_spa), &(mac_addresses->router_ip_addr[var].s_addr), 4);
            // if (table->prefix[index] == 24) {

             // } else {
             //   int p = 0;
             //   int answer = 0;
             //   for(; p < table->table_length; p++) {
             //     if (table->prefix[index] == 16) {
             //       answer = p;
             //       printf("Second IP: %s\n",inet_ntoa(*(struct in_addr*)&table->second_ip[answer].s_addr));
             //     }
             //   }
             //   // only will work for part 2
             //   memcpy(&(arp_request->arp_spa), &table->second_ip[answer].s_addr, 4);
             // }
             memcpy(&(arp_request->arp_tha), broadcast_addr, 6);
             memcpy(&(arp_request->arp_tpa), &(ip_request->daddr), 4);

             // will need to send to correct socket
             int y = send(mac_addresses->file_descriptors[num], reply_data, 42, 0);

             if (y < 0) {
               printf("ERROR sending ARP Request!\n");
               perror("Error sending ARP request");
               continue;
             }

             printf("ARP Request packet sent\n");
             // arp header here:

             // sending request here:

          }
          }
           //else {
      //  }

      }
      //printf("For Loop #%d\n", i);
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
