/* ------------ bob -------------*/

#include "includes.h"

#define ALICE_PORT 1111
#define CAROL_PORT 2222
#define BOB_PORT 3333
#define P_SIZE 16		//program size = number of instructions
#define D_SIZE 9		//data size = number of data elements 

/* ---------- Synchronization Variables --------*/

float state_B = 0;
extern float state_A;
extern float state_C;
int send_to_Alice = 0;		//flag to signal client thread to write to socket
int send_to_Carol = 0; 		//flag to signal server thread to write to socket
int rcv_from_Alice = 0;		//flag to signal client thread to read from socket
int rcv_from_Carol = 0; 	//flag to signal server thread to read from socket
int client_rcv = 0;		//flag to signal main that client thread has recvd data
int server_rcv = 0;		//flag to signal main that servr thread has rcvd data
int terminate_client = 0; 	//flag to signal terminate to client thread
int terminate_server = 0; 	//flag to signal terminate to server thread
int done = 0;			//flag to signal end of program execution

/* ---------- P & D Global Variables --------*/
int n_p = 0;			//size of program
int n_d = 0;			//size of data
int **P_B;			//Bob's random share of program
int **D_B;			//Bob's random share of data

/* ----------- Subroutine Prototypes -----------*/
void client(char *ip, int portno, char op);
int server(int portno, char op);
int ObliviousAddition(int arg1, int arg2);
int ObliviousMultiplication(int arg1, int arg2);
int ObliviousComparison(char* op, int arg1, int arg2);
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
	int D[D_SIZE][2];
	int Pi_P[P_SIZE];
	int Pi_D[D_SIZE];
	int R_P[5*P_SIZE];
	int R_D[2*D_SIZE];
	int len;
	int scalar1 = 0;
	int scalar2 = 0;
	int scalar3 = 0;
	int scalar4 = 0;
	char src = 'B';
	char dest = 0;
	int state = 0;

}msg_for_Alice,  msg_for_Carol, msg_from_Alice, msg_from_Carol;


/* --------------- Bob Initial Setup --------------*/
void initialize()
{
	ifstream fin;
	fin.open("P_B.txt");
  	string line;
    	
	//Bob's Share of Program P''
	while (getline(fin,line))
       		++n_p;
  	fin.close();
	P_B = new int*[n_p];
  	for(int i=0; i < n_p; i++)
		P_B[i] = new int[5];
  
  	fin.open("P_B.txt");
  	for(int i=0; i < n_p; i++)
	{
		for(int j=0; j < 5; j++)
			fin >> P_B[i][j];
	 }

	fin.close();

	//Bob's Share of Data D'
	fin.open("D_B.txt");
  	while (getline(fin,line))
       		++n_d;
  	fin.close();
  	D_B = new int*[n_d];
  	for(int i=0; i < n_d; i++)
		D_B[i] = new int[2];
  
  	fin.open("D_B.txt");
  	for(int i=0; i < n_d; i++)
	{
		for(int j=0; j < 2; j++)
			fin >> D_B[i][j];
	 }
}

/* ----------- Bob's Share of Program P" -----------*/
/*int P_B[P_SIZE][5] = {{-75,100,-92, 1,0},{-718,111,-32,-365,0},
		    {61,-425,852,36,0},{192,-47,-52,6,0},
		    {0,0,0,0,0},{919,18,-332,-265,0},
		    {0,976,67,-80,0},{120,-33,0,-430,0},
		    {6,1,9,9,0},{301,63,-931,870,0},
		    {771,-366,83,-109,0},{-561,229,36,879,0},
		    {0,0,13,0,0},{-387,55,-714,-17,0},
		    {97,-693,226,-488,0},{-1,-1,-1,-1,0}};
*/
/* ----------- Bob's Share of Data D" -----------*/
//int D_B[D_SIZE][2] = {{-611,0},{25,0},{577,0},{-83,0},{-771,0},{-6,0},{37,0},{-432,0},{733,0}};


/*-------------- Main Program ----------------*/
int main(int argc, char* argv[])
{

	int curr = 0;			//index of current insturction being executed
	char ip[] = "127.0.0.1";	//localhost
	int x_B, y_B;

	//Establish connections with remote hosts i.e Bob & Carol
	std::thread server_thread(server, BOB_PORT, argv[1][0]);	//As server to Carol
	sleep(5);		//wait 5 seconds for Alice & Carol to go live
	std::thread client_thread(client, ip, ALICE_PORT, argv[1][0]);	//As client to Alice
	
	while(!done)
	{
		/*------- Step 1 -------*/
		msg_for_Alice.scalar1 = P_A[curr][0];	//Bob prepares to send a" to Alice
		msg_for_Alice.scalar2 = P_A[curr][1];	//Bob prepares to send b" to Alice
		send_to_Alice = 1;			//Signal the client thread to send msg to Bob
		rcv_from_Alice = 1;			//Singal the client thread to receive msg from Alice 
		while(send_to_Alice);			//Waiting to send a" & b" to Alice
		while(!client_rcv);			//Waiting to receive a' & b' from Alice
		client_rcv = 0;			
		
		/*------- Step 2 -------*/
		int a, b;
		a = P_B[curr][0] + msg_from_Alice.scalar1;
		b = P_B[curr][1] + msg_from_Alice.scalar2;
		
		/*------- Step 3 -------*/
		D_B[a][0] = D_B[a][0] - D_B[b][0];

		/*------- Step 4 -------*/
		int beta = ObliviousComparison(">=",0,D_B[a][0]);	
		
		/*------- Step 5 -------*/	
		while(!client_rcv);			//waiting to receive rands r_1 & r_2 from Alice
		client_rcv = 0;
		int r_1 = msg_from_Alice.scalar1;
		int r_2 = msg_from_Alice.scalar2;

		/*------- Step 6 -------*/	
		msg_for_Carol.scalar1 = P_B[curr][2];
		msg_for_Carol.scalar2 = P_B[curr][3];
		send_to_Carol = 1;
		while(send_to_Carol);

		/*------- Step 8 -------*/
		rcv_from_Carol = 1;
		while(!server_rcv);
		server_rcv = 0;

		/*------- Step 9 -------*/
		msg_for_Carol.scalar1 = beta*(msg_from_Carol.scalar1 - r_1)+
					beta*(msg_from_Carol.scalar2 - r_2);
		send_to_Carol = 1;
		while(send_to_Carol);

		/*------- Step 12 -------*/
		for(int i = 0; i < P_SIZE; i++)
		    for(int j = 0; j < 5; j++)
			msg_for_Carol.P[i][j] = P_B[i][j];
			
		for(int i = 0; i < D_SIZE; i++)
		    for(int j = 0; j < 2; j++)
			msg_for_Carol.D[i][j] = D_B[i][j];
		
		send_to_Carol = 1;
		while(send_to_Carol);
		
		/*------- Step 13 -------*/
		rcv_from_Carol = 1;
		while(!server_rcv);
		server_rcv = 0;
		
		for(int i = 0; i < P_SIZE; i++)
		    for(int j = 0; j < 5; j++)
			P_B[i][j] = msg_from_Carol.P[i][j];
			
		for(int i = 0; i < D_SIZE; i++)
		    for(int j = 0; j < 2; j++)
			D_B[i][j] = msg_from_Carol.D[i][j];

		/*------- Step 14 -------*/
		rcv_from_Alice = 1;
		while(!client_rcv);
		client_rcv = 0;
		
		/*------- Step 15 -------*/		
		for(int i = 0;  i < P_SIZE; i++)
		    for(int j = 0; j < 5; j++)
			P_B[i][j] -= msg_from_Alice.R_P[i * 5 + j];
		
		for(int i = 0; i < D_SIZE; i++)
		    for(int j = 0; j < 2; j++)
			D_B[i][j] -= msg_from_Alice.R_D[i * 2 + j];

		/*------- Step 16 -------*/
		GeneratePermutation(msg_for_Carol.Pi_P, P_SIZE);
		GeneratePermutation(msg_for_Carol.Pi_D, D_SIZE);
		GenerateRandoms(msg_for_Carol.R_P, 5*P_SIZE);
		GenerateRandoms(msg_for_Carol.R_D, 2*D_SIZE);
		send_to_Carol = 1;
		
		/*------- Step 17 -------*/
		PermuteP(msg_for_Carol.Pi_P, P_SIZE);
		ReSplitP(msg_for_Carol.R_P, P_SIZE);		
		PermuteD(msg_for_Carol.Pi_D, D_SZIE);
		ReSplitD(msg_for_Carol.R_D, D_SIZE);				
		while(send_to_Carol);

		/*------- Step 18 -------*/
		GenerateRandoms(msg_for_Alice.R_P, 5*P_SIZE);
		GenerateRandoms(msg_for_Alice.R_D, 2*D_SIZE);
		send_to_Alice = 1;
		while(send_to_Alice);

		/*------- Step 19 -------*/
		ReSplitP(msg_for_Alice.R_P, P_SIZE);
		ReSplitD(msg_for_Alice.R_D, D_SIZE);
		
		/*------- Step 20 -------*/
		GeneratePermutation(msg_for_Alice.Pi_P, P_SIZE);
		GeneratePermutation(msg_for_Alice.Pi_D, D_SIZE);
		send_to_Alice = 1;
		PermuteP(msg_for_Alice.Pi_P, P_SIZE);
		PermuteD(msg_for_Alice.Pi_D,D_SIZE);
		while(send_to_Alice);
		
				
		
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
	int p_B, p1_B, p2_B, x_B, y_B;
	x_B = arg1;
	y_B = arg2;
	p_B = x_B * y_B;	//Bob locally computes x"y"
	printf("1. Bob's share of x: x\" = %d\n", x_B);
	printf("1. Bob's share of y: y\" = %d\n", y_B);
	printf("1. Bob locally computes x\"y\"=%d\n", p_B);		

	//Preparing to initiate OMHelper to compute x'y"
	srand(time(NULL));      
	int b_1 = rand()%1000;	//Bob randomly splits y"
	int b_2 = y_B - b_1; 	
	msg_for_Alice.scalar1 = b_1; 	//Bob prepares b_1 for Alice
	msg_for_Alice.dest = 'A'; 
	msg_for_Carol.scalar1 = b_2;	//Bob prepares b_2 for Carol
	msg_for_Carol.dest = 'C'; 
		
	printf("2. Preparing to initate OMHelper to compute x'y\"\n");
	printf("2. Bob randomly splits y\"(=%d)to: b_1(=%d) + b_2(=%d)\n", y_B,b_1, b_2);		
	printf("2. Bob ---> Alice: b_1 = %d\n", msg_for_Alice.scalar1);
	printf("2. Bob ---> Carol: b_2 = %d\n", msg_for_Carol.scalar1);
	
	send_to_Alice = 1;	//Signal client thread to send b_1 to Alice
	send_to_Carol =  1;	//Signal server thread to send b_2 to Carol
	rcv_from_Alice = 1;	//Signal client thread to rcv a_1 from Alice
	while(!client_rcv);	//Waiting to recv a_1 from Alice
	client_rcv = 0;
	p1_B = b_1 * msg_from_Alice.scalar1; //computing a_1 b_1
	p1_B += b_2 * msg_from_Alice.scalar1; //computing a_1 b_1 + a_1 b_2
	rcv_from_Carol = 1;	//Signal server thread to rcv r from Carol		
	while(!server_rcv);	//Waiting to rcv r from Carol
	server_rcv = 0;
	p1_B += msg_from_Carol.scalar1; //computing a_1 b_1 + a_1 b_2 + r
			
	printf("3. Bob <--- Alice: a_1 = %d\n", msg_from_Alice.scalar1);		
	printf("3. Bob <--- Carol: r = %d\n", msg_from_Carol.scalar1);
	printf("3. Bob computes his share of x'y\": p_1\" = a_1 b_1 + a_1 b_2 +r = %d \n", p1_B);
		
		
	//Preparing to initiate OMHelper to compute x"y'
	 
	b_1 = rand()%1000;	//Bob randomly splits x"
	b_2 = x_B - b_1; 	
	msg_for_Alice.scalar1 = b_1; 	//Bob prepares b_1 for Alice
	msg_for_Alice.dest = 'A'; 
	msg_for_Carol.scalar1 = b_2;	//Bob prepares b_2 for Carol
	msg_for_Carol.dest = 'C';
		
	printf("4. Preparing to initate OMHelper to compute x\"y'\n");
	printf("4. Bobb randomly splits x\"(=%d)to: b_1(=%d) + b_2(=%d)\n", x_B,b_1, b_2);		
	printf("4. Bob ---> Alice: b_1 = %d\n", msg_for_Alice.scalar1);
	printf("4. Bob ---> Carol: b_2 = %d\n", msg_for_Carol.scalar1);

	send_to_Alice = 1;	//Signal client thread to send b_1 to Alice
	send_to_Carol =  1;	//Signal server thread to send b_2 to Carol
	rcv_from_Alice = 1;	//Signal client thread to rcv a_1 from Alice
	while(!client_rcv);	//Waiting to recv a_1 from Alice
	client_rcv = 0;
	p2_B = b_1 * msg_from_Alice.scalar1; //computing a_1 b_1
	p2_B += b_2 * msg_from_Alice.scalar1; //computing a_1 b_1 + a_1 b_2
	rcv_from_Carol = 1;	//Signal server thread to rcv r from Carol		
	while(!server_rcv);	//Waiting to rcv r from Carol
	server_rcv = 0;
	p2_B += msg_from_Carol.scalar1; //computing a_1 b_1 + a_1 b_2 + r

	printf("5. Bob <--- Alice: a_1 = %d\n", msg_from_Alice.scalar1);		
	printf("5. Bob <--- Carol: r = %d\n", msg_from_Carol.scalar1);
	printf("5. Bob computes his share of x\"y': p_2\" = a_1 b_1 + a_1 b_2 +r = %d \n", p2_B);
	
	p_B = p_B + p1_B + p2_B;	//computing p' = x"y" + p_1" + p_2"
	return p_B;	
	//printf("\n=> Bob's Share of the Final Product : %d\n", p_B);
	
}

int ObliviousComparison(char* op, int arg1, int arg2)
{
	if(op == ">=")
	{
		int r_1, r_2, r_2_B, r_3, r_3_B, r_4, sgn, p_B, q_B, x_B, y_B;
		srand(time(NULL));
		r_3 = abs(rand() % 1000);
		r_3_B = abs(rand() % 1000);		
		r_4 = abs(rand() % 1000);
		msg_for_Alice.scalar1 = r_3;
		msg_for_Alice.scalar2 = r_4;
		msg_for_Alice.scalar3 = r_3 - r_3_B;
		send_to_Alice = 1;				
		while(send_to_Alice);		
		rcv_from_Alice = 1;		
		while(!client_rcv);
		client_rcv = 0;
		sgn = msg_from_Alice.scalar1;
		r_2 = msg_from_Alice.scalar2;
		r_2_B = msg_from_Alice.scalar3;
		r_1 = abs(r_2 - r_3) + r_4;
		
		
		x_B = 2 * arg1; //Reduction from >= to >
		y_B = 2 * arg2; //Reduction from >= to >
		
		if(sgn == 1)
		{
			p_B = r_1 * x_B + r_2_B;
			q_B = r_1 * y_B + r_3_B;	
		}
		else if(sgn == -1)
		{
			q_B = r_1 * x_B + r_2_B;
			p_B = r_1 * y_B + r_3_B;
		}
		
	
		msg_for_Carol.scalar1 = p_B;
		msg_for_Carol.scalar2 = q_B;
	
		send_to_Carol = 1;
		while(send_to_Carol);
		rcv_from_Carol = 1;
		while(!server_rcv);
		server_rcv = 0;
		if(sgn == 1)	//positive logic 	
			return msg_from_Carol.scalar1;
		else 
			return (1 - msg_from_Carol.scalar1);       	
		//printf("Bob's share of the answer is: %d\n", msg_from_Carol.scalar1);
	}
	else if(op == "==")
	{
		int r_1, r_2, r_3, r_4, b_1, b_2;
		rcv_from_Alice = 1;
		while(!client_rcv);
		client_rcv = 0;
		r_1 = msg_from_Alice.scalar1;
		r_2 = msg_from_Alice.scalar2;
		b_1 = ObliviousComparison(">=", (r_1 * arg1)+r_2, (r_1 * arg2) + r_2);
		rcv_from_Alice = 1;		
		while(!client_rcv);
		client_rcv = 0;
		r_3 = msg_from_Alice.scalar1;
		r_4 = msg_from_Alice.scalar2;
		b_2 = ObliviousComparison(">=", (r_3 * arg2)+r_4, (r_3 * arg1) + r_4);
		return (b_1 + b_2);	
	}


}

void GeneratePermutation(int* arr, int size)
{
	int* flags = new int[size];
	int i,  r;	
	for(i = 0; i < size; i++)
		flags[i] = -1;	
	
	for(i = 0; i < size; )
	{
		r = rand() % size; 
		if(flags[r] == -1)
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
		temp[i][j] = P_B[i][j];
	for(i = 0; i < size; i++)
	    for(j = 0; j < 5; j++)
		P_B[pi[i]][j] = temp[i][j];
}

void ReSplitP(int* rands, int size)
{
	for(int i = 0;  i < size; i++)
		for(int j = 0; j < 5; j++)
			P_B[i][j] += rands[i * 5 + j];
}


void PermuteD(int* pi, int size)
{
	int temp[D_SIZE][2];
	int i, j;
	for(i = 0; i < size; i++)
	    for(j = 0; j < 2; j++)		
		temp[i][j] = D_B[i][j];
	for(i = 0; i < size; i++)
	    for(j = 0; j < 2; j++)
		D_B[pi[i]][j] = temp[i][j];
}

void ReSplitD(int* rands, int size)
{
	for(int i = 0; i < size; i++)
	    for(int j = 0; j < 2; j++)
		D_B[i][j] += rands[i * 2 + j];
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
		printf("ERROR opening server socket in Bob!\n");
	
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
		printf("ERROR on binding server socket in Bob!\n");
	
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,
				(struct sockaddr *) &cli_addr, 
				&clilen);
	if (newsockfd < 0) 
	{
		printf("ERROR on accept at Bob's Server!");
		return 1;
	}	
	while(!terminate_server)
	{		
		if(send_to_Carol)
		{		
			n = write(newsockfd,&msg_for_Carol,sizeof(msg_for_Carol));
			send_to_Carol = 0;
			if (n < 0) 
				printf("ERROR in Bob writing to Carol's socket!\n");
              	
		}
		if(rcv_from_Carol)
		{		
			n = read(newsockfd, &msg_from_Carol, sizeof(msg_from_Carol));
			if (n < 0) 
				printf("ERROR in Bob reading from Alice's socket!\n");
			else
			{
				server_rcv = 1;
				rcv_from_Carol = 0;
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
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		printf("ERROR opening client socket in Bob!\n");
	server = gethostbyname(ip);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host(Alice is not live!)\n");
		//exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		printf("ERROR connecting Bob to Alice!\n");
	while(!terminate_client)
	{	
		if(send_to_Alice)	
		{	
			n = write(sockfd,&msg_for_Alice, sizeof(msg_for_Alice));
			send_to_Alice = 0;
			if (n < 0) 
				printf("ERROR writing to client socket in Bob!\n");
		}
		if(rcv_from_Alice)
		{		
			n = read(sockfd, &msg_from_Alice, sizeof(msg_from_Alice));			
			if (n < 0) 
				printf("ERROR reading from client socket in Bob!\n");
			else
			{
				client_rcv = 1;
				rcv_from_Alice = 0;
			}
		}
	}
	close(sockfd);
}

