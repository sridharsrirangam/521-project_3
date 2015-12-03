#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <sstream>
#include <fstream>

#include <math.h>


using namespace std;

int fetch_age_cycle=0;
int cycle_count = 0;

int go_exec = 0; //set by issue, indicates exc can go ahead
int go_WB = 0; //set by execute
int go_RT = 1;

unsigned int ROB_size;
unsigned int IQ_size;
unsigned int width;
FILE *pFile;

#include "class_definitions.h"
  
instruction *i_b;

instruction *DE;
instruction *RN;
instruction *RR;
instruction *DI;

execution_list *exc_lst;
Writeback_list *WB;
ROB_table rob;
RMT_block rmt[67];
ISSUE_queue IQ;

bool DE_empty=0, RN_empty=0,RR_empty=0, DI_empty=0;


int main(int argc, char *argv[])
{
  string trace_file;
  char pc_str[10];
  unsigned long pc;
  int opcode;
  int dr;  //destination register
  int sr1, sr2; //source register 1 and 2. using int because value can be -1 i.e no reg

  int total_reads = 0;
  bool Advance_cycle();
  void Fetch();
  void Decode();
  void Rename();
  void RegRead();
  void Dispatch();
  void Issue();
  void Execute();
  void Writeback();
  void Retire();

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
  
  //i_b.instruction_bundle_c(width);

  rob.ROB_table_c(ROB_size);
 
  
  IQ.ISSUE_queue_c(IQ_size);

  exc_lst = new execution_list[5*width];
  WB = new Writeback_list[5*width];
 //while(fscanf(pFile,"%s %d %d %d %d",&pc_str,&opcode,&dr,&sr1,&sr2))
  do
  {
    cout<<"ready states "<<DE_empty<<" "<<RN_empty<<" "<<RR_empty<<" "<<DI_empty<<endl;
    if(feof(pFile))break;
    total_reads++;
    pc = strtoul(pc_str,0,16);
    // cout<<hex<<pc<<" "<<dec<<opcode<<" "<<dr<<" "<<sr1<<" "<<sr2<<endl;
    Retire();
    Writeback();
    Execute();
    Issue();
    Dispatch();
    RegRead();
    Rename();
    Decode();
    Fetch();

  }while(Advance_cycle());
  for(int i=0;i<67;i++) cout<<"rmt["<<i<<"] = "<<rmt[i].valid<<" "<<rmt[i].tag<<endl;
 // for(int i=0;i<IQ_size;i++) cout<<"IQ["<<i<<"] = "<<IQ.IQ_entry[i].valid<<" "<<hex<<IQ.IQ_entry[i].instr->pc<<" "<<"age "<<IQ.IQ_entry[i].instr->age<<endl;
  cout<<"total reads: "<<dec<<total_reads<<endl;

}//end of main

bool Advance_cycle()
{
  cout<<"CYCLE COUNT: "<<cycle_count<<endl;
  if(feof(pFile)) //& pipeline_empty
      return false;
  cycle_count++;
  return true;
 }



void Fetch()
{
  unsigned long pc;
  int opcode;
  int dr;
  int sr1,sr2;
  char pc_str[10];

  //if((DE_empty != 1)&&(! feof(pFile))) //set to 1 if DE is not empty
  if(! feof(pFile)) //set to 1 if DE is not empty
  {
    i_b = new instruction[width];
    //i_b.instruction_bundle_c(width);
   for(int i=0;i<width;i++)
   {
    fscanf(pFile,"%s %d %d %d %d",&pc_str,&opcode,&dr,&sr1,&sr2);
    if(feof(pFile)) break;
    i_b[i].pc = strtoul(pc_str,0,16);
    i_b[i].opcode = opcode;
    i_b[i].dr = dr;
    i_b[i].sr1 = sr1;
    i_b[i].sr2 = sr2;
    i_b[i].sr1_org = sr1;
    i_b[i].sr2_org = sr2;
    i_b[i].age = fetch_age_cycle;
    i_b[i].rdy_rs1 = 0;
    i_b[i].rdy_rs2 = 0;
    fetch_age_cycle++;
    cout<<" instruction "<<i<<" "<<hex<<i_b[i].pc<<dec<<" "<<i_b[i].opcode<<" "<<i_b[i].dr<<" "<<i_b[i].sr1_org<<" "<<i_b[i].sr2_org<<" age "<<i_b[i].age<<endl;
    DE = i_b;
    i_b[i].fetch_entry = cycle_count;
    cout<<"instruction: "<<i_b[i].age<<" fetch entry: "<<i_b[i].fetch_entry<<endl;
   // cout<<DE<<endl;
   }
    
  }
}//end of fetch
 
void Decode()//maybe send bundle pointer to decode from fetch
{
  if(DE != NULL)
  {
    cout<<"Decode check 1"<<endl;
    if(RN_empty != 1) // 0 means empty
    {
      DE_empty = 0;
      cout<<"Decode check 2"<<endl;
      RN = DE;
      DE = NULL;
    }
    else DE_empty = 1;
  }


}

void Rename()
{
  cout<<"Rename check 1"<<endl;
  if(RN != NULL)
  {
    cout<<"Rename check 2"<<endl;
    //if((!RR_empty)&&(rob.is_ROB_free()))
    if((rob.is_ROB_free())&&(RR_empty != 1))
    {
      RN_empty = 0;
      cout<<"Rename check 3"<<endl;
      for(int i=0;i<width;i++)
      {
        rob.rob_entry[rob.tail].pc = RN[i].pc;
        rob.rob_entry[rob.tail].dst = RN[i].dr;
        rob.rob_entry[rob.tail].rdy = 0;
        rob.rob_entry[rob.tail].exc = 0;
        rob.rob_entry[rob.tail].mis = 0;
        rob.rob_entry[rob.tail].instr = &RN[i];

        
        //rename source tags.
        if((RN[i].sr1 != -1)&&(rmt[RN[i].sr1].valid == 1)) //register is valid and ROB entry exits
        {
          cout<<"sr 1 renamed"<<endl;
          RN[i].sr1 = rmt[ RN[i].sr1 ].tag;
          RN[i].sr1_rob_or_arf = 1;
        }

        if((RN[i].sr2 != -1)&&(rmt[RN[i].sr2].valid == 1)) //register is valid and ROB entry exits
        {
          cout<<"sr 2 renamed"<<endl;
          RN[i].sr2 = rmt[ RN[i].sr2 ].tag;
          RN[i].sr2_rob_or_arf = 1;
        }

        // Enter the rob value in RMT for destination
        if(RN[i].dr != -1)
        {
          rmt[ RN[i].dr ].tag = rob.tail;
          RN[i].rob_tag = rob.tail;
          rmt[ RN[i].dr ].valid = 1;
          rob.incr_tail();
        }
        cout<<" rename "<<i<<" "<<hex<<i_b[i].pc<<dec<<" "<<RN[i].opcode<<" "<<RN[i].dr<<" "<<RN[i].sr1<<" "<<RN[i].sr2<<" age "<<RN[i].age<<endl;
      }

    RR = RN;
    RN = NULL;
    }
    else RN_empty = 1;
  }
  }



void RegRead()
{ 
    if(RR != NULL)
    {
      if(DI_empty != 1)
      {
      RR_empty = 0;
      cout<<"RR check"<<endl;
      for(int i=0; i<width; i++)
      {
        if(RR[i].sr1_rob_or_arf == 0) { RR[i].rdy_rs1 = 1; cout<<"sr1 set to ready"<<endl; }
        if(RR[i].sr2_rob_or_arf == 0) { RR[i].rdy_rs2 = 1; cout<<"sr2 set to ready"<<endl; }
      }
      DI = RR;
      RR = NULL;

    }
    else RR_empty = 1;
    }
}

void Dispatch()
{
  if(DI != NULL)
  {
    if(IQ.is_IQ_free()) // 1 if there is place
    {
      DI_empty = 0;
      for(int i=0;i<width;i++)
      {
        int free_entry = IQ.free_entry(); //TODO write a funcion to return an empty place in issue queue
        cout<<"free entry "<<free_entry<<endl;
        if(free_entry != -1){
        cout<<"DI check"<<endl;
        IQ.IQ_entry[free_entry].valid = 1;
        //IQ.IQ_entry[free_entry].dst_tag = DI.instr_bundle[i].dr;
      //  if(DI[i].sr1_rob_or_arf == 0) IQ.IQ_entry[free_entry].rdy_rs1 = 1; //ARF
      //  if(DI->instr_bundle[i].sr2_rob_or_arf == 0) IQ.IQ_entry[free_entry].rdy_rs2 = 1; //ARF
        IQ.IQ_entry[free_entry].instr = &DI[i];
        IQ.incr_tail();
        }
      }
      DI = NULL;
    }

    else DI_empty = 1;
  }
    //else DI_empty = 1;
}

void Issue()
{
  //find 4 oldest instructions from issue queue
  if(IQ.is_IQ_empty())
  {
    int min = 11000;
    int min_last = -1;
    int *index;
    index = new int[width];
    for(int z=0;z<width;z++) index[z] = (-1);
    int count = 0;
    for(int i=0;i<width;i++) //since we need 4 instructions
    {
      int min = 11000;
      for(int j=0;j<IQ_size;j++)
      {
        if(IQ.IQ_entry[j].valid == 1)
        {
          if(IQ.IQ_entry[j].instr != NULL)
          {
            if((IQ.IQ_entry[j].instr->rdy_rs1 == 1)&&(IQ.IQ_entry[j].instr->rdy_rs2==1))
            {
              if((IQ.IQ_entry[j].instr->age <min)&&(IQ.IQ_entry[j].instr->age > min_last))
              { 
                min = IQ.IQ_entry[j].instr->age; 
                index[i] = j;
                cout<<"in min"<<min<<" "<<index[i]<<endl;
               }
             }
          }
        }
      }
        min_last = min;
        cout<<"min last:"<<min_last<<endl;
               // count++;
               // cout<<"count: "<<count<<endl;
    }
      for(int z=0;z<width;z++) 
      { cout<<"index:"<<index[z]<<endl;
        if(index[z] >=0) { count++; cout<<"count:"<<count<<endl;}
      }
      //unless 4 instructions are ready, we dont send them to execute.verify this in forums and implement
       if(count == width)
       {
         for(int i=0;i<width;i++)
         {
            IQ.IQ_entry[index[i]].valid = 0;
            cout<<"Issue min "<<min<<" "<<index<<endl;
            ///to get a free netry in execute list
             int free_entry;
             for(int k=0; k< 5*width; k++)
             {
               if(exc_lst[k].valid == 0) { free_entry = k; break; }
             }
             exc_lst[free_entry].instr =  IQ.IQ_entry[i].instr;
             exc_lst[free_entry].valid = 1;
             if(IQ.IQ_entry[i].instr->opcode == 0) exc_lst[free_entry].cycle_to_complete = 1;
             if(IQ.IQ_entry[i].instr->opcode == 1) exc_lst[free_entry].cycle_to_complete = 2;
             if(IQ.IQ_entry[i].instr->opcode == 2) exc_lst[free_entry].cycle_to_complete = 5;
        }
         go_exec = 1;
       }

  }
}

void Execute()
{
 if(go_exec == 1)
 {
  for(int i=0;i<5*width;i++)
  {
    cout<<" EXECUTE check"<<endl;
    if((exc_lst[i].valid == 1)&&(exc_lst[i].cycle_to_complete == 1)) //instructions that are finishing this cycle
    {
      cout<<" EXECUTE check 1"<<endl;
      int dr = exc_lst[i].instr->dr; //this is the destination that will complete this cycle.
      for(int k=0; k<IQ.size; k++)
      {
          if(IQ.IQ_entry[k].instr != NULL)
          {
          if((IQ.IQ_entry[k].instr->sr1 == dr)&&(dr != -1))
          {
            IQ.IQ_entry[k].instr->rdy_rs1 = 1;
          }
          if((IQ.IQ_entry[k].instr->sr2 == dr)&&(dr != -1))
          {
            IQ.IQ_entry[k].instr->rdy_rs2 = 1;
          }
        }
      }
      //wake up instructions in DI and RR also
     

      //Put the instruction in WB
      cout<<" EXECUTE check 2"<<endl;
      int free_entry;
      for(int k=0;k< 5*width;k++)
      {
        if(WB[k].valid == 0) { free_entry = k; break; }
      }
      WB[free_entry].valid = 1;
      WB[free_entry].instr = exc_lst[i].instr;
      exc_lst[i].valid = 0;


    }
  }
  go_exec = 0;
  go_WB = 1;
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
  if(go_WB == 1)
  {
    cout<<"writeback check"<<endl;
    for(int i=0; i<5*width;i++)
    {
      if(WB[i].valid == 1)
      {
        rob.rob_entry[WB[i].instr->rob_tag].rdy =1;
      }
    }
    go_WB = 0;
    go_RT = 1;
  }

}

void Retire()
{
  for(int i=0;i<width;i++)
  {
    for(int k=rob.head;k<ROB_size;k++)
    {
     if(rob.rob_entry[k].rdy == 1)
     {
       rob.incr_head();
       cout<<"retired insruction "<<rob.rob_entry[k].instr->age<<endl;
     }
    }
  }


}

  
