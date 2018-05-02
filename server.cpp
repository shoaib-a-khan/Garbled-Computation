#include "includes.h"

/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
using namespace std;


int server(int portno)
{
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[256];
	ssize_t r;
	int n;
   	
	struct sockaddr_in serv_addr, cli_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		printf("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	//portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");

	listen(sockfd,5);
	clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,
				(struct sockaddr *) &cli_addr, 
				&clilen);
	if (newsockfd < 0) 
		printf("ERROR on accept");
	
	while (1) 
    	{
		bzero(buffer,256);
		n = read(newsockfd,buffer,255);
		if (n < 0) printf("ERROR reading from socket");
		printf("Here is the message: %s\n",buffer);
		n = write(sock,"I got your message",18);
		if (n < 0) printf("ERROR writing to socket");
              	sleep(1);
    	}
    	//close(newsockfd);
	//close(sockfd);
	}
	return 0; 
}
