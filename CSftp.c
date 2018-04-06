#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "dir.h"
#include "usage.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int logged = 0, connection = 0, binary = 0, portno, file_descriptor, new_file_descriptor, length_address, length_client, file_descriptor1, length_address1, new_file_descriptor1, portno1;
struct sockaddr_in address_server, address_client, address_server1;
char buffer[256], root_dir[1024], buffer1[256];
// Here is an example of how to use the above function. It also shows
// one how to get the arguments passed on the command line.

void itoa(int value, char* str, int base){
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int sign;

	// Validate base
	if (base<2 || base>35){ *wstr='\0'; return; }

	// Take care of sign
	if ((sign=value) < 0) value = -value;

	// Conversion. Number is reversed.
	do *wstr++ = num[value%base]; while(value/=base);

	if(sign<0) *wstr++='-';

	*wstr='\0';

	// Reverse string
	char* beg = str;
	char* end = wstr-1;
	char aux;

	while(end>beg){
		aux=*end, *end--=*beg, *beg++=aux;
	}

}

char* substrings(char* str, int start, int end) {
	char* val;
	int i;

	val= malloc(end + 1);

	if (val == NULL) {
		printf("Unable to allocate memory.\n");
		exit(1);
	}

	for (i = 0; i < end - start; i++) {
		*(val+i) = *(str + start + i);
	}

	*(val+i-1) = '\0';

	return val;
}


void user1(char* buffer){
  if(logged){
		char* response = "503 Already logged in.\n";
    write(new_file_descriptor, response, strlen(response));
  }
  else {
		if(strcasecmp("USER CS317\r\n", buffer) != 0){
			char* response = "530 Not logged in.\n";
			printf("Not logged\n");
	    write(new_file_descriptor, response, strlen(response));
  	}
  	else{
	    logged = 1;
	    char* response="230 User logged in, proceed.\n";
	    write(new_file_descriptor, response, strlen(response));
  	}
  }
  memset(buffer, 0, strlen(buffer));
  return;
}

void nlst(char* buffer){
  // int size = strlen(buffer);
	char* response = "150 File status okay; about to open data connection.\n";
	write(new_file_descriptor, response, strlen(response));
  if (strlen(buffer) > 6) {
		char* response3 = "501 Syntax error in parameters or arguments.\n";
		write(new_file_descriptor, response3, strlen(response3));
	} else {
		printf("Printed %d directory entries\n", listFiles(new_file_descriptor1, "."));
	}
	close(new_file_descriptor1);
	char* response4 = "226 Closing data connection, file transfer successful\n";
	write(new_file_descriptor, response4, strlen(response4));
  memset(buffer, 0, strlen(buffer));
  return;
}

void cwd(char* buffer){
  char* query=substrings(buffer, 4, strlen(buffer)-1);
	printf("Query before:%s\n", query);
//  strcat(query,"apple");
  printf("cwd, query:%s\n",query );

  if (strncmp(substrings(query, 0, 2), "./", 2) == 1 || strstr(query, "..") != NULL) {
		char* response2 = "504 Command not implemented for that parameter.\n";
		write(new_file_descriptor, response2, strlen(response2));
	}
  else {
	int i=0;
	while(query[i] != '\0'){
    query[i] = tolower(query[i]);
    i++;
  }
	printf("Query1:%s\n",query);
	if (chdir(query) != 0) {
		perror("error while changing dir");
	 	char* response1 = "550 Requested action not taken. Directory does not exist\n";
	  write(new_file_descriptor, response1, strlen(response1));
		memset(buffer, 0, strlen(buffer));
		return;
	}
  char* temp = "200 directory changed to ";
  strcat(query,"\n");
  int newSize = strlen(temp)+strlen(query)+1;
  char* response = (char *)malloc(newSize);
  strcpy(response,temp);
  strcat(response,query);
  write(new_file_descriptor, response, strlen(response));
	memset(buffer, 0, strlen(buffer));
	return;
	}

}

void cdup(char* buffer){
  char cwd[1024];

		printf("CDUP in progress\n");
    if(getcwd(cwd, sizeof(cwd)) == NULL){
      perror("getcwd() error");
    }
    if(!strncmp(root_dir, cwd, strlen(cwd))){
			char* response1 = "503 Bad sequence of commands.\n";
      write(new_file_descriptor, response1, strlen(response1));
    }
    else{
      if(chdir("../")!=0){
        perror("error while changing dir");

      }
			else{
				printf("Changed back\n");
			}
      char* query="../\n";
      char* temp = "200 directory changed to ";
      // strcat(query,"\n");
      int newSize = strlen(temp)+strlen(query)+1;
      char* response = (char *)malloc(newSize);
      strcpy(response,temp);
      strcat(response,query);
      write(new_file_descriptor, response, strlen(response));
    }

  memset(buffer, 0, strlen(buffer));
	return;
}

void quit1(char* buffer){
  logged=0;
  connection=1;
  memset(buffer, 0, strlen(buffer));
  char* response="221 Service closing control connection.\n";
  write(new_file_descriptor, response, strlen(response));
	return;
}

void retr(char* buffer){
  return;
}

void type(char* buffer){
  char* type = substrings(buffer, 5, strlen(buffer)-1);
  printf("Type:%s",type);
  if(!strcasecmp(type, "A")){
    binary=0;
  }
  else if(!(strcasecmp(type, "I"))){
    binary=1;
	}
  else {
    char* response1="504 Command not implemented for that parameter.\n";
    write(new_file_descriptor, response1, strlen(response1));
    return;
  }
  char* response="200 Command okay.\n";
  write(new_file_descriptor, response, strlen(response));
  return;
}

void mode(char* buffer){
  char* mode = substrings(buffer, 5, strlen(buffer)-1);
  if(strcasecmp(mode, "S")!=0){
    char* response1="504 Command not implemented for that parameter.\n";
    write(new_file_descriptor, response1, strlen(response1));
    return;
  }
  else{
    char* response="200 Command okay.\n";
    write(new_file_descriptor, response, strlen(response));
    return;
  }
}

void stru(char* buffer){
	char* stru = substrings(buffer, 5, strlen(buffer)-1);
	if(strcasecmp(stru, "F") != 0){
		char* response1="504 Command not implemented for that parameter.\n";
    write(new_file_descriptor, response1, strlen(response1));
    return;
  }
  else {
    char* response="200 Command okay.\n";
    write(new_file_descriptor, response, strlen(response));
    return;
  }
}

void pasv(){
	char response[256], host[1024];
	char* ip1;
	struct hostent *h;
	struct in_addr **list;
	int ip[4];

	if((file_descriptor1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}
	address_server1.sin_family = AF_INET;
	address_server1.sin_port = 0;
	address_server1.sin_addr.s_addr = INADDR_ANY;
	length_address1 = sizeof(address_server1);
	if(bind(file_descriptor1, (struct sockaddr *) &address_server1, sizeof(address_server1)) < 0){
		perror("Error in bind");
		exit(EXIT_FAILURE);
	}
	if(getsockname(file_descriptor1, (struct sockaddr *) &address_server1, (socklen_t *) &length_address1) < 0){
		perror("getsockname error");
		exit(EXIT_FAILURE);
	}
	gethostname(host, sizeof(host));
	if((h = gethostbyname(host)) == NULL){
		herror("gethostbyname error");
		return;
	}
	list = (struct in_addr **)h->h_addr_list;
	ip1 = inet_ntoa(*list[0]);
	sscanf(ip1, "%d.%d.%d.%d", &ip[0],&ip[1],&ip[2],&ip[3]);
	int portno2 = ntohs(address_server1.sin_port);
	printf("Port: %d\nIP: %s\n", portno2,ip1);
	if(listen(file_descriptor1, 5) < 0) {
		perror("Cannot Listen");
		exit(EXIT_FAILURE);
	}
	char p1[256];
	char p2[256];
	int port1 = portno2/256;
	int port2 = portno2%256;
	itoa(port1, p1, 10);
	itoa(port2, p2, 10);
	sprintf(response, "227 Entering Passive Mode (%d, %d, %d, %d, %d, %d)\n", ip[0], ip[1], ip[2], ip[3], port1, port2);
	write(new_file_descriptor, response, strlen(response));
	if((new_file_descriptor1 = accept(file_descriptor1, (struct sockaddr *) &address_client, (socklen_t *) &length_client)) < 0){
		perror("Error in accept");
		exit(EXIT_FAILURE);
	}
	return;
}

void client_parser(char* buffer){
  int i = 0;
	memset(buffer1, 0, strlen(buffer1));
	strcpy(buffer1, buffer);
  while(buffer[i] != '\0'){
    buffer[i] = toupper(buffer[i]);
    i++;
  }

  if (strncmp("USER", buffer, 4) == 0) {
    user1(buffer);
  }
	else {
    if (logged) {
      if (strncmp("QUIT", buffer, 4) == 0) {
        quit1(buffer);
      }
      else if ((strncmp("CWD", buffer, 3) == 0) || (strncmp("XCWD", buffer, 4) == 0)){
        printf("HERE\n");
        cwd(buffer);
      }
      else if (strncmp("CDUP", buffer, 4) == 0) {
				printf("CDUP here\n" );
        cdup(buffer);
      }
      else if (strncmp("TYPE", buffer, 4) == 0) {
        type(buffer);
      }
      else if (strncmp("MODE", buffer, 4) == 0) {
        mode(buffer);
      }
      else if (strncmp("STRU", buffer, 4) == 0) {
        stru(buffer);
      }
      else if (strncmp("RETR", buffer, 4) == 0) {
        retr(buffer);
      }
      else if (strncmp("PASV", buffer, 4) == 0) {
        pasv();
      }
			else if ((strncmp("NLST", buffer, 4) == 0)) {
        nlst(buffer);
      }
			else {
				printf("Others\n");
	      char* response="500\r\n";
	    	write(new_file_descriptor, response, strlen(response));
      }
    }
    else {
      char* response="530 Not Logged in.\r\n";
      write(new_file_descriptor, response, strlen(response));
      memset(buffer,0,strlen(buffer));
    }
  }
  return;
}

int main(int arwgc, char **argv) {

    // Check the command line arguments
    if (arwgc != 2) {
      usage(argv[0]);
      return -1;
    }

    // get port
    portno = atoi(argv[1]);

    // Create socket
    if((file_descriptor=socket(AF_INET, SOCK_STREAM, 0)) == 0){
      perror("error creating socket");
      exit(EXIT_FAILURE);
    }
    address_server.sin_family = AF_INET;
    address_server.sin_port = htons(portno);
    address_server.sin_addr.s_addr = INADDR_ANY;
    length_address = sizeof(address_server);

    // Bind to port
    if (bind(file_descriptor, (struct sockaddr *) &address_server, length_address) < 0) {
      perror("error while binding");
      exit(EXIT_FAILURE);
    }

    // listen for clients
    if (listen(file_descriptor, 5) < 0) {
      perror("listen fail");
      exit(EXIT_FAILURE);
    }

    length_client = sizeof(address_client);

    while(1){
      // Get new connection
      if((new_file_descriptor = accept(file_descriptor, (struct sockaddr*) &address_client, (socklen_t*) &length_client)) < 0){
        perror("error while accepting");
        exit(EXIT_FAILURE);
      }
      char* response="220 Service ready for new user.\n";
      write(new_file_descriptor, response, strlen(response));
      memset(buffer, 0, strlen(buffer));
      if (getcwd(root_dir, sizeof(root_dir)) == NULL) {
        perror("getcwd() error");
      }
      if(read(new_file_descriptor, buffer, 255) < 0){
        perror("error reading from socket");
        exit(EXIT_FAILURE);
      }
      do{
        client_parser(buffer);
				printf("Buffer:%s\n",buffer );
        memset(buffer, 0, strlen(buffer));
				int x = read(new_file_descriptor, buffer, 255);
        if(x < 0){
          perror("error reading from socket");
          exit(EXIT_FAILURE);
        }
      } while(connection == 0);
      close(new_file_descriptor);
    }

      return 0;
  }
