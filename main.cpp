#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <sstream>
#include <fstream>

#include <math.h>


using namespace std;

int fetch_age_cycle=0;
int cycle_count = 1;

int go_exec = 0; //set by issue, indicates exc can go ahead
int go_WB = 0; //set by execute
int go_RT = 1;
bool go_issue = 0;

unsigned int ROB_size;
unsigned int IQ_size;
unsigned int width;
FILE *pFile;

ofstream myfile;


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

ISSUE_queue *IQ;

bool DE_empty=0, RN_empty=0,RR_empty=0, DI_empty=0;


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


int main(int argc, char *argv[])
{
  string trace_file;
  char pc_str[10];
  unsigned long pc;
  int opcode;
  int dr;  //destination register
  int sr1, sr2; //source register 1 and 2. using int because value can be -1 i.e no reg

  int total_reads = 0;
  //bool Advance_cycle();
  //void Fetch();
  //void Decode();
  //void Rename();
  //void RegRead();
  //void Dispatch();
  //void Issue();
  //void Execute();
  //void Writeback();
  //void Retire();

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
 
  IQ = new ISSUE_queue(IQ_size); 
 // IQ->ISSUE_queue_c(IQ_size);

  exc_lst = new execution_list[5*width];
  WB = new Writeback_list[5*width];

  myfile.open("scope_check.txt");

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
 // for(int i=0;i<IQ_size;i++) cout<<"IQ["<<i<<"] = "<<IQ->IQ_entry[i].valid<<" "<<hex<<IQ->IQ_entry[i].instr->pc<<" "<<"age "<<IQ->IQ_entry[i].instr->age<<endl;
  cout<<"total reads: "<<dec<<total_reads<<endl;
  myfile.close();
}//end of main

bool Advance_cycle()
{
  if(feof(pFile)) //& pipeline_empty
      return false;
  cycle_count++;
  cout<<"CYCLE COUNT: "<<cycle_count<<endl;
  return true;
 }

void print_instr(instruction *instr)
{
  if(instr != NULL)
  {
  cout<<" PC: "<<hex<<instr->pc<<dec<<" opcode :"<<instr->opcode<<" dr: "<<instr->dr_org<<" dr rn: "<<instr->dr<<" sr1: "<<instr->sr1_org
    <<" sr1 rn: "<<instr->sr1<<" sr2: "<<instr->sr2_org<<" sr2 rn: "<<instr->sr2<<" rdy sr1: "<<instr->rdy_rs1
    <<" rdy sr2: "<<instr->rdy_rs2<<" rob_or arf sr1: "<<instr->sr1_rob_or_arf<<" rob_or arf sr2: "<<instr->sr2_rob_or_arf<<" rob tag "<<instr->rob_tag<<" age: "<<instr->age<<endl;
cout<<" fetch entry "<<instr->fetch_entry<<" decode entry "<<instr->decode_entry<<" rename entry "<<instr->rename_entry<<" regread entry "<<instr->regread_entry<<" dispatch entry "<<instr->dispatch_entry<<" issue entry "<<instr->issue_entry<< " execute entry "<<instr->execute_entry<<" writeback entry "<<instr->writeback_entry<<" retire entry "<<instr->retire_entry<<endl;
}
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
    i_b[i].dr_org = dr;
    i_b[i].sr1 = sr1;
    i_b[i].sr2 = sr2;
    i_b[i].sr1_org = sr1;
    i_b[i].sr2_org = sr2;
    i_b[i].age = fetch_age_cycle;
    i_b[i].rdy_rs1 = 0;
    i_b[i].rdy_rs2 = 0;
    fetch_age_cycle++;
    //cout<<" instruction "<<i<<" "<<hex<<i_b[i].pc<<dec<<" "<<i_b[i].opcode<<" "<<i_b[i].dr<<" "<<i_b[i].sr1_org<<" "<<i_b[i].sr2_org<<" age "<<i_b[i].age<<endl;
    DE = i_b;
    i_b[i].fetch_entry = cycle_count-1;
    i_b[i].decode_entry = cycle_count;
    //cout<<"instruction: "<<i_b[i].age<<" fetch entry: "<<i_b[i].fetch_entry<<endl;
   // cout<<DE<<endl;
   }
    
  }
//        cout<<"fetch abc "<<&IQ->IQ_entry[0]<<endl;
}//end of fetch
 
void Decode()//maybe send bundle pointer to decode from fetch
{
  if(DE != NULL)
  {
      for(int i =0;i<width;i++) DE[i].rename_entry = cycle_count;
    cout<<"Decode check 1"<<endl;
    if(RN_empty != 1) // 0 means empty
    {
      for(int i =0;i<width;i++) DE[i].rename_entry = cycle_count;
      DE_empty = 0;
      cout<<"Decode check 2"<<endl;
      RN = DE;
      DE = NULL;
    }
    else DE_empty = 1;
  }
 //       cout<<"decode abc "<<&IQ->IQ_entry[0]<<endl;


}

void Rename()
{
  cout<<"Rename check 1"<<endl;
  if(RN != NULL)
  {
  //      cout<<"rename abc 1 "<<&IQ->IQ_entry[0]<<endl;
    cout<<"Rename check 2"<<endl;
    //if((!RR_empty)&&(rob.is_ROB_free()))
    if((rob.is_ROB_free() >= width)&&(RR_empty != 1))
    {
      RN_empty = 0;
      cout<<"Rename check 3"<<endl;
      for(int i=0;i<width;i++)
      {
   //     cout<<"rename abc 2 "<<&IQ->IQ_entry[0]<<endl;
   //     cout<<"rob tail"<<rob.tail<<endl;
       // rob.rob_entry[rob.tail].pc = RN[i].pc;
  //      cout<<"rename abc 3 "<<&IQ->IQ_entry[0]<<endl;
       // rob.rob_entry[rob.tail].dst = RN[i].dr;
        rob.rob_entry[rob.tail].rdy = 0;
       // cout<<" rename rob set"<<endl;
       // print_instr(&RN[i]);
       // rob.rob_entry[rob.tail].exc = 0;
       // rob.rob_entry[rob.tail].mis = 0;
   //     cout<<"rename abc 4 "<<&IQ->IQ_entry[0]<<endl;
        rob.rob_entry[rob.tail].instr = &RN[i];
        // newly addded
       // rob.rob_entry[rob.tail].valid = 1;

        
        //rename source tags.
        if(RN[i].sr1 == -1) RN[i].sr1_rob_or_arf = 0;
        if(RN[i].sr2 == -1) RN[i].sr2_rob_or_arf = 0;

        if((RN[i].sr1 != -1)&&(rmt[RN[i].sr1].valid == 1)) //register is valid and ROB entry exits
        {
          //cout<<"sr 1 renamed"<<endl;
          RN[i].sr1 = rmt[ RN[i].sr1 ].tag;
          RN[i].sr1_rob_or_arf = 1;
        }

        if((RN[i].sr2 != -1)&&(rmt[RN[i].sr2].valid == 1)) //register is valid and ROB entry exits
        {
          //cout<<"sr 2 renamed"<<endl;
          RN[i].sr2 = rmt[ RN[i].sr2 ].tag;
          RN[i].sr2_rob_or_arf = 1;
        }

        // Enter the rob value in RMT for destination
         // rob.rob_entry[rob.tail].instr = &RN[i];
         // rob.rob_entry[rob.tail].rdy = 0;
         if(RN[i].dr != -1)
         {
          rmt[ RN[i].dr ].tag = rob.tail; //no need for -1
         // RN[i].rob_tag = rob.tail; 
          rmt[ RN[i].dr ].valid = 1; //no need for -1
         }
          RN[i].rob_tag = rob.tail; 
          RN[i].dr = rob.tail;  //do it always
          rob.incr_tail();     //do it always
         // print_instr(&RN[i]);
         // RN[i].regread_entry = cycle_count;
       // }
       // print_instr(&RN[i]);
        //cout<<" rename "<<i<<" "<<hex<<i_b[i].pc<<dec<<" "<<RN[i].opcode<<" "<<RN[i].dr<<" "<<RN[i].sr1<<" "<<RN[i].sr2<<" age "<<RN[i].age<<endl;
      
       RN[i].regread_entry = cycle_count;
      }

    RR = RN;
    RN = NULL;
    }
    else RN_empty = 1;
  }
    //    cout<<"rename abc end "<<&IQ->IQ_entry[0]<<endl;
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
        RR[i].dispatch_entry = cycle_count;
        if(RR[i].sr1_rob_or_arf == 0) { RR[i].rdy_rs1 = 1;} //cout<<"sr1 set to ready"<<endl; }
        if(RR[i].sr2_rob_or_arf == 0) { RR[i].rdy_rs2 = 1; }//cout<<"sr2 set to ready"<<endl; }
        //print_instr(&RR[i]);
      // RR[i].dispatch_entry = cycle_count;
        if(RR[i].sr1_rob_or_arf == 1)
        {
          cout<<"regread rob "<<RR[i].sr1<<" "<<rob.rob_entry[RR[i].sr1].rdy<<endl;
          if(rob.rob_entry[RR[i].sr1].rdy == 1) RR[i].rdy_rs1 = 1;
        }
        if(RR[i].sr2_rob_or_arf == 1)
        {
          if(rob.rob_entry[RR[i].sr2].rdy == 1) RR[i].rdy_rs2 = 1;
        }
        cout<<"readread stage"<<endl;
        print_instr(&RR[i]);

      }
      DI = RR;
      RR = NULL;

    }
    else RR_empty = 1;
    }
     //   cout<<"regread abc "<<&IQ->IQ_entry[0]<<endl;
}

void Dispatch()
{
  if(DI != NULL)
  {
    if(IQ->is_IQ_empty() >= width) // 1 if there is place
    {
      cout<<"Dispatch check"<<endl;
      DI_empty = 0;
      for(int i=0;i<width;i++)
      {
        int free_entry = IQ->free_entry(); //TODO write a funcion to return an empty place in issue queue
        cout<<"free entry "<<free_entry<<endl;
        cout<<IQ<<endl;
        if(free_entry != (-1)){
        cout<<"DI check"<<endl;
        IQ->IQ_entry[free_entry].valid = 1;
        IQ->IQ_entry[free_entry].v = 1;
        //IQ->IQ_entry[free_entry].dst_tag = DI.instr_bundle[i].dr;
      //  if(DI[i].sr1_rob_or_arf == 0) IQ->IQ_entry[free_entry].rdy_rs1 = 1; //ARF
      //  if(DI->instr_bundle[i].sr2_rob_or_arf == 0) IQ->IQ_entry[free_entry].rdy_rs2 = 1; //ARF
        IQ->IQ_entry[free_entry].instr = &DI[i];
        IQ->IQ_entry[free_entry].valid = 1;
        IQ->IQ_V[free_entry] = 1;
        //IQ->incr_tail();
        }
      // RR[i].dispatch_entry = cycle_count;
       DI[i].issue_entry = cycle_count;
      }
      DI = NULL;
      go_issue = 1;
    }
    else 
    {
      DI_empty = 1;
      cout<<"DI empty set"<<endl;
    }
  }
    //else DI_empty = 1;
      //  cout<<"dispatch abc "<<&IQ->IQ_entry[0]<<endl;
}

void Issue()
{
  //find 4 oldest instructions from issue queue
  //if(IQ->is_IQ_empty())
  if(go_issue)
  {
    int min = 11000;
    int min_last = -1;
    int index = -1;
    int prev_index = -1;
   // index = new int[width];
   // for(int z=0;z<width;z++) index[z] = (-1);
    int count = 0;
    for(int i=0;i<width;i++) //since we need 4 instructions
    {
      int min = 11000;
      for(int j=0;j<IQ_size;j++)
      {
       // if(IQ->IQ_entry[j].valid == 1)
        if(IQ->IQ_V[j] == 1)
        {
          if(IQ->IQ_entry[j].instr != NULL)
          {
            if((IQ->IQ_entry[j].instr->rdy_rs1 == 1)&&(IQ->IQ_entry[j].instr->rdy_rs2==1))
            {
              if((IQ->IQ_entry[j].instr->age <min)) //&&(IQ->IQ_entry[j].instr->age > min_last))
              { 
                min = IQ->IQ_entry[j].instr->age; 
                index = j;
                cout<<"in min "<<min<<" "<<index<<" prev "<<prev_index<<endl;
               }
             }
          }
        }
      }
        //cout<<"min last:"<<min_last<<endl;
               // count++;
               // cout<<"count: "<<count<<endl;
        if((index > -1)&&(index != prev_index)) 
        {
            IQ->IQ_entry[index].valid = 0;
            IQ->IQ_entry[index].v = 0;
            IQ->IQ_V[index] = 0;
            IQ->IQ_entry[index].instr->execute_entry = cycle_count;
            //cout<<"Issue min "<<min<<" "<<index<<endl;
            ///to get a free netry in execute list
             int free_entry;
             for(int k=0; k< 5*width; k++)
             {
               if(exc_lst[k].valid == 0) { free_entry = k; break; }
             }
             exc_lst[free_entry].instr =  IQ->IQ_entry[index].instr;
            // if(IQ->IQ_entry[0].instr->pc == 0x2b663c) cout<<"ALERT"<<" index "<<index[i]<<endl;
             cout<<hex<<exc_lst[free_entry].instr->pc<<dec<<endl;
             exc_lst[free_entry].valid = 1;
             if(IQ->IQ_entry[index].instr->opcode == 0) exc_lst[free_entry].cycle_to_complete = 1;
             if(IQ->IQ_entry[index].instr->opcode == 1) exc_lst[free_entry].cycle_to_complete = 2;
             if(IQ->IQ_entry[index].instr->opcode == 2) exc_lst[free_entry].cycle_to_complete = 5;
             prev_index = index;
        }
         go_exec = 1;
       }

  }
      for(int a=0;a<IQ_size;a++) 
          {
             if(IQ->IQ_entry[a].instr != NULL) 
              cout<<"issue queue "<<a<<" "<<hex<<IQ->IQ_entry[a].instr->pc<<dec 
              <<" "<<IQ->IQ_entry[a].instr->rdy_rs1<<" "<<IQ->IQ_entry[a].instr->rdy_rs2<<" age: "<<IQ->IQ_entry[a].instr->age <<" valid "<<IQ->IQ_entry[a].valid<<" "<<IQ->IQ_entry[a].v<<" test "<<IQ->IQ_V[a]<<endl;
        //  print_instr(IQ->IQ_entry[a].instr);
          }
       // cout<<"issue abc "<<&IQ->IQ_entry[0]<<endl;
}

void Execute()
{
 if(go_exec == 1)
 {
  for(int i=0;i<5*width;i++)
  {
    //cout<<" EXECUTE check"<<endl;
    if((exc_lst[i].valid == 1)&&(exc_lst[i].cycle_to_complete == 1)) //instructions that are finishing this cycle
    {
      cout<<" EXECUTE check 1"<<endl;
      int dr = exc_lst[i].instr->dr; //this is the destination that will complete this cycle.
      int dr_org = exc_lst[i].instr->dr_org; //this is the original  destination that will complete this cycle.
      cout<<"Exc "<<hex<<exc_lst[i].instr->pc<<dec<<endl;
      print_instr(exc_lst[i].instr);
      for(int k=0; k<IQ_size; k++)
      {
      //  cout<<"check 1"<<endl;
       // cout<<IQ<<" "<<k<<endl;
       // cout<<"abc "<<&IQ->IQ_entry[0]<<endl;
       // cout<<"check 2"<<endl;
        if(IQ->IQ_entry[k].instr != NULL)
        {
       // cout<<"check 2"<<endl;
          if((IQ->IQ_entry[k].instr->sr1 == dr))//&&(dr != -1))
          {
       // cout<<"check 3"<<endl;
            IQ->IQ_entry[k].instr->rdy_rs1 = 1;
            cout<<"execute sr1 wakeup"<<endl;
            //print_instr(IQ->IQ_entry[k].instr);
          }
          if((IQ->IQ_entry[k].instr->sr2 == dr))//&&(dr != -1))
          {
       // cout<<"check 4"<<endl;
            IQ->IQ_entry[k].instr->rdy_rs2 = 1;
          }
        }
        else cout<<"IQ instr null"<<endl;
       // else cout<<"instr is null"<<endl;
       // cout<<"check 5"<<endl;
      }
      //wake up instructions in DI and RR also
      if(DI != NULL)
      {
        for(int k=0;k<width;k++)
        {
          if((DI[k].sr1 == dr)) /*&&(dr != -1))*/ { DI[k].rdy_rs1 = 1; cout<<"ready set in decode bundle sr1 "<<dr<<endl;}
          if((DI[k].sr2 == dr)) /*&&(dr != -1))*/ { DI[k].rdy_rs2 = 1; cout<<"ready set in decode bundle sr2 "<<dr<<endl;}
          cout<<"decode constents in execute stage"<<endl;
          print_instr(&DI[k]);
        }
      }
      if(RR != NULL)
      {
        for(int k=0;k<width;k++)
        {
          if((RR[k].sr1 == dr))/*&&(dr != -1))*/ { RR[k].rdy_rs1 = 1; cout<<"ready set in regread bundle sr1"<<endl; }
          if((RR[k].sr2 == dr))/*&&(dr != -1))*/ { RR[k].rdy_rs2 = 1; cout<<"ready set in regread bundle sr2"<<endl; }
          cout<<"RR  constents in execute stage"<<endl;
          print_instr(&RR[k]);
        }
      }
     // if(RN != NULL)
     // {
     //   for(int k=0;k<width;k++)
     //   {
     //     cout<<"RN contents during execute"<<endl;
     //     print_instr(&RN[k]);
     //   }
     // }


    

      //Put the instruction in WB
      int free_entry;
      for(int k=0;k< 5*width;k++)
      {
        if(WB[k].valid == 0) { free_entry = k; break; }
      }
     // cout<<"WB free entry" <<free_entry<<endl;
      WB[free_entry].valid = 1;
      WB[free_entry].instr = exc_lst[i].instr;
      exc_lst[i].valid = 0;
      exc_lst[i].instr->writeback_entry = cycle_count;


    }
  }
  //go_exec = 0;
  go_WB = 1;
 }
  cout<<" EXECUTE check 2"<<endl;
  for(int i = 0;i<5*width;i++)
  {
    if (exc_lst[i].valid == 1)
    {
      exc_lst[i].cycle_to_complete--;
     cout << "c2c: " << exc_lst[i].cycle_to_complete<<" "<<hex<<exc_lst[i].instr->pc <<dec<< " " << endl;
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
        cout<<"rob tag "<<WB[i].instr->rob_tag<<" rob rdy "<<rob.rob_entry[WB[i].instr->rob_tag].rdy<<endl;
        cout<<"writeback instruction"<<endl;
        print_instr(WB[i].instr);
        cout<<" ready rob tag "<<WB[i].instr->rob_tag<<" "<<rob.rob_entry[WB[i].instr->rob_tag].rdy<<endl;
        WB[i].valid = 0;
        WB[i].instr->retire_entry = cycle_count;
      if(RR != NULL)
      {
        for(int k=0;k<width;k++)
        {
           if((RR[k].sr1 == WB[i].instr->dr))/*&&(dr != -1))*/ { RR[k].rdy_rs1 = 1; cout<<"ready set in rename bundle sr1"<<endl; }
            if((RR[k].sr2 == WB[i].instr->dr))/*&&(dr != -1))*/ { RR[k].rdy_rs2 = 1; cout<<"ready set in rename bundle sr2"<<endl; }
         // cout<<"wb instruction"<<endl;
         // print_instr(WB[i].instr);
          cout<<"RN  constents in execute stage"<<endl;
          print_instr(&RR[k]);
        }
      }
      }
      
    }
    go_WB = 0;
    go_RT = 1;
  }
      //  cout<<"writeback abc "<<&IQ->IQ_entry[0]<<endl;

}

void Retire()
{
  cout<<"rob at retire: head "<<rob.head<<endl;
 // if((rob.head == 0)&&(rob.tail == 1)) rob.head = 1; //to handle initial condition
  for(int i = 0;i<ROB_size;i++)
  {
  // if(rob.rob_entry[i].instr != NULL)cout<<" PC "<<hex<<rob.rob_entry[i].instr->pc<<dec<<" age "<<rob.rob_entry[i].instr->age<<" ready "<<rob.rob_entry[i].rdy<<endl;
  }
 // for(int i=0;i<width;i++)
 // {
    int head = rob.head;
    for(int k=head;k<(head+width);k++)
    {
      cout<<"retire iter "<<k<<endl;
     if((rob.rob_entry[k].rdy == 0)){ cout<<"breaking bad "<<rob.rob_entry[k].rdy<<endl; print_instr(rob.rob_entry[k].instr); break;} //&&(!(rob.head == 0)&&(rob.tail == 1))) break;
     if(rob.rob_entry[k].rdy == 1)
     {
      // rob.rob_entry[k].instr->retire_entry = cycle_count;
      // rob.rob_entry[k].valid = 0;
       cout<<"retired insruction "<<rob.rob_entry[k].instr->age<<" rob head "<<rob.head<<endl;
       for(int i =0; i<IQ_size; i++)
       {
         if(rmt[i].tag == k) rmt[i].valid = 0;
       }
       rob.incr_head();
       myfile<<rob.rob_entry[k].instr->age<<" "<<"fu{"<<rob.rob_entry[k].instr->opcode<<"} src{"<<rob.rob_entry[k].instr->sr1_org<<","<<rob.rob_entry[k].instr->sr2_org<<"} dst{"
         <<rob.rob_entry[k].instr->dr_org<<"} FE{"<<rob.rob_entry[k].instr->fetch_entry<<","
         <<(rob.rob_entry[k].instr->decode_entry - rob.rob_entry[k].instr->fetch_entry)
         <<"} DE{"<<rob.rob_entry[k].instr->decode_entry<<","<<(rob.rob_entry[k].instr->rename_entry - rob.rob_entry[k].instr->decode_entry)<<"} RN{"
         <<rob.rob_entry[k].instr->rename_entry<<","<<(rob.rob_entry[k].instr->regread_entry - rob.rob_entry[k].instr->rename_entry)<<"} RR{"
         <<rob.rob_entry[k].instr->regread_entry<<","<<(rob.rob_entry[k].instr->dispatch_entry - rob.rob_entry[k].instr->regread_entry)<<"} DI{"
         <<rob.rob_entry[k].instr->dispatch_entry<<","<<(rob.rob_entry[k].instr->issue_entry - rob.rob_entry[k].instr->dispatch_entry)<<"} IS{"
         <<rob.rob_entry[k].instr->issue_entry<<","<<(rob.rob_entry[k].instr->execute_entry - rob.rob_entry[k].instr->issue_entry)<<"} EX{"
         <<rob.rob_entry[k].instr->execute_entry<<","<<(rob.rob_entry[k].instr->writeback_entry - rob.rob_entry[k].instr->execute_entry)<<"} WB{"
         <<rob.rob_entry[k].instr->writeback_entry<<","<<(rob.rob_entry[k].instr->retire_entry - rob.rob_entry[k].instr->writeback_entry)<<"} RT{"
         <<rob.rob_entry[k].instr->retire_entry<<","<<(cycle_count - rob.rob_entry[k].instr->retire_entry)<<"}"<<endl;
  
     }
    }
 // }


       // cout<<"retire abc "<<&IQ->IQ_entry[0]<<endl;
}

  
