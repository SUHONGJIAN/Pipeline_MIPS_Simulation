#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
    IFStruct() {
        PC = bitset<32> (0);
        nop = 0;
    }
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
    IDStruct() {
        Instr = bitset<32> (0);
        nop = 1;
    }
};

struct EXStruct {
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
    EXStruct() {
        Read_data1 = bitset<32> (0);
        Read_data2 = bitset<32> (0);
        Imm = bitset<16> (0);
        Rs = bitset<5> (0);
        Rt = bitset<5> (0);
        Wrt_reg_addr = bitset<5> (0);
        is_I_type = 0;
        rd_mem = 0;
        wrt_mem = 0;
        alu_op = 1;
        wrt_enable = 0;
        nop = 1;
    }
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;
    MEMStruct() {
        ALUresult = bitset<32> (0);
        Store_data = bitset<32> (0);
        Rs = bitset<5> (0);
        Rt = bitset<5> (0);
        Wrt_reg_addr = bitset<5> (0);
        rd_mem = 0;
        wrt_mem = 0;
        wrt_enable = 0;
        nop = 1;
    }
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
    WBStruct() {
        Wrt_data = bitset<32> (0);
        Rs = bitset<5> (0);
        Rt = bitset<5> (0);
        Wrt_reg_addr = bitset<5> (0);
        wrt_enable = 0;
        nop = 1;
    }
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
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
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
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
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
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
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
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}

bitset<32> signExtend(bitset<16> imm)
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
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;

    stateStruct state, newState;

    int cycle = 0;
             
    while (true) {
    	
        /* --------------------- WB stage --------------------- */
        //WB operation
        if (state.WB.nop == 0) {
            if (state.WB.wrt_enable == 1) {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }
        }



        /* --------------------- MEM stage --------------------- */
        //MEM operation and update newState.WB(next state)
        if (state.MEM.nop == 0) {

            if (state.MEM.rd_mem == 1) {
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            }
            else if (state.MEM.wrt_mem == 1) {
                if (state.WB.nop == 0 && state.WB.wrt_enable && state.WB.Wrt_reg_addr == state.MEM.Rt) {
                    state.MEM.Store_data = state.WB.Wrt_data;       //MEM-MEM sw Forwarding
                }

                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                newState.WB.Wrt_data = state.MEM.Store_data;   //will not be used
            }
            else if (state.MEM.wrt_enable == 1) {
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }

            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;

        }
        newState.WB.nop = state.MEM.nop;
        


        /* --------------------- EX stage --------------------- */
        //EX operation and update newState.MEM(next stage)
        if (state.EX.nop == 0) {

   //          //Forwarding to avoid RAW Hazards
   //          if (state.EX.rd_mem == 1) {        //此时命令为load
   //          	if (state.MEM.wrt_enable == 1 && state.MEM.rd_mem != 1) {      //上条命令为add/sub/halt: add-load
   //          		if (state.MEM.Wrt_reg_addr == state.EX.Rs) {
   //          			state.EX.Read_data1 = state.MEM.ALUresult;
   //          		}
   //          	}
   //          	if (state.WB.wrt_enable == 1) {            //上上条命令为add/sub/lw/halt: (2distance)add-load, load-load(多考虑了load-load?)
   //          		if (state.WB.Wrt_reg_addr == state.EX.Rs) {
   //          			state.EX.Read_data1 = state.WB.Wrt_data;
   //          		}
   //          	}
   //          }
   //          else if (state.EX.wrt_mem == 1) {       //此时命令为store
   //          	if (state.WB.wrt_enable == 1) {         //上上条命令为add/sub/lw/halt: (2distance)add-load, load-load(多考虑了load-load?)
   //          		if (state.WB.Wrt_reg_addr == state.EX.Rt) {
   //          			state.EX.Read_data2 = state.WB.Wrt_data;
   //          		}
   //          	}
   //          }
   //          else if (state.EX.wrt_enable == 1) {    //此时命令为add/sub/halt
   //          	if (state.WB.wrt_enable == 1) {      //上上条命令为add/sub/lw/halt: (2distance) add-add, load-add
   //          		if (state.WB.Wrt_reg_addr == state.EX.Rs) {           
   //          			state.EX.Read_data1 = state.WB.Wrt_data;
   //          		}
   //          		if (state.WB.Wrt_reg_addr == state.EX.Rt) {
   //          			state.EX.Read_data2 = state.WB.Wrt_data;
   //          		}
   //          	}
   //          	if (state.MEM.rd_mem == 1 && state.MEM.nop == 1) {       //上条命令为load: load-add (stall+forwarding - stall在ID stage进行)
   //          		if (state.MEM.Wrt_reg_addr == state.EX.Rs) {
   //          			// state.EX.Read_data1 = state.MEM.ALUresult;
   //          			state.EX.Read_data1 = myDataMem.readDataMem(state.MEM.ALUresult);
   //          		}
   //          		if (state.MEM.Wrt_reg_addr == state.EX.Rt) {
   //          			// state.EX.Read_data2 = state.MEM.ALUresult;
   //          			state.EX.Read_data2 = myDataMem.readDataMem(state.MEM.ALUresult);
   //          		}
   //          	}
   //          	else if (state.MEM.wrt_enable) {     //上条命令为add/sub/halt: add->add
   //          		if (state.MEM.Wrt_reg_addr == state.EX.Rs) {
   //          			state.EX.Read_data1 = state.MEM.ALUresult;
   //          		}
   //          		if (state.MEM.Wrt_reg_addr == state.EX.Rt) {
   //          			state.EX.Read_data2 = state.MEM.ALUresult;
   //          		}
   //          	}
			// }
            if (state.WB.nop == 0 && state.WB.wrt_enable == 1 && state.WB.Wrt_reg_addr == state.EX.Rs) {
                state.EX.Read_data1 = state.WB.Wrt_data;        //MEM-EX rs forwarding
            }

            if (state.WB.nop == 0 && state.WB.wrt_enable == 1 && state.WB.Wrt_reg_addr == state.EX.Rt) {
                if ((state.EX.is_I_type == 0 && state.EX.wrt_enable == 1) || state.EX.wrt_mem == 1) {    //这里还考虑了当前为store时2distance的情况(将read_Data2也就是mem.store_data改成了wb.wrt_Data)
                    state.EX.Read_data2 = state.WB.Wrt_data;       //MEM-EX rt forwarding
                }
            }

            if (state.MEM.nop == 0 && state.MEM.wrt_enable == 1 && state.MEM.Wrt_reg_addr == state.EX.Rs) {
                state.EX.Read_data1 = state.MEM.ALUresult;        //EX-EX rs forwarding
            }

            if (state.MEM.nop == 0 && state.MEM.wrt_enable == 1 && state.MEM.Wrt_reg_addr == state.EX.Rt) {
                if ((state.EX.is_I_type == 0) && (state.EX.wrt_enable == 1)) {       //这里没有考虑sw，因为我们为1distance的sw选择mem-mem,而不是ex-ex
                    state.EX.Read_data2 = state.MEM.ALUresult;        //EX-EX rt forwarding
                }
            }


            if (state.EX.is_I_type == 0) {         //R-type
                if (state.EX.wrt_enable == 1) {
                    if (state.EX.alu_op == 1) {
                        newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong();
                    }
                    else if (state.EX.alu_op == 0) {
                        newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong();
                    }
                }
                else {              //branch???
                    newState.MEM.ALUresult = 0;
                }
            }
            else if (state.EX.is_I_type == 1) {        //I-type
                newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() + signExtend(state.EX.Imm).to_ulong();
            }
            newState.MEM.Store_data = state.EX.Read_data2;
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            newState.MEM.wrt_enable = state.EX.wrt_enable;

        }
        newState.MEM.nop = state.EX.nop;
        
         

        /* --------------------- ID stage --------------------- */
        //ID operation and update newState.EX(next stage)
        if (state.ID.nop == 0) {

            newState.EX.Rs = bitset<5> (state.ID.Instr.to_string().substr(6, 5));
            newState.EX.Read_data1 = bitset<32> (myRF.readRF(newState.EX.Rs));
            newState.EX.Rt = bitset<5> (state.ID.Instr.to_string().substr(11, 5));
            newState.EX.Read_data2 = bitset<32> (myRF.readRF(newState.EX.Rt));
            newState.EX.Imm = bitset<16> (state.ID.Instr.to_string().substr(16, 16));

            if (state.ID.Instr.to_string().substr(0, 6) == "000000") {        //R-type
                newState.EX.Wrt_reg_addr = bitset<5> (state.ID.Instr.to_string().substr(16, 5));
                newState.EX.is_I_type = 0;
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 0;
                if (state.ID.Instr.to_string().substr(26, 6) == "100001") {    //addu
                    newState.EX.alu_op = 1;
                }
                else if (state.ID.Instr.to_string().substr(26, 6) == "100011") {    //subu
                    newState.EX.alu_op = 0;
                }
                newState.EX.wrt_enable = 1;
            }
            else if (state.ID.Instr.to_string().substr(0, 6) == "100011") {       //lw
                newState.EX.Wrt_reg_addr = bitset<5> (state.ID.Instr.to_string().substr(11, 5));
                newState.EX.is_I_type = 1;
                newState.EX.rd_mem = 1;
                newState.EX.wrt_mem = 0;
                newState.EX.alu_op = 1;
                newState.EX.wrt_enable = 1;
            }
            else if (state.ID.Instr.to_string().substr(0, 6) == "101011") {       //sw
                newState.EX.Wrt_reg_addr = bitset<5> (state.ID.Instr.to_string().substr(11, 5));     //will not be used
                newState.EX.is_I_type = 1;
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 1;
                newState.EX.alu_op = 1;
                newState.EX.wrt_enable = 0;
            }
            else if (state.ID.Instr.to_string().substr(0, 6) == "000100") {        //beq(bne)
                newState.EX.Wrt_reg_addr = 0;
                newState.EX.is_I_type = 0;
                newState.EX.rd_mem = 0;
                newState.EX.wrt_mem = 0;
                newState.EX.alu_op = 1;
                newState.EX.wrt_enable = 0;
                if (myRF.readRF(bitset<5> (state.ID.Instr.to_string().substr(6, 5))) != myRF.readRF(bitset<5> (state.ID.Instr.to_string().substr(11, 5)))) {
                    newState.IF.PC = state.IF.PC.to_ulong() + bitset<30> (signExtend(bitset<16> (state.ID.Instr.to_ulong())).to_string().substr(2, 30)).to_ulong() * 4;
                    newState.ID.nop = 1;
                    newState.EX.nop = 0;
                    newState.IF.nop = 0;

                    printState(newState, cycle++);
                    state = newState;
                    continue;
                }
            }

            if (state.EX.nop == 0 && state.EX.rd_mem == 1 && state.ID.Instr.to_string().substr(0, 6) == "000000") {      //上条指令是store，这条指令是r-type
                if (state.EX.Wrt_reg_addr == bitset<5> (state.ID.Instr.to_string().substr(6, 5)) || state.EX.Wrt_reg_addr == bitset<5> (state.ID.Instr.to_string().substr(11, 5))) {
                    newState.ID = state.ID;
                    newState.IF = state.IF;
                    newState.EX.nop = 1;

                    printState(newState, cycle++);
                    state = newState;
                    continue;
                }
            }
        }
        newState.EX.nop = state.ID.nop;

            

        /* --------------------- IF stage --------------------- */
        //IF operation and update newState.ID(next stage)
        if (state.IF.nop == 0) {

            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);

            if (myInsMem.readInstr(state.IF.PC).to_string() != "11111111111111111111111111111111") {
                newState.IF.PC = state.IF.PC.to_ulong() + 4;
                newState.IF.nop = 0;
            } 
            else {
                newState.IF.PC = state.IF.PC.to_ulong();
                state.IF.nop = 1;
                newState.IF.nop = 1;
            }

        }
        newState.ID.nop = state.IF.nop;



        /* --------------------- Extra stage --------------------- */
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop) break;

        //Print results to stateresult.txt file and update whole state.
        printState(newState, cycle++); //print states after executing cycle 0, cycle 1, cycle 2 ... 
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */ 

    }

    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}
