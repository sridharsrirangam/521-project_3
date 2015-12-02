#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <sstream>
#include <fstream>

#include <math.h>

#include "class_definitions.h"

using namespace std;

int fetch_age_cycle=0;

unsigned int ROB_size;
unsigned int IQ_size;
unsigned int width;
FILE *pFile;
  
instruction_bundle i_b;

instruction_bundle *DE;
instruction_bundle *RN;

execution_list *exc_lst;

ROB_table rob;
RMT_block rmt[67];
ISSUE_queue IQ;

int DE_empty, RN_empty;

int main(int argc, char *argv[])
{
  string trace_file;
  char pc_str[10];
  unsigned long pc;
  int opcode;
  int dr;  //destination register
  int sr1, sr2; //source register 1 and 2. using int because value can be -1 i.e no reg

  int total_reads = 0;
  void Fetch();

  if(argc != 5 ) //TODO
  {
    cout<<"invalid number of arguments"<<endl;
  }

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
  
  i_b.instruction_bundle_c(width);

  rob.ROB_table_c(ROB_size);
 
  
  IQ.ISSUE_queue_c(IQ_size);

  exc_lst = new execution_list[5*width];

 //while(fscanf(pFile,"%s %d %d %d %d",&pc_str,&opcode,&dr,&sr1,&sr2))
  while(1)
  {
    if(feof(pFile))break;
    total_reads++;
    pc = strtoul(pc_str,0,16);
   // cout<<hex<<pc<<" "<<dec<<opcode<<" "<<dr<<" "<<sr1<<" "<<sr2<<endl;
    Fetch();


  }
  cout<<"total reads: "<<dec<<total_reads<<endl;

}//end of main

void Fetch()
{
  unsigned long pc;
  int opcode;
  int dr;
  int sr1,sr2;
  char pc_str[10];

  //if((DE_not_empty)&&(! feof(pFile))) //set to 1 if DE is not empty
  if(! feof(pFile)) //set to 1 if DE is not empty
  {
   for(int i=0;i<width;i++)
   {
    fscanf(pFile,"%s %d %d %d %d",&pc_str,&opcode,&dr,&sr1,&sr2);
    if(feof(pFile)) break;
    i_b.instr_bundle[i].pc = strtoul(pc_str,0,16);
    i_b.instr_bundle[i].opcode = opcode;
    i_b.instr_bundle[i].dr = dr;
    i_b.instr_bundle[i].sr1 = sr1;
    i_b.instr_bundle[i].sr2 = sr2;
    i_b.instr_bundle[i].age = fetch_age_cycle;
    fetch_age_cycle++;
    cout<<" instruction "<<i<<" "<<hex<<i_b.instr_bundle[i].pc<<dec<<" "<<opcode<<" "<<dr<<" "<<sr1<<" "<<sr2<<endl;
    DE = &i_b;
   }
    
  }
}//end of fetch
 
void Decode()//maybe send bundle pointer to decode from fetch
{
  if(DE != NULL)
  {
    if(!RN_empty) // 0 means empty
    {
      RN = DE;
      DE = NULL;
    }
  }


}

void Rename()
{
  if(RN != NULL)
  {
    if((!RR_empty)&&(is_ROB_free()))
    {
      for(int i=0;i<width;i++)
      {
        rob.rob_entry[tail].pc = RN->instr_bundle[i].pc;
        rob.rob_entry[tail].dst = RN->instr_bundle[i].dr;
        rob.rob_entry[tail].rdy = 0;
        rob.rob_entry[tail].exc = 0;
        rob.rob_entry[tail].mis = 0;

        
        //rename source tags.
        if(rmt[RN->instr_bundle[i].sr1].valid == 1) //ROB entry exits
        {
          RN->instr_bundle[i].sr1 = rmt[ RN->instr_bundle[i].sr1 ].tag;
          RN->instr_bundle[i].sr1_rob_or_arf = 1;
        }

        if(rmt[RN->instr_bundle[i].sr2].valid == 1) //ROB entry exits
        {
          RN->instr_bundle[i].sr2 = rmt[ RN->instr_bundle[i].sr2 ].tag;
          RN->instr_bundle[i].sr2_rob_or_arf = 1;
        }

        // Enter the rob value in RMT for destination
        rmt[ RN->instr_bundle[i].dr ].tag = tail;
        rmt[ RN->instr_bundle[i].dr ].valid = 1;
        rob.incr_tail();
      }

    }
  }

}

void RegRead()
{


}

void Dispatch()
{
  if(DI != NULL)
  {
    if(IQ.is_IQ_free()) // 1 if there is place
    {
      for(int i=0;i<width;i++)
      {
        int free_entry //TODO write a funcion to return an empty place in issue queue
        IQ.IQ_entry[free_entry].valid = 1;
        //IQ.IQ_entry[free_entry].dst_tag = DI.instr_bundle[i].dr;
        if(DI->instr_bundle[i].sr1_rob_or_arf == 0) IQ.IQ_entry[free_entry].rdy_rs1 = 1; //ARF
        if(DI->instr_bundle[i].sr2_rob_or_arf == 0) IQ.IQ_entry[free_entry].rdy_rs2 = 1; //ARF
        IQ.IQ_entry.instr = &DI.instr_bundle[i];
      }
      DI = NULL;
    }

  }
}

void Issue()
{
  //find 4 oldest instructions from issue queue

  //remove instructon from IQ
  exc_lst[].instr =  IQ.IQ_entry[].instr;
  if(IQ.IQ_entry[].instr->opcode == 0) exc_lst[].cycle_to_complete = 1;
  if(IQ.IQ_entry[].instr->opcode == 1) exc_lst[].cycle_to_complete = 2;
  if(IQ.IQ_entry[].instr->opcode == 2) exc_lst[].cycle_to_complete = 5;

}

void Execute()
{
  for(int i=0;i<5*width;i++)
  {
    if((exc_lst[i].valid == 1)&&(exc_lst[i].cycle_to_complete == 1)) //instructions that are finishing this cycle
    {
      int dr = exc_lst[i].instr->dr; //this is the destination that will complete this cycle.
      for(int i=0; i<IQ.size; i++)
      {
        if((IQ.IQ_entry[i].instr->sr1 == dr)&&(dr != -1))
        {
          IQ.IQ_entry[i].rdy_sr1 = 1;
        }
        if((IQ.IQ_entry[i].instr->sr2 == dr)&&(dr != -1))
        {
          IQ.IQ_entry[i].rdy_sr2 = 1;
        }
      }
      //wake up instructions in DI and RR also
      

    }
  }
  for(int i = 0;i<5*width;i++)
  {
    if (exc_lst[i].valid == 1)
    {
      exc_lst[i].cycle_to_complete--;
    }
  }

}

void Writeback()
{

}

void Retire()
{

}

    
