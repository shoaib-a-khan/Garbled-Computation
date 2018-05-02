/* ------------ carol -------------*/

#include "includes.h"

#define ALICE_PORT 1111
#define CAROL_PORT 2222
#define BOB_PORT 3333
#define P_SIZE 16		//program size = number of instructions
#define D_SIZE 9		//data size = number of data elements 
using namespace std;

/* ---------- Synchronization Variables --------*/
float state_C = 0;
extern float state_A;
extern float state_B;
int send_to_Alice = 0;		//flag to signal server thread to write to socket
int send_to_Bob = 0; 		//flag to signal client thread to write to socket
int rcv_from_Alice = 0;		//flag to signal server thread to read from socket
int rcv_from_Bob = 0; 		//flag to signal client thread to read from socket
int client_rcv = 0;		//flag to signal main that client thread has recvd data
int server_rcv = 0;		//flag to signal main that servr thread has rcvd data
int terminate_client = 0; 	//flag to signal terminate to client thread
int terminate_server = 0; 	//flag to signal terminate to server thread
int done = 0;			//flag to signal end of program execution


/* ----------- Subroutine Prototypes -----------*/
void client(char *ip, int portno, char op);
int server(int portno, char op);
int ObliviousAddition(int arg1, int arg2);
int ObliviousMultiplication(int arg1, int arg2);
int ObliviousComparison(char* op);
void ObliviousShuffle();
void PermuteP(int* pi, int size);
void PermuteD(int* pi, int size);
void ReSplitP(int* rands, int size);
void ReSplitD(int* rands, int size);



/* ----------- Abstract Data Types -------------*/
struct message
{
	int P[P_SIZE][5];
	int D[D_SIZE];
	int Pi_P[P_SIZE];
	int Pi_D[D_SIZE];
	int R_P[5*P_SIZE];
	int R_D[2*D_SIZE];
	int len;
	int scalar1 = 0;
	int scalar2 = 0;
	int scalar3 = 0;
	int scalar4 = 0;
	char src= 'C';
	char dest=0;
	int state=0;
	
}msg_for_Alice,  msg_for_Bob, msg_from_Alice, msg_from_Bob;

/* ----------- Globals -------------*/
int p_A, p_B, q_A, q_B;
int P_C[P_SIZE][5];
int D_C[D_SIZE][2];




/*-------------- Main Program ----------------*/
int main(int argc, char* argv[])
{
	int next, p, q, t1, t2, r_3;
	char ip[] = "127.0.0.1";
	
	std::thread server_thread(server, CAROL_PORT, op); //server to Alice
	sleep(5);	
	std::thread client_thread(client, ip, BOB_PORT, op);
	srand(time(NULL));

	while(!done)
	{
		/*------- Step 4 -------*/	
		ObliviousComparison(">=");

		/*------- Step 7 -------*/
		rcv_from_Alice = 1;
		rcv_from_Bob = 1;		
		while(!server_rcv);
		server_rcv = 0;
		t1 = msg_from_Alice.scalar1;
		t2 = msg_from_Alice.scalar2;
		r_3 = rand() % 1000;
		msg_for_Alice.scalar1 = t1 + r_3;
		msg_for_Alice.scalar2 = t2 + r_3;
		send_to_Alice = 1;
		while(!client_rcv);
		client_rcv = 0;
		t1 = msg_from_Bob.scalar1;
		t2 = msg_from_Bob.scalar2;
		msg_for_Bob.scalar1 = t1 + r_3;
		msg_for_Bob.scalar2 = t2 + r_3;
		send_to_Bob = 1;
		while(send_to_Alice || send_to_Bob);
		
		/*------- Step 10 -------*/
		rcv_from_Alice = 1;
		rcv_from_Bob = 1;
		while(!server_rcv);
		server_rcv = 0;
		t1 = msg_from_Alice.scalar1;
		while(!client_rcv);
		client_rcv = 0;
		t2 = msg_from_Bob.scalar1;
		next = t1 + t2 - r_3;

		/*------- Step 11 -------*/
		ObliviousShuffle();

		
		




		/*case 'x': //Carol in multiplication (helper) protocol
			while(client_done != 1 || server_done != 1);
			client_done = 0;
			server_done = 0;
			srand(0);				
			q_B = rand()%1000;
			q_A = p_A * p_B - q_B;
			main_done_1 = 1;

			while(client_done != 1 || server_done != 1);
			client_done = 0;
			server_done = 0;				
			q_B = rand()%1000;
			q_A = p_A * p_B - q_B;
			main_done_2 = 1;
			break;

		case '>':
                        while(client_done != 1 || server_done != 1);
                        client_done = 0;
                        server_done = 0;
                        p = p_A + p_B;
                        q = q_A + q_B;
                        printf("Received %d and %d from Alice\n", p_A, q_A);
                        printf("Received %d and %d from Bob\n", p_B, q_B);
                        srand(time(NULL));
                        send_A = rand() % 1000;
                        send_B = p > q ? 1 - send_A : -1 * send_A;
                        p_A = send_A;
                        p_B = send_B;
                        main_done_1 = 1;

                        break;		
		default:
			printf("Operation not permitted.\n");
			break;*/
	}

	server_thread.join();
	client_thread.join();
	
	return 1;

}


/*---------------- Subroutines ----------------*/
int Oblivious Addition()
{
	return 1;
}


int ObliviousMultiplication()
{

}

int ObliviousComparison(char* op)
{
	int p, q, p_A, q_A, p_B, q_B, b_A, b_B;
	srand(time(NULL));
	if(op == ">=")
	{
		rcv_from_Alice = 1;
		rcv_from_Bob = 1;		
		while(!server_rcv);
		server_rcv = 0;
		p_A = msg_from_Alice.scalar1;
		q_A = msg_from_Alice.scalar2;
		while(!client_rcv);
		client_rcv = 0;		
		p_B = msg_from_Bob.scalar1;
		q_B = msg_from_Bob.scalar2;
		p = p_A + p_B;
		q = q_A + q_B;
		if(p-q > 0)
		{
			b_A = rand() % 1000;
			b_B = 1 - b_A;
		}
		else
		{	
			b_A = rand() % 1000;
			b_B = 0 - b_A;
		}
		msg_for_Alice.scalar1 = b_A;
		msg_for_Bob.scalar1 = b_B;
		send_to_Alice = 1;
		send_to_Bob = 1;
		while(send_to_Alice || send_to_Bob);
	}
	else if(op == "==")
	{	
	
	}
}

void ObliviousShuffle()
{
	/*------------------Pi_AC------------------*/
	rcv_from_Alice = 1;
	rcv_from_Bob = 1;	
	while(!server_rcv);		//receive permutations Pi_AC and randoms R_AC from Alice for both P and D 
	server_rcv = 0;
	while(!client_rcv);		//receive P'' & D'' from Bob
	client_rcv = 0;
	
	for(int i=0; i < P_SIZE; i++)
		for(int j=0; j < 5; j++)
			P_C[i][j] = msg_from_Bob.P[i][j];	//make local copy of P''
	for(int i=0; i < D_SIZE; i++)
		for(int j=0; j < 2; j++)
			D_C[i][j] = msg_from_Bob.D[i][j];	//make local copy of D''

	PermuteP(msg_from_Alice.Pi_P, P_SIZE);		//Apply Pi_AC for Program to P''s local copy
	ResplitP(msg_from_Alice.R_P, P_SIZE);		//Resplit P''s local copy using R_AC for Program
	PermuteD(msg_from_Alice.Pi_D, D_SIZE);		//Apply Pi_AC for Data to D''s local copy
	ResplitD(msg_from_Alice.R_D, D_SIZE);		//Resplit D''s local copy using R_AC for Data
	
	for(int i=0; i < P_SIZE; i++)
		for(int j=0; j < 5; j++)
			msg_for_Bob.P[i][j] = P_C[i][j];	//prepare to send permuted & resplit P'' back to Bob
	for(int i=0; i < D_SIZE; i++)
		for(int j=0; j < 2; j++)
			msg_for_Bob.D[i][j] = D_C[i][j];	//prepare to send permuted & resplit D'' back to Bob
	send_to_Bob = 1;
	while(send_to_Bob);
	
	/*------------------Pi_BC------------------*/
	rcv_from_Alice = 1;
	rcv_from_Bob = 1;	
	while(!server_rcv);		//receive P' & D' from Alice
	server_rcv = 0;
	while(!client_rcv);		//receive permutations Pi_BC and randoms R_BC from Bob for both P and D 
	client_rcv = 0;
	
	for(int i=0; i < P_SIZE; i++)
		for(int j=0; j < 5; j++)
			P_C[i][j] = msg_from_Alice.P[i][j];	//make local copy of P'
	for(int i=0; i < D_SIZE; i++)
		for(int j=0; j < 2; j++)
			D_C[i][j] = msg_from_Alice.D[i][j];	//make local copy of D'

	PermuteP(msg_from_Bob.Pi_P, P_SIZE);		//Apply Pi_BC for Program to P's local copy
	ResplitP(msg_from_Bob.R_P, P_SIZE);		//Resplit P's local copy using R_BC for Program
	PermuteD(msg_from_Bob.Pi_D, D_SIZE);		//Apply Pi_BC for Data to D's local copy
	ResplitD(msg_from_Bob.R_D, D_SIZE);		//Resplit D's local copy using R_BC for Data
	
	for(int i=0; i < P_SIZE; i++)
		for(int j=0; j < 5; j++)
			msg_for_Alice.P[i][j] = P_C[i][j];	//prepare to send permuted & resplit P' back to Alice
	for(int i=0; i < D_SIZE; i++)
		for(int j=0; j < 2; j++)
			msg_for_Alice.D[i][j] = D_C[i][j];	//prepare to send permuted & resplit D' back to Alice
	send_to_Alice = 1;
	while(send_to_Alice);	
	

}

void PermuteP(int* pi, int size)
{
	int temp[P_SIZE][5];
	int i, j;
	for(i = 0; i < size; i++)
	    for(j = 0; j < 5; j++)
		temp[i][j] = P_C[i][j];
	for(i = 0; i < size; i++)
	    for(j = 0; j < 5; j++)
		P_C[pi[i]][j] = temp[i][j];
}

void ReSplitP(int* rands, int size)
{
	for(int i = 0;  i < size; i++)
		for(int j = 0; j < 5; j++)
			P_C[i][j] -= rands[i * 5 + j];
}


void PermuteD(int* pi, int size)
{
	int temp[D_SIZE][2];
	int i, j;
	for(i = 0; i < size; i++)
	    for(j = 0; j < 2; j++)		
		temp[i][j] = D_C[i][j];
	for(i = 0; i < size; i++)
	    for(j = 0; j < 2; j++)
		D_C[pi[i]][j] = temp[i][j];
}

void ReSplitD(int* rands, int size)
{
	for(int i = 0; i < size; i++)
	    for(int j = 0; j < 2; j++)
		D_C[i][j] -= rands[i * 2 + j];
}



/*Client of Bob */
void client(char *ip, int portno, char op)
{

	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	struct message* buffer = (struct message *)malloc(sizeof(struct message));
	//portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		printf("ERROR opening socket");
	server = gethostbyname(ip);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		//exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		printf("ERROR connecting");

	while(!terminate_client)
	{	
		if(send_to_Bob)	
		{	
			n = write(sockfd,&msg_for_Bob, sizeof(msg_for_Bob));
			send_to_Bob = 0;
			if (n < 0) 
				printf("ERROR writing to client socket in Carol!\n");
		}
		if(rcv_from_Bob)
		{		
			n = read(sockfd, &msg_from_Bob, sizeof(msg_from_Bob));			
			if (n < 0) 
				printf("ERROR reading from client socket in Carol!\n");
			else
			{
				client_rcv = 1;
				rcv_from_Bob = 0;
			}
		}
	}
	close(sockfd);
}

/*Server to Alice */
int server(int portno, char op)
{
	std::thread t;
	int n;
	int sockfd, newsockfd;
	socklen_t clilen;

	struct sockaddr_in serv_addr, cli_addr;
	struct message* buffer = (struct message *)malloc(sizeof(struct message));
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		printf("ERROR opening socket");
	
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
                printf("setsockopt(SO_REUSEADDR) failed");
	
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
	{
		printf("ERROR on accept at Carol's Server!");
		return 1;
	}	
	while(!terminate_server)
	{		
		if(send_to_Alice)
		{		
			n = write(newsockfd,&msg_for_Alice,sizeof(msg_for_Alice));
			send_to_Alice = 0;
			if (n < 0) 
				printf("ERROR in Carol writing to server socket!\n");
              	
		}
		if(rcv_from_Alice)
		{		
			n = read(newsockfd, &msg_from_Alice, sizeof(msg_from_Alice));
			if (n < 0) 
				printf("ERROR in Carol reading from server socket!\n");
			else
			{
				server_rcv = 1;
				rcv_from_Alice  = 0;
			}	
		
		}			
			
	}

	/*switch(op)
	{
		case 'x': 
			/* read a_2
			//bzero(buffer, sizeof(int));
			n = read(newsockfd,buffer,sizeof(struct message));
			printf("Finished reading. Receieved %d from A\n", buffer->scalar1);
			p_A = buffer->scalar1;		
			server_done = 1;	
			while(!main_done_1);
			buffer->scalar1 = q_A;
			n = write(newsockfd, buffer, sizeof(message));

			//bzero(buffer, sizeof(int));
			n = read(newsockfd, buffer,sizeof(struct message));
			p_A = buffer->scalar1;		
			server_done = 1;	
			while(!main_done_2);
			buffer->scalar1 = q_A;
			n = write(newsockfd, buffer, sizeof(struct message));
			break;
		case '>':
                        n = read(newsockfd,buffer,sizeof(struct message));
                        p_A = buffer->scalar1;
                        q_A = buffer->scalar2;
                        server_done = 1;
                        while(!main_done_1);
                        buffer->scalar1 = p_A;
                        buffer->scalar2 = 0;
                        n = write(newsockfd, buffer, sizeof(struct message));

		default:
			break;

	}*/

	return 0; 
}
