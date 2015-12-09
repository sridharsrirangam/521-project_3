#ifndef MY_HEADER_CLASSES
#define MY_HEADER_CLASSES

using namespace std;

bool rob_first_entry = 0;

class instruction{
  public:
    unsigned long pc;
    int opcode;
    int dr;
    int sr1,sr2;
    int sr1_org,sr2_org;
    int dr_org;
    int sr1_rob_or_arf, sr2_rob_or_arf;  //if 1, tag is ROb, else ARF
    int age;
    int rdy_rs1, rdy_rs2;
    int rob_tag;
    
    //variables to log entry and exit times
    int fetch_entry;
   // int fetch_exit;
    int decode_entry;
   // int decode_exit;
    int rename_entry;
   // int rename_exit;
    int regread_entry;
   // int regread_exit;
    int dispatch_entry;
   // int dispatch_exit;
    int issue_entry;
   // int issue_exit;
    int execute_entry;
   // int execute_exit;
    int writeback_entry;
   // int writeback_exit;
    int retire_entry;
   // int retire_exit;

    instruction()
    {
      rdy_rs1 = 0;
      rdy_rs2 = 0;
      fetch_entry = 0;
      decode_entry = 0;
      rename_entry = 0;
      regread_entry = 0;
      dispatch_entry = 0;
      issue_entry = 0;
      execute_entry = 0;
      writeback_entry = 0;
      retire_entry = 0;
    }

};

class instruction_bundle{
  public:
    instruction *instr_bundle;
    unsigned int width;

    void instruction_bundle_c(unsigned int w){
      width = w;
      instr_bundle = new instruction[width];
    }
};


class RMT_block{
  public:
    int valid;
    int tag;

    RMT_block()
    {
      valid = 0;
      tag = 0;
    }
};

class issue_queue_entry{
  public:
    int valid;
    int v;
    //int dst_tag;
   // int rdy_rs1;
   // int rdy_rs2;
    //int tag_rs1;
    //int tag_rs2;
    instruction *instr;

    issue_queue_entry(){
     // valid = 0;
     // v =0;
      //dst_tag = 0;
      //rdy_rs1 = 0;
      //rdy_rs2 = 0;
     // tag_rs1 = 0;
     // tag_rs2 = 0;
    }
};

class ISSUE_queue{
  public:
    issue_queue_entry *IQ_entry;  
    int *IQ_V;
    unsigned int size;
    int head, tail;
   // ISSUE_queue()
   // {
   // }
     ISSUE_queue(unsigned int s)
    {
      size = s;
      head = 0;
      tail = 0;
      IQ_entry = new issue_queue_entry[size];
      IQ_V = new int[size];
     // for(int i=0;i<size;i++)
     // {
      //  cout<<" pointer "<<&IQ_entry[i]<<endl;
     // }
     // for(int i =0;i<size;i++) IQ_V[i] = 0;
    }
    //bool is_IQ_free()
    //{
    //  //check if there are four free entries in IQ and respond 1 if free
    //  int space;
    //  //implement full or empty calculations using head nad tail
    //  if(head > tail) space = head - tail - 1;
    //  if(tail > head) space = size - tail + head ;//TODO change in ROB also
    //  cout<<"IQ stats "<<space<<" "<<head<<" "<<tail<<endl; 
    //  if(space>width) return 1;
    //  if((head == 0)&&(tail == 0)) return 1; //maybe not correct
    //  else if(space<width) return 0;
    //}
    void incr_tail()
    {
      if(tail<size) tail++;
      else if (tail == size) tail = 0;
    }
    int free_entry()
    {
     // for(int i =0;i<size;i++) cout<<"IQ_V "<<i<<" "<<IQ_V[i]<<endl;
      for(int i=0; i<size; i++)
      {
       // if(IQ_entry[i].valid == 0) return i;
        if(IQ_V[i] == 0) {
        cout<<"abc "<<i<<endl;
        return i;
        }
      }
      return (-1);

    }
    int is_IQ_empty()
    {
      int count = 0;
      for(int i=0;i<size; i++)
      {
       // if(IQ_entry[i].valid == 0) count++;
        if(IQ_V[i] == 0) count++;
      }
      cout<<"issue count "<<count<<endl;
      return count;
    }
};


class ROB_entry{
  public:
    int value;
  //  int dst;
    int rdy;
 //   int exc;
 //   int mis;
 //   int pc;
  //  int valid;
    instruction *instr;
  //  ROB_entry()
  //  {
  //    valid = 0;
  //  }
};

class ROB_table {
  public:
    ROB_entry *rob_entry;
    int head, tail;
    unsigned int size;
  
    void ROB_table_c(unsigned int s)
    {
      size = s;
      head = 0;
      tail = 0; //was 0
      rob_entry = new ROB_entry[size];
    }

   // bool is_ROB_free()
   // {
   //   int space;
   //   //implement full or empty calculations using head nad tail
   //   for(int i=head;i<head+width;i++)
   //   {
   //     if(rob_entry[i].instr != NULL)
   //       if(rob_entry[i].rdy == 0) return 0;
   //   }
   //   if(head > tail) space = head - tail - 1;
   //   if(tail > head) space = size - tail + head - 1;
   //   cout<<"rob stats "<<space<<" "<<head<<" "<<tail<<endl; 
   //   if(space>width) return 1;
   //   if((head == 0)&&(tail == 0)) return 1; //maybe not correct
   //   else if(space<width) return 0;
   // }
    
    //int is_ROB_free()
    //{
    //  cout<<"rob stats "<<" "<<head<<" "<<tail<<endl; 
    //  int count = 0;
    //  if(tail>=head)
    //  {
    //    for(int i=tail;i<size; i++)
    //    {
    //      if(rob_entry[i].valid == 0) count++;
    //    }
    //    for(int i=0;i<head; i++)
    //    {
    //      if(rob_entry[i].valid == 0) count++;
    //    }
    //  }
    //  if(tail<head)
    //  {
    //    for(int i=tail;i<head;i++)
    //    {
    //      if(rob_entry[i].valid == 0) count++;
    //    }
    //  }
    int is_ROB_free()
    {
      cout<<"rob stats "<<" "<<head<<" "<<tail<<endl; 
      int count = 0;
      if(tail>head)
      {
        for(int i=tail;i<size; i++)
        {
           count++;
        }
        for(int i=0;i<head; i++)
        {
           count++;
        }
      }
      if(tail<head)
      {
        for(int i=tail;i<head;i++)
        {
           count++;
        }
      }
      if((head == 0)&&(tail == 0) &&(rob_first_entry == 0))
      {
        cout<<"Entered first entry"<<endl;
        rob_first_entry = 1;
        count = size;

      }

    
      if(count<4) cout<<"rob full width is "<<width<<endl;
      return count;
    }
    void incr_tail()
    {
      if(tail<size-1) tail++;
      else if (tail == size-1 ) tail = 0;
      if(tail == head) cout<<"rob is full by tail increment"<<endl;
    }
    void incr_head()
    {
      if(head<size-1) head++;
      else if (head == size-1 ) head = 0;
    }

};

class execution_list {
  public:
    int valid;
    int cycle_to_complete;
    instruction *instr;
    execution_list()
    {
      valid = 0;
    }
    bool isValid() { return valid;}
};

class  Writeback_list{
  public:
    int valid;
    instruction *instr;
    Writeback_list()
    {
      valid = 0;
    }
    bool isValid() { return valid;}
};





#endif
