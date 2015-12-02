#ifndef MY_HEADER_CLASSES
#define MY_HEADER_CLASSES

using namespace std;


class instruction{
  public:
    unsigned long pc;
    int opcode;
    int dr;
    int sr1,sr2;
    int sr1_rob_or_arf, sr2_rob_or_arf;  //if 1, tag is ROb, else ARF
    int age;

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
    int dst_tag;
    int rdy_rs1;
    int rdy_rs2;
    int tag_rs1;
    int tag_rs2;

    issue_queue_entry(){
      valid = 0;
      dst_tag = 0;
      rdy_rs1 = 0;
      rdy_rs2 = 0;
      tag_rs1 = 0;
      tag_rs2 = 0;
    }
};

class ISSUE_queue{
  public:
    issue_queue_entry *IQ_entry;  
    unsigned int size;
    void ISSUE_queue_c(unsigned int s)
    {
      size = s;
      IQ_entry = new issue_queue_entry[size];
    }
    void is_IQ_free()
    {
      //check if there are four free entries in IQ and respond 1 if free
    }
};


class ROB_entry{
  public:
    int value;
    int dst;
    int rdy;
    int exc;
    int mis;
    int pc;
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
      tail = 0;
      rob_entry = new ROB_entry[size];
    }

    int is_ROB_free()
    {
      //implement full or empty calculations using head nad tail
    }
    void incr_tail()
    {

    }

};







#endif
