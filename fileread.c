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
#include <netinet/if_ether.h>
#include <string.h>

int main(int argc, char** argv){
	FILE *file1;
	char* buffer;
	size_t length = 0;
	ssize_t readFile;
	int numLine = 0;
	
		
	struct fTable{
		uint32_t matchIP;
		uint32_t sendIP;
		int prefixLength;
		char socket[10];
	};

	struct fTable* FArray[10];
	
	
	file1 = fopen("r1-table.txt", "r");

	/**
 		int functionHandler(String cmd) {
    		uint32_t value = strtoul(cmd.c_str(), NULL, 10);
    		Serial.printlnf("value is: %u", value);
    		return 1;
		} 
	***/


	if(file1 != NULL){

		
		while((readFile = getline(&buffer, &length, file1 ) != -1 )){
			//getting the IP from the string			
			char tempIP[50] = strcpy(tempIP, buffer);
			char* ptr;
			ptr = strchr(tempIP, "/");
			int index = ptr - buffer;
			tempIP[index] = "\0";		
			printf("%s", buffer);
			numLine++;
		}	

	}else{
		printf("The file cannot be read.");
	}

	

}
