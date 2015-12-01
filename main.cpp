#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <sstream>
#include <fstream>

#include <math.h>

//#include "class_definitions.cpp"

using namespace std;

int main(int argc, char *argv[])
{
  string trace_file;
  char pc_str[10];
  unsigned long pc;
  int opcode;
  int dr;  //destination register
  int sr1, sr2; //source register 1 and 2. using int because value can be -1 i.e no reg

  unsigned int ROB_size;
  unsigned int IQ_size;
  unsigned int width;
  int total_reads = 0;

  if(argc != 5 ) //TODO
  {
    cout<<"invalid number of arguments"<<endl;
  }

  FILE *pFile;
  char *fname = (char *)malloc(30);
  
  ROB_size = atoi(argv[1]);
  IQ_size = atoi(argv[2]);
  width = atoi(argv[3]);
  fname = argv[4];
  cout<<"ROB size: "<<ROB_size<<endl;
  cout<<"IQ size: "<<IQ_size<<endl;
  cout<<"WIDTH: "<<width<<endl;
  cout<<"Trace file: "<<fname<<endl;

  pFile = fopen(fname,"r");
  if(pFile == 0)
  {
    cout<<"Trace file problem"<<endl;
    exit(0);
  }
  while(fscanf(pFile,"%s %d %d %d %d",&pc_str,&opcode,&dr,&sr1,&sr2))
  {
    if(feof(pFile))break;
    total_reads++;
    pc = strtoul(pc_str,0,16);
    cout<<hex<<pc<<" "<<dec<<opcode<<" "<<dr<<" "<<sr1<<" "<<sr2<<endl;



  }
  cout<<"total reads: "<<dec<<total_reads<<endl;
}

  
