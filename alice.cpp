/* --------------- Alice -----------------------*/
#include "includes.h"

#define ALICE_PORT 1111
#define CAROL_PORT 2222
#define BOB_PORT 3333
#define P_SIZE 16		//program size = number of instructions
#define D_SIZE 9		//data size = number of data elements 
/* ---------- Synchronization Variables --------*/
float state_A = 0;
extern float state_B;
extern float state_C;
int send_to_Bob = 0;		//flag to signal server thread to write to socket
int send_to_Carol = 0; 		//flag to signal client thread to write to socket
int rcv_from_Bob = 0;		//flag to signal server thread to read from socket
int rcv_from_Carol = 0;		//flag to signal client thread to read from socket
int client_rcv = 0;		//flag to signal main that client thread has recvd data
int server_rcv = 0;		//flag to signal main that servr thread has rcvd data
int terminate_client = 0;	//flag to signal terminate to client thread
int terminate_server = 0;	//flag to signal terminate to server thread
int done = 0;			//flag to signal end of program simulation

/* ---------- P & D Global Variables --------*/
int n_p = 0;			//size of program
int n_d = 0;			//size of data
int **P_A;			//Alice's random share of program
int **D_A;			//Alice's random share of data

/* ----------- Subroutine Prototypes -----------*/
void client(char *ip, int portno, char op);
int server(int portno, char op);
int ObliviousAddition(int arg1, int arg2);
int ObliviousMultiplication(int arg1, int arg2);
int ObliviousComparison(char* op, int arg1, int arg2);
void ObliviousShuffle();
void GeneratePermutation(int* arr, int size);
void GenerateRandoms(int* arr, int size);
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
	int scalar1=0;
	int scalar2=0;
	int scalar3 = 0;
	int scalar4 = 0;
	char src= 'A';
	char dest=0;
	int state=0;

} msg_for_Bob,  msg_for_Carol, msg_from_Bob, msg_from_Carol;


/* --------------- Alice Initial Setup --------------*/
void initialize()
{
	ifstream fin;
	fin.open("P_A.txt");
  	string line;
    	
	//Alice's Share of Program P'
	while (getline(fin,line))
       		++n_p;
  	fin.close();
	P_A = new int*[n_p];
  	for(int i=0; i < n_p; i++)
		P_A[i] = new int[5];
  
  	fin.open("P_A.txt");
  	for(int i=0; i < n_p; i++)
	{
		for(int j=0; j < 5; j++)
			fin >> P_A[i][j];
	 }

	fin.close();
	
	//Alice's Share of Data D'
	fin.open("D_A.txt");
  	while (getline(fin,line))
       		++n_d;
  	fin.close();
  	D_A = new int*[n_d];
  	for(int i=0; i < n_d; i++)
		D_A[i] = new int[2];
  
  	fin.open("D_A.txt");
  	for(int i=0; i < n_d; i++)
	{
		for(int j=0; j < 2; j++)
			fin >> D_A[i][j];
	}
	fin.close();
}



/*int P_A[P_SIZE][5] = {{77,-100,93,2,0},{721,-111,35,367,1},
		{-59,426,-849,-33,2},{-188,47,56,0,3},
		{5,0,6,5,4},{-915,-17,338, 271,5},
		{6,-976,-60,89,6},{-113,33,9, 438,7},
		{0,0,0,0,8},{-295,-63, 943,-860,9},
		{-763,366,-72,121,10},{567,-223,-24,-868,11},
		{2,0,0,15,12},{391,-55,728,32,13},
		{-91,693,-211,503,14},{0,0,0,0,15}};	

*/
/* ----------- Alice's Share of Data D' -----------*/
//int D_A[D_SIZE][2] = {{611,0},{-26,1},{-576,2},{83,3},{771,4},{7,5},{-36,6},{432,7},{-733,8}};	



/*-------------- Main Program ----------------*/

int main(int argc, char* argv[])
{
		
	int curr = 0;			//index of current insturction being executed
	char ip[] = "127.0.0.1";	//localhost
	int x_A, y_A;
	
	//Establish connections with remote hosts i.e Bob & Carol
	std::thread server_thread(server, ALICE_PORT, argv[1][0] );	//Alice as server to Bob
	sleep(5);							//wait 5 seconds for Bob and Carol to go live
	std::thread client_thread(client, ip, CAROL_PORT, argv[1][0]);	//Alice as client to Carol

	while(!done)
	{	
		/*------- Step 1 -------*/		
		msg_for_Bob.scalar1 = P_A[curr][0];	//Alice prepares to send a' to Bob
		msg_for_Bob.scalar2 = P_A[curr][1];	//Alice prepares to send b' to Bob
		send_to_Bob = 1;			//Signal the server thread to send msg to Bob
		rcv_from_Bob = 1;			//Singal the server thread to receive msg from Bob 
		while(send_to_Bob);			//Waiting to send a' & b' to Bob
		while(!server_rcv);			//Waiting to receive a" & b" from Bob
		server_rcv = 0;			
		
		/*------- Step 2 -------*/
		int a, b;
		a = P_A[curr][0] + msg_from_Bob.scalar1;	//Alice computes a = a' + a"
		b = P_A[curr][1] + msg_from_Bob.scalar2;	//Alice computes b = b' + b"
		
		/*------- Step 3 -------*/
		D_A[a][0] = D_A[a][0] - D_A[b][0];		//Alice computers D'[a]:= D[a]-D[b]
		
		/*------- Step 4 -------*/
		int beta = ObliviousComparison(">=",0,D_A[a][0]);	//Alice & Bob run OC(>=) with Carol's help
		
		/*------- Step 5 -------*/		
		srand(time(NULL));		
		int r_1 = rand() % 1000;
		int r_2 = rand() % 1000;
		msg_for_Bob.scalar1 = r_1;
		msg_for_Bob.scalar2 = r_2;
		send_to_Bob = 1;
		while(send_to_Bob);

		/*------- Step 6 -------*/	
		msg_for_Carol.scalar1 = P_A[curr][2] + r_1;
		msg_for_Carol.scalar2 = P_A[curr][3] + r_2;
		send_to_Carol = 1;
		while(send_to_Carol);
		
		/*------- Step 8 -------*/
		rcv_from_Carol = 1;
		while(!client_rcv);
		client_rcv = 0;	

		/*------- Step 9 -------*/
		msg_for_Carol.scalar1 = beta*(msg_from_Carol.scalar1 - r_1)+
					(1-beta)*(msg_from_Carol.scalar2 - r_2);
		send_to_Carol = 1;
		while(send_to_Carol);
		
		/*------- Step 11 -------*/
		GeneratePermutation(msg_for_Carol.Pi_P, P_SIZE);
		GeneratePermutation(msg_for_Carol.Pi_D, D_SIZE);
		GenerateRandoms(msg_for_Carol.R_P, 5*P_SIZE);
		GenerateRandoms(msg_for_Carol.R_D, 2*D_SIZE);
		send_to_Carol = 1;
		
		/*------- Step 13 -------*/
		PermuteP(msg_for_Carol.Pi_P, P_SIZE);
		ReSplitP(msg_for_Carol.R_P, P_SIZE);		
		PermuteD(msg_for_Carol.Pi_D, D_SZIE);
		ReSplitD(msg_for_Carol.R_D, D_SIZE);				
		while(send_to_Carol);

		/*------- Step 14 -------*/
		GenerateRandoms(msg_for_Bob.R_P, 5*P_SIZE);
		GenerateRandoms(msg_for_Bob.R_D, 2*D_SIZE);
		send_to_Bob = 1;
		while(send_to_Bob);

		/*------- Step 15 -------*/
		ReSplitP(msg_for_Bob.R_P, P_SIZE);
		ReSplitD(msg_for_Bob.R_D, D_SIZE);
		
		/*------- Step 17 -------*/
		for(int i = 0; i < P_SIZE; i++)
		    for(int j = 0; j < 5; j++)
			msg_for_Carol.P[i][j] = P_A[i][j];
			
		for(int i = 0; i < D_SIZE; i++)
		    for(int j = 0; j < 2; j++)
			msg_for_Carol.D[i][j] = D_A[i][j];
		
		send_to_Carol = 1;
		while(send_to_Carol);
		
		/*------- Step 18 -------*/
		rcv_from_Carol = 1;
		while(!client_rcv);
		client_rcv = 0;
		
		for(int i = 0; i < P_SIZE; i++)
		    for(int j = 0; j < 5; j++)
			P_A[i][j] = msg_from_Carol.P[i][j];
			
		for(int i = 0; i < D_SIZE; i++)
		    for(int j = 0; j < 2; j++)
			D_A[i][j] = msg_from_Carol.D[i][j];

		/*------- Step 19 -------*/
		rcv_from_Bob = 1;
		while(!server_rcv);
		server_rcv = 0;
		
		for(int i = 0;  i < P_SIZE; i++)
		    for(int j = 0; j < 5; j++)
			P_A[i][j] -= msg_from_Bob.R_P[i * 5 + j];
		
		for(int i = 0; i < D_SIZE; i++)
		    for(int j = 0; j < 2; j++)
			D_A[i][j] -= msg_from_Bob.R_D[i * 2 + j];

		/*------- Step 20 -------*/
		rcv_from_Bob = 1;
		while(!server_rcv);
		server_rcv = 0;
		PermuteP(msg_from_Bob.Pi_P, P_SIZE);
		PermuteD(msg_from_Bob.Pi_D, D_SIZE);



	}	
	terminate_client = 1;
	terminate_server = 1;
	server_thread.join();
	client_thread.join();
		
	return 1;

}


/*---------------- Subroutines ----------------*/
int ObliviousAddition(int arg1, int arg2)
{
	return (arg1 + arg2);
}

int ObliviousMultiplication(int arg1, int arg2)
{
	int p_A, p1_A, p2_A, x_A, y_A;
	int a_1, a_2;
	x_A = arg1;
	y_A = arg2;
	p_A = x_A * y_A;		//Alice locally computes x'y'
	/*		
	printf("1. Alice's share of x: x' = %d\n", x_A);
	printf("1. Alice's share of y: y' = %d\n", y_A);
	printf("1. Alice locally computes x'y'=%d\n", p_A);		
	*/
	//Preparing to initiate OMHelper to compute x'y"
	srand(time(NULL));      
	a_1 = rand()%1000;		//Alice randomly splits x'
	a_2 = x_A - a_1; 	
	msg_for_Bob.scalar1 = a_1; 	//Alice prepares a_1 for Bob
	msg_for_Bob.dest = 'B'; 
	msg_for_Carol.scalar1 = a_2;	//Alice prepares a_2 for Carol
	msg_for_Carol.dest = 'C';
	/*
	printf("2. Preparing to initate OMHelper to compute x'y\"\n");
	printf("2. Alice randomly splits x'(=%d)to: a_1(=%d) + a_2(=%d)\n", x_A,a_1, a_2);		
	printf("2. Alice ---> Bob: a_1 = %d\n", msg_for_Bob.scalar1);
	printf("2. Alice ---> Carol: a_2 = %d\n", msg_for_Carol.scalar1);
	*/			
	send_to_Bob = 1;	//Signal server thread to send a_1 to Bob
	send_to_Carol =  1;	//Signal client thread to send a_2 to Carol
	while(send_to_Bob);	//Waiting to send a_1 to Bob
	while(send_to_Carol);	//Waiting to send a_2 to Carol
	rcv_from_Bob = 1;	//Signal server thread to rcv b_1 from Bob
	rcv_from_Carol = 1;	//Signal client thread to rcv (a_2 b_2 - r) from Carol	
	while(!server_rcv);	//Waiting to recv b_1 from Bob
	server_rcv = 0;
	p1_A = a_2 * msg_from_Bob.scalar1; //computing a_2 b_1
	while(!client_rcv);	//Waiting to rcv (a_2 b_2 - r) from Carol
	client_rcv = 0;
	p1_A += msg_from_Carol.scalar1; //computing a_2 b_1 + a_2 b_2 - r
	/*
	printf("3. Alice <--- Bob: b_1 = %d\n", msg_from_Bob.scalar1);		
	printf("3. Alice <--- Carol: (a_2 b_2 - r) = %d\n", msg_from_Carol.scalar1);
	printf("3. Alice computes her share of x'y\": p_1' = a_2 b_1 + a_2 b_2 -r = %d \n", p1_A);
	*/
	//Preparing to initiate OMHelper to compute x"y'
	a_1 = rand()%1000;	//Alice randomly splits y'
	a_2 = y_A - a_1; 	
	msg_for_Bob.scalar1 = a_1; 	//Alice prepares a_1 for Bob
	msg_for_Bob.dest = 'B'; 
	msg_for_Carol.scalar1 = a_2;	//Alice prepares a_2 for Carol
	msg_for_Carol.dest = 'C';
	/*	
	printf("4. Preparing to initate OMHelper to compute x\"y'\n");
	printf("4. Alice randomly splits y'(=%d)to: a_1(=%d) + a_2(=%d)\n", y_A,a_1, a_2);		
	printf("4. Alice ---> Bob: a_1 = %d\n", msg_for_Bob.scalar1);
	printf("4. Alice ---> Carol: a_2 = %d\n", msg_for_Carol.scalar1);
	*/
	send_to_Bob = 1;	//Signal server thread to send a_1 to Bob
	send_to_Carol =  1;	//Signal client thread to send a_2 to Carol
	while(send_to_Bob);	//Waiting to send a_1 to Bob
	while(send_to_Carol);	//Waiting to send a_2 to Carol
	rcv_from_Bob = 1;	//Signal server thread to rcv b_1 from Bob
	rcv_from_Carol = 1;	//Signal client thread to rcv (a_2 b_2 - r) from Carol		
	while(!server_rcv);	//Waiting to recv b_1 from Bob
	server_rcv = 0;
	p2_A = a_2 * msg_from_Bob.scalar1; //computing a_2 b_1
	while(!client_rcv);	//Waiting to recv (a_2 b_2 - r) from Carol
	client_rcv = 0;
	p2_A += msg_from_Carol.scalar1; //computing a_2 b_1 + a_2 b_2 - r
	/*
	printf("5. Alice <--- Bob: b_1 = %d\n", msg_from_Bob.scalar1);		
	printf("5. Alice <--- Carol: (a_2 b_2 - r) = %d\n", msg_from_Carol.scalar1);
	printf("5. Alice computes her share of x\"y': p_2' = a_2 b_1 + a_2 b_2 -r = %d \n", p2_A);
	*/	
	p_A = p_A + p1_A + p2_A;	//computing p' = x'y' + p_1' + p_2'
	return p_A			
	//printf("\n=> Alice's Share of the Final Product = %d\n", p_A);
				
}


int ObliviousComparison(char* op, int arg1, int arg2)
{
	if(op == ">=")
	{	
		int sgn, r_1, r_2, r_2_A, r_3, r_3_A, r_4, p_A, q_A, x_A, y_A;
        	srand(time(NULL));
	        sgn = rand() % 1000;
        	sgn = sgn > 0 ? 1 : -1; //Alice decides whether we use positive or negative logic
		r_2 = abs(rand() % 1000);
		r_2_A = abs(rand() % 1000);
		msg_for_Bob.scalar1 = sgn;
		msg_for_Bob.scalar2 = r_2;
		msg_for_Bob.scalar2 = r_2 - r_2_A;
        	send_to_Bob = 1;
	       	while(send_to_Bob);
		rcv_from_Bob = 1;
		while(!server_rcv);
		server_rcv = 0;
		r_3 = msg_from_Bob.scalar1;
		r_4 = msg_from_Bob.scalar2;
		r_3_A = msg_from_Bob.scalar3;
		r_1 = abs(r_2 - r_3) + r_4;
		
		x_A = 2 * (arg1 + 1); 	    //Reduction of >= to >
        	y_A = 2 * arg2; 	    //Reduction of >= to >

        	if(sgn == 1)
		{
			p_A = r_1 * x_A + r_2_A;
			q_A = r_1 * y_A + r_3_A;	
		}
		else if(sgn == -1)
		{
			q_A = r_1 * x_A + r_2_A;
			p_A = r_1 * y_A + r_3_A;
		}
		
	
		msg_for_Carol.scalar1 = p_A;
		msg_for_Carol.scalar2 = q_A;
	
		send_to_Carol = 1;
		while(send_to_Carol);
		rcv_from_Carol = 1;
		while(!client_rcv);
		client_rcv = 0;
		
		if(sgn == 1)	//positive logic 	
			return msg_from_Carol.scalar1;
		else 
			return -(msg_from_Carol.scalar1);       	
		//printf("Alice's share of the answer is: %d\n", msg_from_Carol.scalar1);
	}
	else if(op == "==")
	{
		int r_1, r_2, r_3, r_4, b_1, b_2;
		srand(time(NULL));
		r_1 = rand() % 1000;
		r_2 = rand() % 1000;
		msg_for_Bob.scalar1 = r_1;
		msg_for_Bob.scalar2 = r_2;
		send_to_Bob = 1;
		while(send_to_Bob);
		b_1 = ObliviousComparison(">=", (r_1 * arg1)+r_2, (r_1 * arg2) + r_2);
		r_3 = rand() % 1000;
		r_4 = rand() % 1000;
		msg_for_Bob.scalar1 = r_3;
		msg_for_Bob.scalar2 = r_4;
		send_to_Bob = 1;
		while(send_to_Bob);
		b_2 = ObliviousComparison(">=", (r_3 * arg2)+r_4, (r_3 * arg1) + r_4);
		return (b_1 + b_2 -1);	
	}
}


void GeneratePermutation(int* arr, int size)
{
	int* flags = new int[size];
	int i,  r;	
	for(i = 0; i < size; i++)
		flags[i] = 0;	
	
	for(i = 0; i < size; )
	{
		r = rand() % size; 
		if(flags[r] == 0)
		{
			flags[r] = 1;
			arr[i] = r;
			i++;
		}
	}
	delete flags;	
}

void GenerateRandoms(int* arr,int size)
{
	for(int i = 0; i < size; i++)
		arr[i] = rand() % 1000;
}

void PermuteP(int* pi, int size)
{
	int temp[P_SIZE][5];
	int i, j;
	for(i = 0; i < size; i++)
	    for(j = 0; j < 5; j++)
		temp[i][j] = P_A[i][j];
	for(i = 0; i < size; i++)
	    for(j = 0; j < 5; j++)
		P_A[pi[i]][j] = temp[i][j];
}

void ReSplitP(int* rands, int size)
{
	for(int i = 0;  i < size; i++)
		for(int j = 0; j < 5; j++)
			P_A[i][j] += rands[i * 5 + j];
}


void PermuteD(int* pi, int size)
{
	int temp[D_SIZE][2];
	int i, j;
	for(i = 0; i < size; i++)
	    for(j = 0; j < 2; j++)		
		temp[i][j] = D_A[i][j];
	for(i = 0; i < size; i++)
	    for(j = 0; j < 2; j++)
		D_A[pi[i]][j] = temp[i][j];
}

void ReSplitD(int* rands, int size)
{
	for(int i = 0; i < size; i++)
	    for(int j = 0; j < 2; j++)
		D_A[i][j] += rands[i * 2 + j];
}


int server(int portno, char op)
{
	int sockfd, newsockfd;
	socklen_t clilen;
	char buffer[256];
	ssize_t r;
	int n;
   	
	struct sockaddr_in serv_addr, cli_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		printf("ERROR opening server socket in Alice!\n");
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
                printf("setsockopt(SO_REUSEADDR) failed");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		printf("ERROR on binding server socket in Alice!\n");

	listen(sockfd,5);
	clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,
				(struct sockaddr *) &cli_addr, 
				&clilen);
	if (newsockfd < 0) 
	{
		printf("ERROR on accept at Alice's Server!\n");
		return 1;
	}	
	while(!terminate_server)
	{		
		if(send_to_Bob)
		{		
			n = write(newsockfd,&msg_for_Bob,sizeof(msg_for_Bob));
			send_to_Bob = 0;
			if (n < 0) 
				printf("ERROR in Alice writing to Bob's socket!\n");
              	
		}
		if(rcv_from_Bob)
		{		
			n = read(newsockfd, &msg_from_Bob, sizeof(msg_from_Bob));
			if (n < 0) 
				printf("ERROR in Alice reading from Bob's socket!\n");
			else
			{
				server_rcv = 1;
				rcv_from_Bob = 0;
			}	
		
		}			
			
		//sleep(1);
	}
    	close(newsockfd);
	close(sockfd);
	
	return 0; 
}

void client(char *ip, int portno, char op)
{
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	//portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		printf("ERROR opening client socket in Alice!\n");
	server = gethostbyname(ip);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host(Carol is not live)!\n");
		//exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		printf("ERROR connecting Alice to Carol!\n");
	
	while(!terminate_client)
	{	
		if(send_to_Carol)	
		{	
			n = write(sockfd,&msg_for_Carol, sizeof(msg_for_Carol));
			send_to_Carol = 0;
			if (n < 0) 
				printf("ERROR writing to client socket in Alice!\n");
		}
		if(rcv_from_Carol)
		{		
			n = read(sockfd, &msg_from_Carol, sizeof(msg_from_Carol));
			if (n < 0) 
				printf("ERROR reading from client socket in Alice!\n");
			else{
	 
				client_rcv = 1;
				rcv_from_Carol = 0;
			}
		}	
	}

	close(sockfd);
}

