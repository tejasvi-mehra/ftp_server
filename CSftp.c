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

int logged = 0, connection = 0, binary = 0, portno, file_descriptor, new_file_descriptor, length_address, length_client;
struct sockaddr_in address_server, address_client;
char buffer[256], root_dir[1024];
// Here is an example of how to use the above function. It also shows
// one how to get the arguments passed on the command line.
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
    write(new_file_descriptor, "530", 3);
  }
  else{
    char check[256];
    memset(check, 0, strlen(check));
    strcpy(check, "USER ");
    strcat(check, "CS317");
    if(strcasecmp("USER CS317", buffer) == 0){
      write(new_file_descriptor, "530", 3);
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
  if (strlen(buffer) > 5) {
		write(new_file_descriptor, "501", 3);
	} else {
    char* response="250 Requested file action okay, completed.\n";
    write(new_file_descriptor, response, strlen(response));
		printf("Printed %d directory entries\n", listFiles(new_file_descriptor, "."));
	}
  memset(buffer, 0, strlen(buffer));
  return;
}

void cwd(char* buffer){
  char* query=substrings(buffer, 4, strlen(buffer));
  printf("in cwd, query:%s\n",query );
  if (strncmp(substrings(query, 0, 2), "./", 2) == 1 || strstr(query, "../") != NULL) {
		write(new_file_descriptor, "504", 3);
	}
  else {
		if (chdir(query) != 0) {
			perror("error while changing dir");
		}

    char* temp = "200 directory changed to ";
    strcat(query,"\n");
    int newSize = strlen(temp)+strlen(query)+1;
    char* response = (char *)malloc(newSize);
    strcpy(response,temp);
    strcat(response,query);
    write(new_file_descriptor, response, strlen(response));
	}
	memset(buffer, 0, strlen(buffer));
	return;
}

void cdup(char* buffer){
  char cwd[1024];
  if(strlen(buffer)>5){
    write(new_file_descriptor, "501", 3);
  }
  else{
    if(getcwd(cwd, sizeof(cwd)) == NULL){
      perror("getcwd() error");
    }
    if(!strncmp(root_dir, cwd, strlen(cwd))){
      write(new_file_descriptor, "503", 3);
    }
    else{
      if(chdir("../")!=0){
        perror("error while changing dir");
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

void client_parser(char* buffer){
  int i = 0;
  while(buffer[i] != '\0'){
    buffer[i] = toupper(buffer[i]);
    i++;
  }

  if (strncmp("USER", buffer, 4) == 0) {
    user1(buffer);
  } else {
    if (logged) {
      if (strncmp("QUIT", buffer, 4) == 0) {
        quit1(buffer);
      }
      else if (strncmp("CWD", buffer, 3) == 0) {
        printf("HERE\n");
        cwd(buffer);
      }
      else if (strncmp("CDUP", buffer, 4) == 0) {
        cdup(buffer);
      }
      // else if (strncmp("TYPE", buffer, 4) == 0) {
      //   type(buffer);
      // }
      // else if (strncmp("MODE", buffer, 4) == 0) {
      //   mode(buffer);
      // }
      // else if (strncmp("STRU", buffer, 4) == 0) {
      //   stru(buffer);
      // }
      // else if (strncmp("RETR", buffer, 4) == 0) {
      //   retr(buffer);
      // }
      // else if (strncmp("PASV", buffer, 4) == 0) {
      //   pasv(buffer);
      // }
      else if (strncmp("NLST", buffer, 4) == 0) {
        nlst(buffer);
      }
    }
    else {
      write(new_file_descriptor, "530", 3);
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
      // Send 220
      char* response="220 Service ready for new user.\n";
      write(new_file_descriptor, response, strlen(response));
      printf("220\n");
      // Reset Buffer
      memset(buffer, 0, strlen(buffer));
      if (getcwd(root_dir, sizeof(root_dir)) == NULL) {
        perror("getcwd() error");
      }
      if(read(new_file_descriptor, buffer, 255) < 0){
        perror("error reading from socket");
        exit(EXIT_FAILURE);
      }
      do{
        printf("parsing:%s\n",buffer);
        client_parser(buffer);
        memset(buffer, 0, strlen(buffer));
        if(read(new_file_descriptor, buffer, 255) < 0){
          perror("error reading from socket");
          exit(EXIT_FAILURE);
        }
        else{
          printf("READ as:%s\n", buffer);
        }

      } while(connection == 0);
      close(new_file_descriptor);
    }

      return 0;
  }
