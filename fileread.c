#include <stdio.h>

int main(int argc, char** argv){
	FILE *file1;
	char* buffer;
	char result[3][50];

	
	file1 = open(argv[1], "r");

	size_t length = 0;
	ssize_t readFile;


	if(file1 != NULL){
		while((readFile = getline(&buffer, &length, file1 ) != -1 )){
			printf("%s", buffer);
		}	

	}else{
		printf("The file cannot be read.");
	}

	

}
