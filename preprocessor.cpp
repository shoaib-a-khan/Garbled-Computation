#include <iostream>
#include <fstream>
#include <string>
#include "includes.h"

#define S_PARAM 1000 		//Range of Randoms
 
using namespace std;

int main () 
{
  ifstream P, D;
  ofstream P_A, P_B, D_A, D_B;
  P.open ("P.txt");
  P_A.open("P_A.txt");
  P_B.open("P_B.txt");
  
  if (P.fail())
  {
	cout << "P.fail()!\n";
	P.clear( );
  }
  if (P_A.fail())
  {
	cout << "P_A.fail()!\n";
	P_A.clear( );
  }
  if (P_B.fail())
  {
	cout << "P_B.fail()!\n";
	P_B.clear( );
  }

  int read = 0, write = 0;
  int r = 0;
  int count = 0;
  srand(time(NULL));
  
  while(P >> read)
  {
	count = ++count % 5;	
	r = rand() % S_PARAM;  	
	read = read - r;
	P_A << read << ((count == 0)?"\n": " ");
	P_B << r << ((count == 0)?"\n": " ");
  }
  P.close();
  P_A.close();
  P_B.close();
  
  count  = 0;
  D.open("D.txt");
  D_A.open("D_A.txt");
  D_B.open("D_B.txt");
  while(D >> read)
  {
	count = ++count % 2;	
	r = rand() % S_PARAM;  	
	read = read - r;
	D_A << read << ((count == 0)?"\n": " ");
	D_B << r << ((count == 0)?"\n": " ");
  }
  D.close();
  D_A.close();
  D_B.close();		
  
  return 1;

}
