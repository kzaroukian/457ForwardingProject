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
    char word0[4] = "abcd";
    u_short* word1 = (u_short*)word0;

    char word2[6];
    u_short ck = 0;
    ck = cksum(word1, 2);
    printf("%lu\n", (sizeof(word1)));
    printf("%x\n", ck);

    memcpy(word2,word1,4);
    memcpy(&word2[4],&ck,2);
    u_short ck2;
    ck2=cksum((u_short*)word2,3);

    printf("%x\n", ck2);

}
