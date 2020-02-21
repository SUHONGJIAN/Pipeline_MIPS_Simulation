#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;   //(这边所有声明一定要加，不然会报错)
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {     //(EX stage)只有read_data1,read_data2,imm,alu_op会用于计算ALUresult，其他全都是其他control signals
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable;
    bool        nop;

    string      INS;
};

struct MEMStruct {    //(MEM stage)只有ALUresult,Store_data,rd_mem,wrt_mem会用于对data memory进行操作，其他全都是其他control signals
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;

    string      INS;
};

struct WBStruct {     //(WB stage)只有wrt_data,wrt_reg_addr,wrt_enable会用于对register进行操作，其他全都是其他control signals
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;

    string      INS;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);     //register[0]应该是常数0(hard-wired)
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)                      //register读操作，返回bitset<32>
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)  //register写操作，无返回值
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult_grading.txt", std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress)             //instruction memory读操作(无写操作)，返回bitset<32>
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);    //采用string的append和"+"是一样的，不过更容易辨认(推荐采用append写法)，然后使用bitset<32> (string)构造法返回所读instruction
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)                //data memory读操作，返回bitset<32>
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//同instruction memory一样写法
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData) //data memory写操作，无返回值     
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult_grading.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult_grading.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        // print IF stage
        if (state.IF.nop == 1)
        {
            printstate<<"IF.PC:\t"<<"X"<<endl; 
        }
        else
        {
            printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        }        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        // print ID stage
        if (state.ID.nop == 1)
        {
        printstate<<"ID.Instr:\t"<<"X"<<endl;         
        }
        else
        {
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl;         
        }
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

        // print EX stage
        if (state.EX.nop == 1)
        {
            printstate<<"EX.Read_data1:\t"<<"X"<<endl;
            printstate<<"EX.Read_data2:\t"<<"X"<<endl;
            printstate<<"EX.Imm:\t"<<"X"<<endl; 
            printstate<<"EX.Rs:\t"<<"X"<<endl;
            printstate<<"EX.Rt:\t"<<"X"<<endl;
            printstate<<"EX.Wrt_reg_addr:\t"<<"X"<<endl;
            printstate<<"EX.is_I_type:\t"<<"X"<<endl; 
            printstate<<"EX.rd_mem:\t"<<"X"<<endl;
            printstate<<"EX.wrt_mem:\t"<<"X"<<endl;        
            printstate<<"EX.alu_op:\t"<<"X"<<endl;
            printstate<<"EX.wrt_enable:\t"<<"X"<<endl;      
        }
        else
        {
            if (("addu" == state.EX.INS) || ("subu" == state.EX.INS))
            {
                printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
                printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
                printstate<<"EX.Imm:\t"<<"X"<<endl; 
                printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
                printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
                printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
                printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
                printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
                printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
                printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
                printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;            
            }            
            
            if ("lw" == state.EX.INS)
            {
                printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
                printstate<<"EX.Read_data2:\t"<<"X"<<endl;
                printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
                printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
                printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
                printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
                printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
                printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
                printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
                printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
                printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;            
            }
            
            if ("sw" == state.EX.INS)
            {
                printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
                printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
                printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
                printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
                printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
                printstate<<"EX.Wrt_reg_addr:\t"<<"X"<<endl;
                printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
                printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
                printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
                printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
                printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;            
            }
            
            if ("beq" == state.EX.INS)
            {
                printstate<<"EX.Read_data1:\t"<<"X"<<endl;
                printstate<<"EX.Read_data2:\t"<<"X"<<endl;
                printstate<<"EX.Imm:\t"<<"X"<<endl; 
                printstate<<"EX.Rs:\t"<<"X"<<endl;
                printstate<<"EX.Rt:\t"<<"X"<<endl;
                printstate<<"EX.Wrt_reg_addr:\t"<<"X"<<endl;
                printstate<<"EX.is_I_type:\t"<<"X"<<endl; 
                printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
                printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
                printstate<<"EX.alu_op:\t"<<"X"<<endl;
                printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;            
            }
        }                    
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;       

        // print MEM stage
        if (state.MEM.nop == 1)
        {
            printstate<<"MEM.ALUresult:\t"<<"X"<<endl;
            printstate<<"MEM.Store_data:\t"<<"X"<<endl; 
            printstate<<"MEM.Rs:\t"<<"X"<<endl;
            printstate<<"MEM.Rt:\t"<<"X"<<endl;   
            printstate<<"MEM.Wrt_reg_addr:\t"<<"X"<<endl;              
            printstate<<"MEM.rd_mem:\t"<<"X"<<endl;
            printstate<<"MEM.wrt_mem:\t"<<"X"<<endl; 
            printstate<<"MEM.wrt_enable:\t"<<"X"<<endl;        
        }
        else
        {
            if (("addu" == state.MEM.INS) || ("subu" == state.MEM.INS) || ("lw" == state.MEM.INS))
            {
                printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
                printstate<<"MEM.Store_data:\t"<<"X"<<endl; 
                printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
                printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
                printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
                printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
                printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
                printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;            
            }
            
            if ("sw" == state.MEM.INS)
            {
                printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
                printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
                printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
                printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
                printstate<<"MEM.Wrt_reg_addr:\t"<<"X"<<endl;              
                printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
                printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
                printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;            
            }
            
            if ("beq" == state.MEM.INS)
            {
                printstate<<"MEM.ALUresult:\t"<<"X"<<endl;
                printstate<<"MEM.Store_data:\t"<<"X"<<endl; 
                printstate<<"MEM.Rs:\t"<<"X"<<endl;
                printstate<<"MEM.Rt:\t"<<"X"<<endl;   
                printstate<<"MEM.Wrt_reg_addr:\t"<<"X"<<endl;              
                printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
                printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
                printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;            
            }
        }                
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl; 
        
        // print WB stage
        if (state.WB.nop == 1)
        {
            printstate<<"WB.Wrt_data:\t"<<"X"<<endl;
            printstate<<"WB.Rs:\t"<<"X"<<endl;
            printstate<<"WB.Rt:\t"<<"X"<<endl;        
            printstate<<"WB.Wrt_reg_addr:\t"<<"X"<<endl;
            printstate<<"WB.wrt_enable:\t"<<"X"<<endl;          
        }
        else
        {
            if (("addu" == state.WB.INS) || ("subu" == state.WB.INS) || ("lw" == state.WB.INS))
            {
                printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
                printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
                printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
                printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
                printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;              
            }
            
            if ("sw" == state.WB.INS)
            {
                printstate<<"WB.Wrt_data:\t"<<"X"<<endl;
                printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
                printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
                printstate<<"WB.Wrt_reg_addr:\t"<<"X"<<endl;
                printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;              
            }
            
            if ("beq" == state.WB.INS)
            {
                printstate<<"WB.Wrt_data:\t"<<"X"<<endl;
                printstate<<"WB.Rs:\t"<<"X"<<endl;
                printstate<<"WB.Rt:\t"<<"X"<<endl;        
                printstate<<"WB.Wrt_reg_addr:\t"<<"X"<<endl;
                printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;              
            }
        }               
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
      
    }
    else cout<<"Unable to open file";
    printstate.close();
}

bitset<32> sign_extend(bitset<16> imm)       //推荐这种写法 (老师写法冗杂不推荐)
{
    string extendedString;
    if (imm[15] == 0) {
        extendedString = "0000000000000000" + imm.to_string();
    } else {
        extendedString = "1111111111111111" + imm.to_string();
    }
    return bitset<32> (extendedString);
}


int main()
{
    RF myRF;                   //从相应类对每个module: RF, INSMem, DataMem分别建立一个实例对象
    INSMem myInsMem;
    DataMem myDataMem;
    
    IFStruct IF = {            //以下是对每个flip-flop(struct)进行初始化
        0,      //PC          
        0,      //nop
    };
    
    IDStruct ID = {
        0,      //Instr
        1,      //nop
    };
    
    EXStruct EX = {
        0,      //Read_data1
        0,      //Read_data2
        0,      //Imm
        0,      //Rs
        0,      //Rt
        0,      //Wrt_reg_addr
        0,      //is_I_type
        0,      //rd_mem
        0,      //wrt_mem
        1,      //alu_op
        0,      //wrt_enable
        1,      //nop
        
        "X",   //INS
    };
    
    MEMStruct MEM = {
        0,      //ALUresult
        0,      //Store_data
        0,      //Rs
        0,      //Rt
        0,      //Wrt_reg_addr
        0,      //rd_mem
        0,      //wrt_mem
        0,      //wrt_enable        
        1,      //nop
        
        "X",   //INS        
    };

    WBStruct WB = {
        0,      //Wrt_data
        0,      //Rs
        0,      //Rt
        0,      //Wrt_reg_addr
        0,      //wrt_enable
        1,      //nop
        
        "X",   //INS        
    };    
   
    stateStruct state = {    //传入已初始化的flip-flop，构建state, newstate
        IF,
        ID,
        EX ,
        MEM,
        WB,
    };
        
    stateStruct newState = state;
        
	bitset<32> Instruction;      //用于IF stage(便于判断当前是否为halt)
	bitset<32> Instr;            //用于ID stage(便于进行decode - ex的)
	string opcode;               //用于ID stage(便于判断当前指令来分别生成控制信号 - ex的)
	string func;                 //用于ID stage(作用同opcode)
	bitset<5> Rs;                //用于ID stage(便于生成EX.Read_Data1, branch的判断)
	bitset<5> Rt;                //用于ID stage(便于生成EX.Read_Data2, lw的ex.wrt_reg_addr, branch的判断)
	bitset<5> Rd;                //用于ID stage(便于生成r-type的ex.wrt_reg_addr)
    bitset<16> Imm;              //用于ID stage(便于生成branch address)

    int cycle = 0;  
    
             
    while (1) {

        /* --------------------- WB stage --------------------- */    //对register file进行更新(当前指令的wrt_enable为1时)
        if (state.WB.nop == 0)
        {
            if (state.WB.wrt_enable == 1)
            {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }         
        }


        /* --------------------- MEM stage --------------------- */   //对newState.WB进行更新(newState.WB.Wrt_data需要分类讨论); 对consume stage在MEM(sw rt)的forwarding进行实现; 对data memory进行更新(当前指令为sw时)
        if (state.MEM.nop == 0)
        {
            if (state.MEM.rd_mem == 1)              //当前指令为lw
            {
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            }
            else if (state.MEM.wrt_mem == 1)        //当前指令为sw(需要判断rt MEM-MEM forwarding,因为sw rt的consume stage在MEM)
            {
                if ((state.WB.nop == 0) && (state.WB.wrt_enable == 1) && (state.WB.Wrt_reg_addr == state.MEM.Rt))
                {
                    state.MEM.Store_data = state.WB.Wrt_data ;    
                    cout<<"MEM-MEM sw rt Forwarding"<<endl;
                }
                    
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                newState.WB.Wrt_data = state.MEM.Store_data;    //will not be used
            }
            else if (state.MEM.wrt_enable == 1)      //当前指令为addu, subu
            {
                //cout<<"addu subu ALUresult:\t"<<state.MEM.ALUresult<<endl;
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }   

            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;             
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;                      
            newState.WB.wrt_enable = state.MEM.wrt_enable;             
        }
        newState.WB.nop = state.MEM.nop;  
        newState.WB.INS = state.MEM.INS;


        /* --------------------- EX stage --------------------- */   //对consume stage在EX的(MEM-EX,EX-EX)forwarding进行实现; 对newState.MEM进行更新(ALUresult需要分类讨论)
        if (state.EX.nop == 0)
        {                       
            if ((state.WB.nop == 0) && (state.WB.wrt_enable == 1) && (state.WB.Wrt_reg_addr == state.EX.Rs))    //[2 distance rs] MEM-EX, [1 distance]上一指令为lw: stall + MEM-EX的情况
            {
                state.EX.Read_data1 = state.WB.Wrt_data;       //使用state.WB.Wrt_data做forwarding
                cout<<"MEM-EX Rs Forwarding"<<endl;
            }
            
            if ((state.WB.nop == 0) && (state.WB.wrt_enable == 1) && (state.WB.Wrt_reg_addr == state.EX.Rt))    //除当前指令为lw的[2 distance rt] MEM-EX, [1 distance]上一指令为lw: stall + MEM-EX的情况
            {
                if (((state.EX.is_I_type == 0) && (state.EX.wrt_enable == 1)) || (state.EX.wrt_mem == 1))   //当前指令addu, subu, sw
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;   //使用state.WB.Wrt_data做forwarding
                    cout<<"MEM-EX Rt Forwarding"<<endl;                
                }
            }
            
            if ((state.MEM.nop == 0) && (state.MEM.rd_mem == 0) && (state.MEM.wrt_mem == 0) && (state.MEM.wrt_enable == 1) && (state.MEM.Wrt_reg_addr == state.EX.Rs)) //除上条指令为lw,sw的[1 distance rs] EX-EX的情况
            {               
                state.EX.Read_data1 = state.MEM.ALUresult;
                cout<<"EX-EX Rs Forwarding"<<endl;
            }
            
            if ((state.MEM.nop == 0) && (state.MEM.rd_mem == 0) && (state.MEM.wrt_mem == 0) && (state.MEM.wrt_enable == 1) && (state.MEM.Wrt_reg_addr == state.EX.Rt)) //除上条指令或当前指令为lw,sw的[1 distance rt] EX-EX的情况
            {
                if ((state.EX.is_I_type == 0) && (state.EX.wrt_enable == 1))   // || (state.EX.wrt_mem == 1))   //addu, subu, for sw, we choose MEM-MEM but EX-EX
                {
                    state.EX.Read_data2 = state.MEM.ALUresult;
                    cout<<"EX-EX Rt Forwarding"<<endl;
                }
            }            
            
            if (state.EX.is_I_type == 0)          //R-type的newState.MEM.ALUresult(使用read_Data1和read_Data2)
            {
                if (state.EX.wrt_enable == 1)
                {
                    if (state.EX.alu_op == 1)           //addu
                    {
                        newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong();
                        //cout<<"addu ALUresult:\t"<<newState.MEM.ALUresult<<endl;                
                    }
                    else if (state.EX.alu_op == 0)      //subu
                    {
                        newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong();
                        //cout<<"subu ALUresult:\t"<<newState.MEM.ALUresult<<endl;                
                    }
                }
                else
                {
                    newState.MEM.ALUresult = 0;         //branch(该程序把beq指令当成了非I-type, 实际上beq是I-type指令)
                }
            }
            else if (state.EX.is_I_type == 1)     //I-type的newState.MEM.ALUresult(使用read_Data1和imm)
            {
                newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + sign_extend(state.EX.Imm).to_ulong();
            }
            
            newState.MEM.Store_data = state.EX.Read_data2;     //store_Data只有sw命令会用到=read_Data2, 对于其他指令store_data不会被用到
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;            
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;              
            newState.MEM.wrt_enable = state.EX.wrt_enable;           
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            //cout<<"Rs_data:\t"<<state.EX.Read_data1.to_ulong()<<endl;
            //cout<<"Rt_data:\t"<<state.EX.Read_data2.to_ulong()<<endl;
            //cout<<"alu_op:\t"<<state.EX.alu_op<<endl;                            
            //cout<<"addu subu ALUresult:\t"<<newState.MEM.ALUresult<<endl;            
        }
        newState.MEM.nop = state.EX.nop;        
        newState.MEM.INS = state.EX.INS;
          

        /* --------------------- ID stage --------------------- */   //对newState.EX进行更新(除了Read_data1, Read_data2, Imm, Rs, Rt, Rd之外, 其他需要按opcode分类讨论); 判断并执行branch跳转; 判断[1 distance]produce-MEM(lw), consume-EX(addu,subu,sw仅rs)的stall, 为了后面的MEM-EX forwarding
        if (state.ID.nop == 0)
        {
            Instr = state.ID.Instr;                     //借助循环外定义的Instr, opcode, func, Rs, Rt, Imm, Rd中间变量便于decode
            opcode = Instr.to_string().substr(0,6);
            func = Instr.to_string().substr(26,6);

            Rs = bitset<5>(Instr.to_string().substr(6,5));
            newState.EX.Rs = Rs;
            newState.EX.Read_data1 = myRF.readRF(Rs);   
            
            Rt = bitset<5>(Instr.to_string().substr(11,5));
            newState.EX.Rt = Rt;
            newState.EX.Read_data2 = myRF.readRF(Rt);
            
            Imm = bitset<16>(Instr.to_string().substr(16,16)); 
            newState.EX.Imm = Imm;
            
            Rd = bitset<5>(Instr.to_string().substr(16,5));
        
            //cout<<"Rs:\t"<<Rs<<endl;
            //cout<<"Rt:\t"<<Rt<<endl;   
            
            if (opcode == "000000")			//R - type: addu, subu
            {                 
                newState.EX.Wrt_reg_addr = Rd; 
                
                newState.EX.is_I_type = 0;
                
                if (func == "100001")			//addu
                {
                    newState.EX.INS = "addu";                    
                    newState.EX.alu_op = 1;     
                }
                
                else if (func == "100011")		//subu
                {
                    newState.EX.INS = "subu";                    
                    newState.EX.alu_op = 0;      
                }
                
                newState.EX.wrt_enable = 1;
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 0;                 
            }            

            else if (opcode == "100011")	//lw: R[rt] = M[R[rs]+SignExtImm]
            {
                newState.EX.INS = "lw";                
                
                newState.EX.Wrt_reg_addr = Rt;
                
                newState.EX.is_I_type = 1;               
                newState.EX.alu_op = 1;
                newState.EX.wrt_enable = 1;                
                newState.EX.rd_mem = 1;
                newState.EX.wrt_mem = 0;                      
            }

            else if (opcode == "101011")	//sw: M[R[rs]+SignExtImm] = R[rt]
            {
                newState.EX.INS = "sw";       
                
                newState.EX.Wrt_reg_addr = Rt;  //will not be used

                newState.EX.is_I_type = 1;                
                newState.EX.alu_op = 1;
                newState.EX.wrt_enable = 0;     //sw的wrt_enable = 0        
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 1;                 
            }
            
            else if (opcode == "000100")	//beq: if(R[rs]==R[rt]) PC=PC+4+BrachAddr
            {
                newState.EX.INS = "beq";
                
                newState.EX.Wrt_reg_addr = 0;
                
                newState.EX.is_I_type = 1;      //这里把beq的is_I_type设为1，但是后面处理时是将其当作0处理(逻辑漏洞)
                newState.EX.alu_op = 1;
                newState.EX.wrt_enable = 0;
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 0;
                
                if (myRF.readRF(Rs) != myRF.readRF(Rt))
                {
                    cout<<"Branch taken"<<endl;
                    newState.EX.nop = 0;       //这里可以看10月10ppt: 不管当前branch指令有效还是无效，都执行完branch的所有stage, 只不过不会更改register和data memory的状态
                    newState.ID.nop = 1;
                    newState.IF.nop = 0;       //执行下一条指令，就是跳转后的指令
                    
                    newState.IF.PC = state.IF.PC.to_ulong() + bitset<30>(sign_extend(Imm).to_string().substr(2,30)).to_ulong()*4;  //当前为ID stage,当前cycle的state.IF.PC已经是当前指令+4的PC; *4相当于+"00"
                    
                    printState(newState, cycle);     
                    state = newState;
                    cycle ++;
                    
                    continue;   //如果branch命令有效, 则直接跳过当前cycle的IF stage 
                } 
                
                cout<<"Branch not taken"<<endl;
            }
            
            if ((state.EX.nop == 0) && (state.EX.rd_mem == 1))    //上一条指令是有效的lw: [1 distance] stall + (MEM-EX), will not consider branch after lw
            {
                if ((state.EX.Wrt_reg_addr == Rs) || ((state.EX.Wrt_reg_addr == Rt) && (newState.EX.is_I_type == 0)))  //rs: lw-addu/lw-subu/lw-sw; rt:lw-addu/lw-subu;
                {                   
                    newState.EX.nop = 1;      //nop (这里可以看10月10ppt)
                    newState.ID = state.ID;   //stall
                    newState.IF = state.IF;   //stall

                    printState(newState, cycle);
                    state = newState;
                    cycle ++;
                    cout<<"Stall"<<endl;
                    continue;
                }  
            }
        }
        newState.EX.nop = state.ID.nop;

        
        /* --------------------- IF stage --------------------- */   //对newState.IF(PC和nop)进行更新, 需要分情况讨论(是否halt指令); 对newState.ID(Instr和nop)进行更新 
        if (state.IF.nop == 0)
        {
            //cout<<"PC:\t"<<state.IF.PC<<endl;
            Instruction = myInsMem.readInstr(state.IF.PC);            
            //cout<<"Instruction:\t"<<Instruction<<endl;

            if (Instruction == 0xffffffff)         //当前指令为halt (newState.IF.nop和newState.ID.nop都应该等于1)
            {
                newState.IF.PC = state.IF.PC.to_ulong();     //halt指令使pc一直保持在当前pc(halt),以确保newState.IF.nop永远为1
                newState.IF.nop = 1;
                state.IF.nop = 1;        //相当于让newState.ID.nop == 1;
                //cout<<"PC:\t"<<state.IF.PC<<endl;                                
            }
            else                                   //当前指令非halt
            {
                newState.IF.PC = state.IF.PC.to_ulong() + 4;
                newState.IF.nop = 0;    
            }
            
            newState.ID.Instr = Instruction;            
        }
        newState.ID.nop = state.IF.nop;

        
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)     //所有stage的nop都为1时退出
            break;
        
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
        state = newState; //一个cycle走完，更新一遍state
                
        cycle ++;	
    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}