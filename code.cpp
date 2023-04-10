﻿
#include <iostream>
#include <map>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <bitset>
#include <sstream>
#include <math.h>

using namespace std;

#define REG_NUM 32 // register number

#define MEMO_SIZE 100 // memory size

class Instruction;

class RegAndMemory
{

public:
    uint32_t reg[REG_NUM];

    char memo[MEMO_SIZE];

    uint32_t pc = 0;

    RegAndMemory()
    {
        for (int i = 0; i < REG_NUM; i++)
        {
            reg[i] = 0;
        }
        memset(memo, 0, MEMO_SIZE);
    }

    void LoadCode(vector<uint32_t> code)
    {
        for(int i=0; i<code.size(); i++)
        {
            this->SetUint32(code[i], 4*i);
        }
    }

    int Execute();

    void SetUint32(uint32_t value, uint32_t address)
    {
        uint64_t div = 1;
        div <<= 8;
        for (int i=3; i>=0; i--)
        {
            memo[address + i] = (value % div);
            value >>= 8;
        }
    }

    uint32_t GetUint32(uint32_t address)
    {
        uint32_t res = 0;
        for (int i = 0; i < 4; i++)
        {
            res <<= 8;
            res += memo[address + i];
        }

        return res;
    }

    string toString()
    {
        char temps[100];
        string res;

        res += "        B D H \n";

        int flag = 0;
        for (int i = 0; i < REG_NUM; i++)
        {
            res += "reg " + to_string(i) + " : ";
            itoa(reg[i], temps, 2);
            res += temps;
            res += " ";
            itoa(reg[i], temps, 10);
            res += temps;
            res += " ";
            itoa(reg[i], temps, 16);
            res += temps;
            res += " ";

            if(flag == 3)
            {
                res += "\n";
                flag = -1;
            }
            else 
            {
                res += "        ";
            }

            flag++;
        }

        int counter = 0;
        int pc_counter = pc;
        for (int i = 0; i < MEMO_SIZE; i++)
        {
            itoa(memo[i], temps, 16);
            res += temps;
            res += "   ";
            counter++;
            pc_counter --;
            if (pc_counter == 0)
            {
                res += " << ";
            }
            if (counter == 4)
            {
                res += "\n";
                counter = 0;
            }
        }

        return res;
    }
};

vector<string> split(string str, string delim)
{
    vector<string> res;
    if ("" == str)
        return res;

    char* strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());

    char* d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char* p = strtok(strs, d);

    while (p)
    {
        string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }

    return res;
}

class Instruction
{
public:
    RegAndMemory* target;

    // register name -> register number
    static map<string, int> map_of_reg_num;

    // register number -> register name
    static map<int, string> map_of_reg_str;

    // oprate code name -> oprate code in uint32
    static map<string, uint32_t> map_of_asm;

    static map<uint32_t, string> map_of_opcode;

    enum InstructionType
    {
        NULLTYPE,
        BITYPE,
        ASMTYPE,
        COMPLETETYPE
    };

    InstructionType instruction_type = NULLTYPE;

    // splited instruction
    vector<string> splt_instruction;

    // the length of splited instruction
    int asm_size = 0;

    //
    uint32_t bi_instruction = 0;

    // 00000000010000000001000
    //        index2    index1
    // get uint value between index2 and index1
    // -> -0000000001

    static int getUValueFromBits(uint32_t bi_code, int index2, int index1)
    {
        int res = 0;

        for (int i = 0; i < 31 - index2; i++)
        {
            bi_code <<= 1;
        }

        for (int i = 0; i <= index2 - index1; i++)
        {
            res = res * 2 + (bi_code >= 2147483648);
            bi_code <<= 1;
        }

        return res;
    }

    // 00000000010000000001000
    //        index2    index1
    // get int value between index2 and index1
    // -> 10000000001

    static int64_t getValueFromBits(uint32_t bi_code, int index2, int index1)
    {
        int64_t res = 0;
        int flag = 1;

        for (int i = 0; i < 31 - index2; i++)
        {
            bi_code <<= 1;
        }

        if (bi_code >= 2147483648)
        {
            flag = -1;
            bi_code <<= 1;
        }
        else 
        {
            bi_code <<= 1;
        }

        for (int i = 0; i < index2 - index1; i++)
        {
            res = res * 2;
            if (flag == 1 && bi_code >= 2147483648)
            {
                res += 1;
            }
            else if (flag == -1 && bi_code < 2147483648)
            {
                res += 1;
            }
            bi_code <<= 1;
        }
        if(flag == -1)
        {
            res += 1;
        }

        return res * flag;
    }

    static void InitMap()
    {
        map_of_reg_num["$zero"] = 0;
        map_of_reg_num["$at"] = 1;
        map_of_reg_num["$v0"] = 2;
        map_of_reg_num["$v1"] = 3;

        map_of_reg_num["$a0"] = 4;
        map_of_reg_num["$a1"] = 5;
        map_of_reg_num["$a2"] = 6;
        map_of_reg_num["$a3"] = 7;

        map_of_reg_num["$t0"] = 8;
        map_of_reg_num["$t1"] = 9;
        map_of_reg_num["$t2"] = 10;
        map_of_reg_num["$t3"] = 11;
        map_of_reg_num["$t4"] = 12;
        map_of_reg_num["$t5"] = 13;
        map_of_reg_num["$t6"] = 14;
        map_of_reg_num["$t7"] = 15;

        map_of_reg_num["$s0"] = 16;
        map_of_reg_num["$s1"] = 17;
        map_of_reg_num["$s2"] = 18;
        map_of_reg_num["$s3"] = 19;
        map_of_reg_num["$s4"] = 20;
        map_of_reg_num["$s5"] = 21;
        map_of_reg_num["$s6"] = 22;
        map_of_reg_num["$s7"] = 23;

        map_of_reg_num["$t8"] = 24;
        map_of_reg_num["$t9"] = 25;
        map_of_reg_num["$k0"] = 26;
        map_of_reg_num["$k1"] = 27;

        map_of_reg_num["$gp"] = 28;
        map_of_reg_num["$sp"] = 29;
        map_of_reg_num["$fp"] = 30;
        map_of_reg_num["$ra"] = 31;

        map_of_reg_str[0] = "$zero";
        map_of_reg_str[1] = "$at";
        map_of_reg_str[2] = "$v0";
        map_of_reg_str[3] = "$v1";

        map_of_reg_str[4] = "$a0";
        map_of_reg_str[5] = "$a1";
        map_of_reg_str[6] = "$a2";
        map_of_reg_str[7] = "$a3";

        map_of_reg_str[8] = "$t0";
        map_of_reg_str[9] = "$t1";
        map_of_reg_str[10] = "$t2";
        map_of_reg_str[11] = "$t3";
        map_of_reg_str[12] = "$t4";
        map_of_reg_str[13] = "$t5";
        map_of_reg_str[14] = "$t6";
        map_of_reg_str[15] = "$t7";

        map_of_reg_str[16] = "$s0";
        map_of_reg_str[17] = "$s1";
        map_of_reg_str[18] = "$s2";
        map_of_reg_str[19] = "$s3";
        map_of_reg_str[20] = "$s4";
        map_of_reg_str[21] = "$s5";
        map_of_reg_str[22] = "$s6";
        map_of_reg_str[23] = "$s7";

        map_of_reg_str[24] = "$t8";
        map_of_reg_str[25] = "$t9";
        map_of_reg_str[26] = "$k0";
        map_of_reg_str[27] = "$k1";

        map_of_reg_str[28] = "$gp";
        map_of_reg_str[29] = "$sp";
        map_of_reg_str[30] = "$fp";
        map_of_reg_str[31] = "$ra";

        
        map_of_asm["slt"] = 0b000000;
        map_of_asm["add"] = 0b000000;
        map_of_asm["addi"] = 0b001000;
        map_of_asm["andi"] = 0b001100;
        map_of_asm["and"] = 0b000000;
        map_of_asm["ori"] = 0b001101;
        map_of_asm["xori"] = 0b001110;
        map_of_asm["beq"] = 0b000100;
        map_of_asm["jal"] = 0b000011;
        map_of_asm["jr"] = 0b000000;


        map_of_opcode[0b000000] = "add";
        map_of_opcode[0b001000] = "addi";
        map_of_opcode[0b001100] = "andi";
        map_of_opcode[0b001101] = "ori";
        map_of_opcode[0b001110] = "xori";
        map_of_opcode[0b000100] = "beq";
        map_of_opcode[0b000011] = "jal";
    }

    Instruction(string str, bool is_asm)
    {
        if (is_asm)
        {
            splt_instruction = split(str, " ,");
            this->instruction_type = InstructionType::ASMTYPE;
        }
        else
        {
            if(str.length() != 32)
            {
                this->instruction_type = NULLTYPE;
                return;
            }
            this->instruction_type = Instruction::BITYPE;
            this->bi_instruction = stoi(str.c_str(), 0, 2);
        }
    }

    void BitTypeConvert_addi();
    void BitTypeConvert_andi();
    void BitTypeConvert_ori();
    void BitTypeConvert_xori();
    void BitTypeConvert_beq();
    void BitTypeConvert_jal();
    void BitTypeConvert_jr();
    void BitTypeConvert_add( );
    void BitTypeConvert_and( );
    void BitTypeConvert_slt( );
    void BitTypeConvert_1();
    void BitTypeConvert_2();
    void BitTypeConvert_3();
    void BitTypeConvert_4();
    void BitTypeConvert_5();
    void BitTypeConvert_6();
    void BitTypeConvert_7();
    void BitTypeConvert_8();
    void BitTypeConvert_9();

    void BitTypeConvert()
    {
        if(this->instruction_type == NULLTYPE)
        {
            return;
        }
        int opcode = getUValueFromBits(this->bi_instruction, 31, 26);
        int func = getUValueFromBits(this->bi_instruction, 5, 0);

        switch (opcode)
        {
            case 0b000000: // add
                switch (func)
                {
                    case 0b001000:
                        BitTypeConvert_jr();
                        break;
                    case 0b100000:
                        BitTypeConvert_add( );
                        break;
                    case 0b100100:
                        BitTypeConvert_and( );
                        break;  
                    case 0b101010:
                        BitTypeConvert_slt( );
                        break; 
                    default:
                        break;
                }
                /* code */
                break;

            case 0b001000: // addi
                BitTypeConvert_addi();
                break;

            case 0b001100: // andi
                BitTypeConvert_andi();
                break;

            case 0b001101: // ori
                BitTypeConvert_ori();
                break;

            case 0b001110: // xori
                BitTypeConvert_xori();
                break;

            case 0b000100:
                BitTypeConvert_beq();
                break;

            case 0b000011:
                BitTypeConvert_jal();
                break;

            default:
                this->instruction_type = NULLTYPE;
                break;
        }
    }

    void AsmTypeConvert_addi();
    void AsmTypeConvert_andi();
    void AsmTypeConvert_ori();
    void AsmTypeConvert_xori();
    void AsmTypeConvert_beq();
    void AsmTypeConvert_jal();
    void AsmTypeConvert_jr();
    void AsmTypeConvert_and();
    void AsmTypeConvert_add( );
    void AsmTypeConvert_slt( );
    void AsmTypeConvert_1();
    void AsmTypeConvert_2();
    void AsmTypeConvert_3();
    void AsmTypeConvert_4();
    void AsmTypeConvert_5();
    void AsmTypeConvert_6();
    void AsmTypeConvert_7();
    void AsmTypeConvert_8();
    void AsmTypeConvert_9();

    void AsmTypeConvert()
    {
        if(this->splt_instruction.size() == 0)
        {
            this->instruction_type = NULLTYPE;

            return;
        }
        switch (map_of_asm[this->splt_instruction[0]])
        {
            case 0b000000: // add

                if (this->splt_instruction[0] == "jr") {
                    AsmTypeConvert_jr();
                }
                else if( this->splt_instruction[0] == "add" )
                {
                    AsmTypeConvert_add( );
                }
                else if( this->splt_instruction[0] == "and" )
                {
                    AsmTypeConvert_and( );
                }
                else if( this->splt_instruction[0] == "slt" )
                {
                    AsmTypeConvert_slt( );
                }
                else
                    ;
                /* code */
                break;

            case 0b001000: // addi
                AsmTypeConvert_addi();
                break;
            case 0b001101: // ori
                AsmTypeConvert_ori();
                break;

            case 0b001100: // andi
                cout << "andi" << endl;
                AsmTypeConvert_andi();
                break;

            case 0b001110: // xori
                AsmTypeConvert_xori();
                break;

            case 0b000100://beq
                AsmTypeConvert_beq();
                break;

            case 0b000011://jal
                AsmTypeConvert_jal();
                break;

            default:
                this->instruction_type = NULLTYPE;
                break;
        }
    }

    void toCompeleteType()
    {
        switch (this->instruction_type)
        {
            case Instruction::InstructionType::NULLTYPE:
                // cout << "NULLTYPE fail to complete" << endl;
                break;
            case Instruction::InstructionType::BITYPE:
                this->BitTypeConvert();
                break;
            case Instruction::InstructionType::ASMTYPE:
                this->AsmTypeConvert();
                break;
            case Instruction::InstructionType::COMPLETETYPE:
                cout << "Already completed" << endl;
                break;

            default:
                break;
        }
    }

    string toString()
    {
        string res;
        int counter = 0;
        for (string stp : splt_instruction)
        {
            counter ++;
            res += stp;
            if(counter > 1 && counter != splt_instruction.size())
            {
                res += ", ";
            }
            else 
            {
                res += " ";
            }
        }

        return res;
    }

    void Exe_addi(RegAndMemory* target);
    void Exe_andi(RegAndMemory* target);
    void Exe_ori(RegAndMemory* target);
    void Exe_xori(RegAndMemory* target);
    void Exe_beq(RegAndMemory* target);
    void Exe_jal(RegAndMemory* target);
    void Exe_jr(RegAndMemory* target);
    void Exe_add(RegAndMemory* target);
    void Exe_and(RegAndMemory* target);
    void Exe_slt(RegAndMemory* target);
    void Exe_1(RegAndMemory* target);
    void Exe_2(RegAndMemory* target);
    void Exe_3(RegAndMemory* target);
    void Exe_4(RegAndMemory* target);
    void Exe_5(RegAndMemory* target);
    void Exe_6(RegAndMemory* target);
    void Exe_7(RegAndMemory* target);
    void Exe_8(RegAndMemory* target);
    void Exe_9(RegAndMemory* target);

    void Execute(RegAndMemory* target)
    {
        int opcode = Instruction::getUValueFromBits(this->bi_instruction, 31, 26);

        int func = getUValueFromBits(this->bi_instruction, 5, 0);

        switch (opcode)
        {
            case 0b000000:
                switch (func)
                {
                    case 0b001000://jr
                        Exe_jr(target);
                        break;
                    case 0b100000:
                        Exe_add( target );
                        break;
                    case 0b100100:
                        Exe_and( target );
                        break;
                    case 0b101010:
                        Exe_slt( target );
                        break;
                    default:
                        break;
                }
                /* code */
                break;

            case 0b001000: // addi
                Exe_addi(target);
                break;

            case 0b001100: // andi
                Exe_andi(target);
                break;

            case 0b001101: // ori
                Exe_ori(target);
                break;

            case 0b001110: // xori
                Exe_xori(target);
                break;

            case 0b000100://beq
                Exe_beq(target);
                break;

            case 0b000011://jal
                Exe_jal(target);
                break;

            default:
                break;
        }
    }
};

int RegAndMemory::Execute()
{
    uint32_t value = this->GetUint32(pc);

    if(value == 0)
    {
        return 0;
    }

    pc += 4;
    stringstream sst;
    sst << bitset<32>(value);
    string code;
    sst >> code;
    Instruction ins(code, false);
    ins.Execute(this);

    return 1;
}

map<string, int> Instruction::map_of_reg_num = map<string, int>();

map<int, string> Instruction::map_of_reg_str = map<int, string>();

map<string, uint32_t> Instruction::map_of_asm = map<string, uint32_t>();

map<uint32_t, string> Instruction::map_of_opcode = map<uint32_t, string>();

void Instruction :: BitTypeConvert_add( )
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back(this->map_of_opcode[opcode]);
    int reg = this->getUValueFromBits(this->bi_instruction, 15, 11);            //des_register;
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
    reg = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
    reg = this->getUValueFromBits(this->bi_instruction, 20, 16);
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);

}

void Instruction :: AsmTypeConvert_add( )
{
    int64_t adding_value = 0;
    adding_value = 0b000000;
    adding_value <<= 26;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[1]];       
    adding_value <<= 11;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[2]];
    adding_value <<= 21;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[3]];
    adding_value <<= 16;
    this->bi_instruction += adding_value;
    this->bi_instruction += 32;            
    return;
}

void Instruction::Exe_add( RegAndMemory *target )
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error : not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 15, 11);
        int reg2 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        int reg3 = this->getValueFromBits(this->bi_instruction, 20, 16);
        cout << "reg1 : " << reg1
            << "reg2 : " << reg2
            << "reg3: " << reg3 << endl;
        target->reg[reg1] = target->reg[reg2] + target->reg[reg3];
    }
}

void Instruction :: BitTypeConvert_slt( )
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back("slt" );
    int reg = this->getUValueFromBits(this->bi_instruction, 15, 11);            //des_register;
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
    reg = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
    reg = this->getUValueFromBits(this->bi_instruction, 20, 16);
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);

}

void Instruction :: AsmTypeConvert_slt( )
{
    int64_t adding_value = 0;
    adding_value = 0b000000;
    adding_value <<= 26;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[1]];       
    adding_value <<= 11;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[2]];
    adding_value <<= 21;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[3]];
    adding_value <<= 16;
    this->bi_instruction += adding_value;
    this->bi_instruction += 0b101010;             
    return;
}

void Instruction::Exe_slt( RegAndMemory *target )
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error : not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 15, 11);
        int reg2 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        int reg3 = this->getValueFromBits(this->bi_instruction, 20, 16);
        cout << "reg1 : " << reg1
            << "reg2 : " << reg2
            << "reg3: " << reg3 << endl;
        target->reg[reg1] = target->reg[reg2] < target->reg[reg3];
    }
}

void Instruction :: BitTypeConvert_and( )
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back( "and" );
    int reg = this->getUValueFromBits(this->bi_instruction, 15, 11);            //des_register;
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
    reg = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
    reg = this->getUValueFromBits(this->bi_instruction, 20, 16);
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
}

void Instruction :: AsmTypeConvert_and( )
{
    int64_t adding_value = 0;
    adding_value = 0b000000;
    adding_value <<= 26;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[1]];      
    adding_value <<= 11;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[2]];
    adding_value <<= 21;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[3]];
    adding_value <<= 16;
    this->bi_instruction += adding_value;
    this->bi_instruction += 36;             
    return;
}

void Instruction::Exe_and( RegAndMemory *target )
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error : not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 15, 11);
        int reg2 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        int reg3 = this->getValueFromBits(this->bi_instruction, 20, 16);
        cout << "reg1 : " << reg1
            << "reg2 : " << reg2
            << "reg3: " << reg3 << endl;
        target->reg[reg1] = target->reg[reg2] & target->reg[reg3];
    }
}

void Instruction::BitTypeConvert_addi()
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back(this->map_of_opcode[opcode]);
    int reg = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
    reg = this->getUValueFromBits(this->bi_instruction, 20, 16);
    this->splt_instruction.push_back(this->map_of_reg_str[reg]);
    string offset = to_string(this->getValueFromBits(this->bi_instruction, 15, 0));
    this->splt_instruction.push_back(offset);
}

void Instruction::AsmTypeConvert_addi()
{
    int64_t adding_value = 0;
    adding_value = 0b001000;
    adding_value <<= 26;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[1]];
    adding_value <<= 21;
    this->bi_instruction += adding_value;
    adding_value = map_of_reg_num[this->splt_instruction[2]];
    adding_value <<= 16;
    this->bi_instruction += adding_value;
    adding_value = atoi(this->splt_instruction[3].c_str());
    this->bi_instruction += adding_value;
    return;
}

void Instruction::Exe_addi(RegAndMemory* target)
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error : not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
        int value = this->getValueFromBits(this->bi_instruction, 15, 0);
        cout << "reg1 : " << reg1
            << "reg2 : " << reg2
            << "val: " << value << endl;
        target->reg[reg1] = target->reg[reg2] + value;
    }
}

void Instruction::BitTypeConvert_andi()
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back(this->map_of_opcode[opcode]);
    int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg1]);
    int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
    this->splt_instruction.push_back(this->map_of_reg_str[reg2]);
    int imm = this->getUValueFromBits(this->bi_instruction, 15, 0);
    this->splt_instruction.push_back(to_string(imm));
}

void Instruction::AsmTypeConvert_andi()
{
    int64_t andi_value = 0;
    andi_value = 0b001100;
    andi_value <<= 26;
    this->bi_instruction += andi_value;
    andi_value = map_of_reg_num[this->splt_instruction[1]];
    andi_value <<= 21;
    this->bi_instruction += andi_value;
    andi_value = map_of_reg_num[this->splt_instruction[2]];
    andi_value <<= 16;
    this->bi_instruction += andi_value;
    andi_value = atoi(this->splt_instruction[3].c_str());
    this->bi_instruction += andi_value;
    return;
}

void Instruction::Exe_andi(RegAndMemory* target)
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error: not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
        int imm = this->getUValueFromBits(this->bi_instruction, 15, 0);
        cout << "reg1: " << reg1 << " reg2: " << reg2 << " imm: " << imm << endl;
        // only for testing purpose
        // target->reg[reg2] = 16;
        target->reg[reg1] = target->reg[reg2] & imm;
    }
}

void Instruction::BitTypeConvert_ori()
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back(this->map_of_opcode[opcode]);
    int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg1]);
    int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
    this->splt_instruction.push_back(this->map_of_reg_str[reg2]);
    int imm = this->getUValueFromBits(this->bi_instruction, 15, 0);
    this->splt_instruction.push_back(to_string(imm));
}

void Instruction::AsmTypeConvert_ori()
{
    int64_t ori_value = 0;
    ori_value = 0b001101;
    ori_value <<= 26;
    this->bi_instruction += ori_value;
    ori_value = map_of_reg_num[this->splt_instruction[1]];
    ori_value <<= 21;
    this->bi_instruction += ori_value;
    ori_value = map_of_reg_num[this->splt_instruction[2]];
    ori_value <<= 16;
    this->bi_instruction += ori_value;
    ori_value = atoi(this->splt_instruction[3].c_str());
    this->bi_instruction += ori_value;
    return;
}

void Instruction::Exe_ori(RegAndMemory* target)
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error: not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
        int imm = this->getUValueFromBits(this->bi_instruction, 15, 0);
        cout << "reg1: " << reg1 << " reg2: " << reg2 << " imm: " << imm << endl;
        // only for testing purpose
        // target->reg[reg2] = 16;
        target->reg[reg1] = target->reg[reg2] | imm;
    }
}

void Instruction::BitTypeConvert_xori()
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back(this->map_of_opcode[opcode]);
    int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg1]);
    int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
    this->splt_instruction.push_back(this->map_of_reg_str[reg2]);
    int imm = this->getUValueFromBits(this->bi_instruction, 15, 0);
    this->splt_instruction.push_back(to_string(imm));
}

void Instruction::AsmTypeConvert_xori()
{
    int64_t xori_value = 0;
    xori_value = 0b001110;
    xori_value <<= 26;
    this->bi_instruction += xori_value;
    xori_value = map_of_reg_num[this->splt_instruction[1]];
    xori_value <<= 21;
    this->bi_instruction += xori_value;
    xori_value = map_of_reg_num[this->splt_instruction[2]];
    xori_value <<= 16;
    this->bi_instruction += xori_value;
    xori_value = atoi(this->splt_instruction[3].c_str());
    this->bi_instruction += xori_value;
    return;
}

void Instruction::Exe_xori(RegAndMemory* target)
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error: not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
        int imm = this->getUValueFromBits(this->bi_instruction, 15, 0);
        cout << "reg1: " << reg1 << " reg2: " << reg2 << " imm: " << imm << endl;
        // only for testing purpose
        // target->reg[reg2] = 16;
        target->reg[reg1] = target->reg[reg2] ^ imm;
    }
}


void Instruction::BitTypeConvert_beq()
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back(this->map_of_opcode[opcode]);
    int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg1]);
    int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
    this->splt_instruction.push_back(this->map_of_reg_str[reg2]);
    int offset = this->getUValueFromBits(this->bi_instruction, 15, 0);
    this->splt_instruction.push_back(to_string(offset));
}

void Instruction::AsmTypeConvert_beq()
{
    int64_t beq_value = 0;
    beq_value = 0b000100;
    beq_value <<= 26;
    this->bi_instruction += beq_value;
    beq_value = map_of_reg_num[this->splt_instruction[1]];
    beq_value <<= 21;
    this->bi_instruction += beq_value;
    beq_value = map_of_reg_num[this->splt_instruction[2]];
    beq_value <<= 16;
    this->bi_instruction += beq_value;
    beq_value = atoi(this->splt_instruction[3].c_str());
    this->bi_instruction += beq_value;
    return;
}

void Instruction::Exe_beq(RegAndMemory* target)
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error: not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        int reg2 = this->getUValueFromBits(this->bi_instruction, 20, 16);
        int offset = this->getUValueFromBits(this->bi_instruction, 15, 0);
        cout << "reg1: " << reg1 << " reg2: " << reg2 << " offset: " << offset << endl;
        // only for testing purpose
        // target->reg[reg2] = 16;
        if (target->reg[reg1] == target->reg[reg2]) {
            target->pc = target->pc + offset;
        }
    }
}



void Instruction::BitTypeConvert_jal()
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back(this->map_of_opcode[opcode]);
    int target = this->getUValueFromBits(this->bi_instruction, 25, 0);
    this->splt_instruction.push_back(to_string(target));
}

void Instruction::AsmTypeConvert_jal()
{
    int64_t jal_value = 0;
    jal_value = 0b000011;
    jal_value <<= 26;
    this->bi_instruction += jal_value;
    jal_value = atoi(this->splt_instruction[1].c_str());
    this->bi_instruction += jal_value;
    return;
}

void Instruction::Exe_jal(RegAndMemory* target)
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error: not able to execute (null or asm only)" << endl;
    }
    else
    {
        int ra = 31;//map_of_reg_num["$ra"]

        int addr_target = this->getUValueFromBits(this->bi_instruction, 25, 0);
        cout << " target: " << addr_target << endl;
        // only for testing purpose
        // target->reg[reg2] = 16;
        target->reg[ra] = target->pc;
        target->pc = (target->pc & 0xFA000000) + addr_target;
    }
}


void Instruction::BitTypeConvert_jr()
{
    int opcode = this->getUValueFromBits(this->bi_instruction, 31, 26);
    this->splt_instruction.push_back("jr");
    int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
    this->splt_instruction.push_back(this->map_of_reg_str[reg1]);
    int func = this->getUValueFromBits(this->bi_instruction, 5, 0);
    this->splt_instruction.push_back(to_string(func));
}

void Instruction::AsmTypeConvert_jr()
{
    int64_t jr_value = 0;
    jr_value = 0b000000;
    jr_value <<= 26;
    this->bi_instruction += jr_value;
    jr_value = map_of_reg_num[this->splt_instruction[1]];
    jr_value <<= 21;
    this->bi_instruction += jr_value;
    jr_value = 0b001000;
    this->bi_instruction += jr_value;
    return;
}

void Instruction::Exe_jr(RegAndMemory* target)
{
    if (this->instruction_type == InstructionType::ASMTYPE || this->instruction_type == InstructionType::NULLTYPE)
    {
        cout << "Error: not able to execute (null or asm only)" << endl;
    }
    else
    {
        int reg1 = this->getUValueFromBits(this->bi_instruction, 25, 21);
        cout << " reg1: " << reg1 << endl;
        target->pc = target->reg[reg1];
    }
}




void Instruction::BitTypeConvert_1()
{
}

void Instruction::AsmTypeConvert_1()
{
}

void Instruction::Exe_1(RegAndMemory* target)
{
}

void Instruction::BitTypeConvert_2()
{
}

void Instruction::AsmTypeConvert_2()
{
}

void Instruction::Exe_2(RegAndMemory* target)
{
}

void Instruction::BitTypeConvert_3()
{
}

void Instruction::AsmTypeConvert_3()
{
}

void Instruction::Exe_3(RegAndMemory* target)
{
}

void Instruction::BitTypeConvert_4()
{
}

void Instruction::AsmTypeConvert_4()
{
}

void Instruction::Exe_4(RegAndMemory* target)
{
}

void Instruction::BitTypeConvert_5()
{
}

void Instruction::AsmTypeConvert_5()
{
}

void Instruction::Exe_5(RegAndMemory* target)
{
}

void Instruction::BitTypeConvert_6()
{
}

void Instruction::AsmTypeConvert_6()
{
}

void Instruction::Exe_6(RegAndMemory* target)
{
}

void Instruction::BitTypeConvert_7()
{
}

void Instruction::AsmTypeConvert_7()
{
}

void Instruction::Exe_7(RegAndMemory* target)
{
}

void Instruction::BitTypeConvert_8()
{
}

void Instruction::AsmTypeConvert_8()
{
}

void Instruction::Exe_8(RegAndMemory* target)
{
}

void Instruction::BitTypeConvert_9()
{
}

void Instruction::AsmTypeConvert_9()
{
}

void Instruction::Exe_9(RegAndMemory* target)
{
}

void test_addi_instruction()
{
    RegAndMemory reg_mem;

    string addi_asm = "addi $s0, $s1, 4";
    Instruction addi_instr(addi_asm, true);
    Instruction::InitMap();

    addi_instr.toCompeleteType();
    cout << bitset<32>(addi_instr.bi_instruction) << endl;

    stringstream sst;

    sst << bitset<32>(addi_instr.bi_instruction);

    string bi_str;

    sst >> bi_str;

    Instruction ins_test(bi_str, false);

    ins_test.toCompeleteType();

    ins_test.Execute(&reg_mem);

    cout << reg_mem.toString() << endl;
}

void test_andi_instruction()
{
    // Create a new instance of RegAndMemory
    RegAndMemory reg_mem;

    // Create an ANDI instruction and assemble it
    string andi_asm = "andi $s0, $s1, 0";
    Instruction andi_instr(andi_asm, true);
    Instruction::InitMap();

    // Convert the instruction to binary and print it
    andi_instr.toCompeleteType();
    cout << bitset<32>(andi_instr.bi_instruction) << endl;

    // Create a new Instruction object from the binary string and execute it
    stringstream sst;
    sst << bitset<32>(andi_instr.bi_instruction);
    string bi_str;
    sst >> bi_str;
    Instruction ins_test(bi_str, false);
    ins_test.toCompeleteType();
    ins_test.Execute(&reg_mem);

    // Print the register and memory values after the execution
    cout << reg_mem.toString() << endl;
}

void test_ori_instruction()
{
    // Create a new instance of RegAndMemory
    RegAndMemory reg_mem;

    // Create an ORI instruction and assemble it
    string ori_asm = "ori $s0, $s1, 0";
    Instruction ori_instr(ori_asm, true);
    Instruction::InitMap();

    // Convert the instruction to binary and print it
    ori_instr.toCompeleteType();
    cout << bitset<32>(ori_instr.bi_instruction) << endl;

    // Create a new Instruction object from the binary string and execute it
    stringstream sst;
    sst << bitset<32>(ori_instr.bi_instruction);
    string bi_str;
    sst >> bi_str;
    Instruction ins_test(bi_str, false);
    ins_test.toCompeleteType();
    ins_test.Execute(&reg_mem);

    // Print the register and memory values after the execution
    cout << reg_mem.toString() << endl;
}

void test_xori_instruction()
{
    // Create a new instance of RegAndMemory
    RegAndMemory reg_mem;

    // Create an ORI instruction and assemble it
    string ori_asm = "xori $s0, $s1, 1";
    Instruction ori_instr(ori_asm, true);
    Instruction::InitMap();

    // Convert the instruction to binary and print it
    ori_instr.toCompeleteType();
    cout << bitset<32>(ori_instr.bi_instruction) << endl;

    // Create a new Instruction object from the binary string and execute it
    stringstream sst;
    sst << bitset<32>(ori_instr.bi_instruction);
    string bi_str;
    sst >> bi_str;
    Instruction ins_test(bi_str, false);
    ins_test.toCompeleteType();
    ins_test.Execute(&reg_mem);

    // Print the register and memory values after the execution
    cout << reg_mem.toString() << endl;
}


void test_jal_instruction()
{
    // Create a new instance of RegAndMemory
    RegAndMemory reg_mem;
    reg_mem.pc = 0;

    // Create an ORI instruction and assemble it
    string ori_asm = "jal 40";
    Instruction ori_instr(ori_asm, true);
    Instruction::InitMap();

    // Convert the instruction to binary and print it
    ori_instr.toCompeleteType();
    cout << bitset<32>(ori_instr.bi_instruction) << endl;

    // Create a new Instruction object from the binary string and execute it
    stringstream sst;
    sst << bitset<32>(ori_instr.bi_instruction);
    string bi_str;
    sst >> bi_str;
    Instruction ins_test(bi_str, false);
    ins_test.toCompeleteType();
    ins_test.Execute(&reg_mem);

    // Print the register and memory values after the execution
    cout << reg_mem.toString() << endl;
    cout << "pc: " << reg_mem.pc << endl;
}


void test_beq_instruction()
{
    // Create a new instance of RegAndMemory
    RegAndMemory reg_mem;
    reg_mem.pc = 0;

    // Create an ORI instruction and assemble it
    string ori_asm = "beq $s0, $s0, 100";
    Instruction ori_instr(ori_asm, true);
    Instruction::InitMap();

    // Convert the instruction to binary and print it
    ori_instr.toCompeleteType();
    cout << bitset<32>(ori_instr.bi_instruction) << endl;

    // Create a new Instruction object from the binary string and execute it
    stringstream sst;
    sst << bitset<32>(ori_instr.bi_instruction);
    string bi_str;
    sst >> bi_str;
    Instruction ins_test(bi_str, false);
    ins_test.toCompeleteType();
    ins_test.Execute(&reg_mem);

    // Print the register and memory values after the execution
    cout << reg_mem.toString() << endl;
    cout << "pc: " << reg_mem.pc << endl;
}

void test_jr_instruction()
{
    // Create a new instance of RegAndMemory
    RegAndMemory reg_mem;
    reg_mem.pc = 0;
    reg_mem.reg[16] = 2;

    // Create an ORI instruction and assemble it
    string ori_asm = "jr $s0";
    Instruction ori_instr(ori_asm, true);
    Instruction::InitMap();

    // Convert the instruction to binary and print it
    ori_instr.toCompeleteType();
    cout << bitset<32>(ori_instr.bi_instruction) << endl;

    // Create a new Instruction object from the binary string and execute it
    stringstream sst;
    sst << bitset<32>(ori_instr.bi_instruction);
    string bi_str;
    sst >> bi_str;
    Instruction ins_test(bi_str, false);
    ins_test.toCompeleteType();
    ins_test.Execute(&reg_mem);

    // Print the register and memory values after the execution
    cout << reg_mem.toString() << endl;
    cout << "pc: " << reg_mem.pc << endl;
}

void running_test()
{
    RegAndMemory rm;
    rm.SetUint32(0x1234, 0);

    char buffer[100];

    itoa(rm.GetUint32(0), buffer, 16);

    cout << buffer << endl;
}


string StringCover(string str, int length, char cover = ' ')
{
    for(int i=0; i<(str.length()-length); i++)
    {
        str = cover + str;
    }

    return str;
}


// 00000000010000000001000
//        index2    index1
// get uint value between index2 and index1
// -> -0000000001

static int getUValueFromBits(uint32_t bi_code, int index2, int index1)
{
    int res = 0;

    for (int i = 0; i < 31 - index2; i++)
    {
        bi_code <<= 1;
    }

    for (int i = 0; i <= index2 - index1; i++)
    {
        res = res * 2 + (bi_code >= 2147483648);
        bi_code <<= 1;
    }

    return res;
}

// 00000000010000000001000
//        index2    index1
// get int value between index2 and index1
// -> 10000000001

static int64_t getValueFromBits(uint32_t bi_code, int index2, int index1)
{
    int64_t res = 0;
    int flag = 1;

    for (int i = 0; i < 31 - index2; i++)
    {
        bi_code <<= 1;
    }

    if (bi_code >= 2147483648)
    {
        flag = -1;
        bi_code <<= 1;
    }
    else 
    {
        bi_code <<= 1;
    }

    for (int i = 0; i < index2 - index1; i++)
    {
        res = res * 2;
        if (flag == 1 && bi_code >= 2147483648)
        {
            res += 1;
        }
        else if (flag == -1 && bi_code < 2147483648)
        {
            res += 1;
        }
        bi_code <<= 1;
    }
    if(flag == -1)
    {
        res += 1;
    }

    return res * flag;
}

class CodeType 
{
public:

    virtual CodeType* add(CodeType* another)
    {
        return nullptr;
    }

    virtual CodeType* sub(CodeType* another)
    {
        return nullptr;
    }

    virtual CodeType* mul(CodeType* another)
    {
        return nullptr;
    }

    virtual CodeType* div(CodeType* another)
    {
        return nullptr;
    }
    
    virtual string toString()
    {
        return "";
    }
};

class IntType : public CodeType
{
public:

    int64_t value;

    string res;

    static int length;

    IntType(int is_empty){}

    IntType(int64_t value, int length)
    {
        this->value = value;
        this->res = IntToBiString(value, length);
    }

    IntType(string res, int length)
    {
        this->res = StringCover(res, length, '0');
        this->value = BiStringToInt(res, length);
    }

    IntType()
    {
        cout << "Start your int" << endl;

        cout << "1. start with an int" << endl;
        cout << "2. start with a string" << endl;
        cout << " >> ";
        int type;
        cin >> type;

        if(type == 1)
        {
            cout << "value :" << endl;
            cout << " >> ";
            cin >> value;
            this->res = IntToBiString(value, length);
        }
        else 
        {
            cout << "string : " << endl;
            cout << " >> ";
            cin >> res;
            this->res = StringCover(res, length, '0');
            this->value = BiStringToInt(this->res, length);
        }
    }

    CodeType* add(CodeType* another)
    {
        CodeType* res_code = new IntType(1);
        ((IntType*)res_code)->value = this->value + ((IntType*)another)->value;
        ((IntType*)res_code)->res = IntToBiString(((IntType*)res_code)->value, length);

        return res_code;
    }

    CodeType* sub(CodeType* another)
    {
        CodeType* res_code = new IntType(1);
        ((IntType*)res_code)->value = this->value - ((IntType*)another)->value;
        ((IntType*)res_code)->res = IntToBiString(((IntType*)res_code)->value, length);

        return res_code;
    }

    CodeType* mul(CodeType* another)
    {
        CodeType* res_code = new IntType(1);
        ((IntType*)res_code)->value = this->value * ((IntType*)another)->value;
        ((IntType*)res_code)->res = IntToBiString(((IntType*)res_code)->value, 2*length);

        return res_code;
    }

    CodeType* div(CodeType* another)
    {
        CodeType* res_code = new IntType(1);
        ((IntType*)res_code)->value = this->value / ((IntType*)another)->value;
        ((IntType*)res_code)->res = IntToBiString(((IntType*)res_code)->value, length);

        return res_code;
    }

    static int error_val;

    // "1 0001 0010 1000" -> -3800
    static int64_t BiStringToInt(string str, int length = 32) 
    {
        IntType::error_val = 0;

        if(str.length() > length)
        {
            cout << "string length doesn't match" << endl;
            IntType::error_val = -1;

            return 0;
        }
        else 
        {
            str = StringCover(str, length);
        }

        int64_t value = 0;
        int flag = 1;

        if(str[0] == '1')
        {
            flag = -1;
        }

        for(int i=1; i<length; i++)
        {
            value <<= 1;
            if(flag == 1)
            {
                if(str[i] == '1')
                {
                    value += 1;
                }
            }
            else 
            {
                if(str[i] == '0')
                {
                    value += 1;
                }
            }
        }

        if(flag == -1)
        {
            value += 1;
        }

        return value * flag;
    }

    // 8 -> 00001000
    static string IntToBiString(int64_t value, int length = 32)
    {
        IntType::error_val = 0;
        int64_t max_value = pow(2, min(length-1, 62)) + 0.01;

        char adding[3] = "01";

        if(value < (-max_value) || (max_value-1) < value)
        {
            cout << "value overflow" << endl;
            cout << "range : [" << -max_value << "," << (max_value-1) << "]" << endl;
            cout << "income value: " << value << endl;
            cout << "income length: " << length << endl;
            
            IntType::error_val = -1;
            return "";
        }

        string res = "";
        int flag = 1;
        if(value < 0)
        {
            value = -value -1;
            res += "1";
            flag = 0;
        }
        else 
        {
            res += "0";
            flag = 1;
        }

        while(max_value)
        {
            max_value /= 2;
            if(max_value == 0)
            {
                break;
            }
            if(value >= max_value)
            {
                res += adding[flag];
                value %= max_value;
            }
            else 
            {
                res += adding[!flag];
            }
        }

        return res;
    }

    static string Adder(string value_1, string value_2, int length = 32, int is_print = 1)
    {
        string add_res = "";
        for(int i=0; i<length; i++)
        {
            add_res += "0";
        }
            
        if(is_print)
        {
            cout << "value_1 : " << value_1 << "  " << BiStringToInt(value_1, length) << endl;
            cout << "value_2 : " << value_2 << "  " << BiStringToInt(value_2, length) << endl;
        }

        int carry = 0;
        for(int i=31; i>=0; i--)
        {
            int res = carry + value_1[i] + value_2[i] - 2*'0';
            carry = res /2;
            res %= 2;
            add_res[i] = '0' + res;
            
            if(is_print)
            {
                cout << "step " << StringCover(to_string(32-i), 2) << " : " << add_res << " , carry = " << carry << endl;
            }
        }

        return add_res;
    }

    static string Suber(string value_1, string value_2, int length = 32, int is_print = 1)
    {
        if(is_print)
        {
            cout << "value_1 : " << value_1 << "  " << BiStringToInt(value_1, length) << endl;
            cout << "value_2 : " << value_2 << "  " << BiStringToInt(value_2, length) << endl;
        }
        value_2 = IntToBiString(-BiStringToInt(value_2, length), length);

        if(is_print)
            cout << "Two's complement of value_2: " << value_2 << endl;

        string sub_res = "";
        for(int i=0; i<length; i++)
        {
            sub_res += "0";
        }

        int carry = 0;
        for(int i=31; i>=0; i--)
        {
            int res = carry + value_1[i] + value_2[i] - 2*'0';
            carry = res /2;
            res %= 2;
            sub_res[i] = '0' + res;

            if(is_print)
                cout << "step " << StringCover(to_string(32-i), 2) << " : " << sub_res << " , carry = " << carry << endl;
        }

        return sub_res;
    }

    static string Muler(string value_1, string value_2, int length = 32)
    {
        int64_t v_1 = BiStringToInt(value_1, length);
        int64_t v_2 = BiStringToInt(value_2, length);
        int64_t res = v_1*v_2;

        return IntToBiString(res, 2*length);
    }

    static string Diver(string value_1, string value_2, int length = 32)
    {
        int64_t v_1 = BiStringToInt(value_1, length);
        int64_t v_2 = BiStringToInt(value_2, length);
        int64_t res = v_1/v_2;

        return IntToBiString(res, 2*length);
    }

    string toString()
    {
        stringstream sst;
        string str;
        sst << "(" << this->value << ";" << this->res << ")";
        sst >> str;
        return str;
    }
};

int IntType::error_val = 0;

int IntType::length = 32;

class FloatType : public CodeType 
{
public:

    float value;

    string res;

    FloatType(float value)
    {
        this->value = value;
        this->res = FloatToBiString(value);
    }

    FloatType(string str)
    {
        this->res = str;
        this->value = BiStringToFloat(str);
    }

    FloatType()
    {
        cout << "Start your float" << endl;

        cout << "1. start with an float" << endl;
        cout << "2. start with a string" << endl;
        cout << " >> ";
        int type;
        cin >> type;

        if(type == 1)
        {
            cout << "value :" << endl;
            cout << " >> ";
            cin >> value;
            this->res = FloatToBiString(value);
        }
        else 
        {
            cout << "string : " << endl;
            cout << " >> ";
            cin >> res;
            this->value = BiStringToFloat(this->res);
        }
    }

    static float BiStringToFloat(string str)
    {
        float value = 0;
        int flag = 1;
        int exponent = 0;
        uint32_t fraction = 0;
        int bias = 127;

        if(str[0] == '1')
        {
            flag = -1;
        }

        for(int i=1; i<9; i++)
        {
            exponent <<= 1;
            if(str[i] == '1')
                exponent += 1;
        }
        
        double t = 0.5;
        double x = 0;
        for(int i=9; i<32; i++)
        {
            if(str[i] == '1')
                x += t;
            t /= 2;
        }

        // cout << "flag: " << flag << endl;
        // cout << "fraction: " << x << endl;
        // cout << "exponent: " << exponent << endl;
        // cout << "d: " << exponent-bias << endl;
        // cout << "pow: " << pow(2, exponent-bias) << endl;

        value = flag*(1.0+x)*pow(2, exponent-bias);

        return value;        
    }

    static string FloatToBiString(float value)
    {
        string res;
        double v = pow(2, 126);
        int exponet = 1;
        int flag = 0;

        if(value < 0)
        {
            flag = 1;
            value = -value;
        }

        while(v > value)
        {
            v /= 2;
            exponet += 1;
        }
        
        string fraction = "";

        value -= v;

        for(int i=0; i<22; i++)
        {
            v /= 2;
            if(value > v)
            {
                fraction += "1";
                value -= v;
            }
            else 
            {
                fraction += "0";
            }
        }

        // cout << "value : " << value << endl;
        // cout << v << endl;

        double d_value = value;
        // 1--0
        double b1 = v;
        double d1 = v - d_value;
        // 0--1
        double b2 = v / 2;
        double d2 = d_value - b2;
        if(value > v)
        {
            fraction += '1';
        }
        else if(value < b2)
        {
            fraction += '0';
        }
        else if(d1 < d2)
        {
            fraction += '1';
        }
        else if(d2 < d1)
        {
            fraction += '0';
        }


        // cout << "str exp" << (254-exponet) << endl;
        exponet = 254-exponet;

        stringstream sst;
        sst << bitset<8>(exponet);
        string str_exponet;
        sst >> str_exponet;

        res += '0' + flag;
        res += str_exponet;
        res += fraction;

        return res;
    }

    virtual CodeType* add(CodeType* another)
    {
        CodeType* new_code = new FloatType(1);
        ((FloatType*)new_code)->value = this->value + ((FloatType*)another)->value;
        ((FloatType*)new_code)->res = FloatToBiString(((FloatType*)new_code)->value);
        return new_code;
    }

    virtual CodeType* sub(CodeType* another)
    {
        CodeType* new_code = new FloatType(1);
        ((FloatType*)new_code)->value = this->value - ((FloatType*)another)->value;
        ((FloatType*)new_code)->res = FloatToBiString(((FloatType*)new_code)->value);
        return new_code;
    }

    virtual CodeType* mul(CodeType* another)
    {
        CodeType* new_code = new FloatType(1);
        ((FloatType*)new_code)->value = this->value * ((FloatType*)another)->value;
        ((FloatType*)new_code)->res = FloatToBiString(((FloatType*)new_code)->value);
        return new_code;
    }

    virtual CodeType* div(CodeType* another)
    {
        CodeType* new_code = new FloatType(1);
        ((FloatType*)new_code)->value = this->value / ((FloatType*)another)->value;
        ((FloatType*)new_code)->res = FloatToBiString(((FloatType*)new_code)->value);
        return new_code;
    }
    
    virtual string toString()
    {
        stringstream sst;
        string res;
        sst << "(" << this->value << ";" << this->res << ")";
        sst >> res;
        return res;
    }
};


class ConvertInterface 
{
public:

    ConvertInterface(){}

    void run()
    {
        cout << "1. int" << endl;
        cout << "2. float" << endl;
        int type;
        cin >> type;

        system("cls");
        CodeType* code_1;
        CodeType* code_2;

        if(type == 1)
        {
            cout << "length : " << endl;
            cout << " >> " ;
            cin >> IntType::length;
            cout << "fisrt : " << endl;
            code_1 = new IntType();
            cout << "second : " << endl;
            code_2 = new IntType();
        }
        else 
        {
            cout << "first : " << endl;
            code_1 = new FloatType();
            cout << "second : " << endl;
            code_2 = new FloatType();
        }

        system("cls");
        cout << "first is : " << code_1->toString() << endl;
        cout << "second is : " << code_2->toString() << endl;
        cout << code_1->toString() << " + " << code_2->toString() << " = " << endl;
        cout << "       " << code_1->add(code_2)->toString() << endl;
        cout << code_1->toString() << " - " << code_2->toString() << " = " << endl;
        cout << "       " << code_1->sub(code_2)->toString() << endl;
        cout << code_1->toString() << " * " << code_2->toString() << " = " << endl;
        cout << "       " << code_1->mul(code_2)->toString() << endl;
        cout << code_1->toString() << " / " << code_2->toString() << " = " << endl;
        cout << "       " << code_1->div(code_2)->toString() << endl;

        cout << "press enter to continue" << endl;
        getchar();
        getchar();
    }

};


int old_test()
{
    string str = "1000100101000";
    // string str = "00001000";

    cout << str << endl;

    cout << IntType::BiStringToInt(str, str.length()) << endl;

    char buffer[100];

    cout << stoi(str.c_str(), 0, 2) << endl; // 该函数负数会有bug，最多支持int32内的正数

    cout << IntType::IntToBiString(IntType::BiStringToInt(str, str.length()), str.length()) << endl;

    int v_1 = 77;
    int v_2 = 100;

    cout << v_1 << "+" << v_2 << endl;

    cout << IntType::IntToBiString(v_1 + v_2, 32) << endl;

    string added_res = IntType::Suber(IntType::IntToBiString(v_1), IntType::IntToBiString(v_2), 32);
    cout << added_res << endl;
    cout << IntType::BiStringToInt(added_res) << endl;

    return 0;
}

int IntTypeTest()
{
    CodeType* code_1 = new IntType("11111111111111101010000001110001", 32);
    CodeType* code_2 = new IntType(-999, 32);
    cout << "first is : " << code_1->toString() << endl;
    cout << "second is : " << code_2->toString() << endl;
    cout << code_1->toString() << " + " << code_2->toString() << " = " << endl;
    cout << "       " << code_1->add(code_2)->toString() << endl;
    cout << code_1->toString() << " - " << code_2->toString() << " = " << endl;
    cout << "       " << code_1->sub(code_2)->toString() << endl;
    cout << code_1->toString() << " * " << code_2->toString() << " = " << endl;
    cout << "       " << code_1->mul(code_2)->toString() << endl;
    cout << code_1->toString() << " / " << code_2->toString() << " = " << endl;
    cout << "       " << code_1->div(code_2)->toString() << endl;

    return 0;
}

int FloatTypeTest()
{
    // float value = 9.15124;
    // string str = FloatType::FloatToBiString(value);

    // cout << value << endl;

    // cout << str << endl;

    // cout << FloatType::BiStringToFloat("01000001000100100110101101111010") << endl;

    // CodeType* code_1 = new FloatType("11111111011111101010000001110001");
    CodeType* code_1 = new FloatType("01000001000100100110101101111011");
    CodeType* code_2 = new FloatType(-999);
    cout << "first is : " << code_1->toString() << endl;
    cout << "second is : " << code_2->toString() << endl;
    cout << code_1->toString() << " + " << code_2->toString() << " = " << endl;
    cout << "       " << code_1->add(code_2)->toString() << endl;
    cout << code_1->toString() << " - " << code_2->toString() << " = " << endl;
    cout << "       " << code_1->sub(code_2)->toString() << endl;
    cout << code_1->toString() << " * " << code_2->toString() << " = " << endl;
    cout << "       " << code_1->mul(code_2)->toString() << endl;
    cout << code_1->toString() << " / " << code_2->toString() << " = " << endl;
    cout << "       " << code_1->div(code_2)->toString() << endl;

    return 1;
}

class UserInterface 
{
public:

    int interface_state = 0;

    enum 
    {
        NULLSTATE,
        STATE_ONE,
        STATE_TWO,
        STATE_THREE,
        STATE_FOUR
    };

    void printUI()
    {
        system("cls");
        int n;
        cout << "1. asm -> bi" << endl;
        cout << "2. bi -> asm" << endl;
        cout << "3. load code and run" << endl;
        cout << "4. convert with int or float" << endl;
        cout << "type number to execute" << endl;
        cout << " >> " ;
        try
        {
            cin >> n;
        }
        catch(const std::exception& e)
        {
            string s;
            cin >> s;
        }

        this->interface_state = n;
        system("cls");
        return;
    }

    void AsmConvert()
    {
        system("cls");
        int n;
        cout << "number of lines: " << endl;
        cout << " >> ";
        cin >> n;
        cout << n << " lines of asm code :" << endl; 

        vector<string> bibuffer;

        for(int i=0; i<n; i++)
        {
            string line;
            getline(cin, line);
            Instruction ins(line, true);
            ins.toCompeleteType();
            if(ins.instruction_type == Instruction::NULLTYPE)
            {
                i--;
                continue;
            }
            stringstream st;
            st << bitset<32>(ins.bi_instruction);
            st >> line;
            bibuffer.push_back(line);
        }

        for(string code : bibuffer)
        {
            cout << code << endl;
        }

        cout << "print enter to continue...";
        getchar();
        this->interface_state = 0;
        system("cls");
    }

    void BiConvert()
    {
        system("cls");
        int n;
        cout << "number of lines: " << endl;
        cout << " >> ";
        cin >> n;

        cout << n << " lines of bi code" << endl;

        vector<string> asmbuffer;

        for(int i=0; i<n; i++)
        {
            string line;
            getline(cin, line);
            Instruction ins(line, false);
            ins.toCompeleteType();
            if(ins.instruction_type == Instruction::NULLTYPE)
            {
                i--;
                continue;
            }

            asmbuffer.push_back(ins.toString());
        }

        for(string code : asmbuffer)
        {
            cout << code << endl;
        }

        cout << "print enter to continue...";
        getchar();
        this->interface_state = 0;
        system("cls");
    }

    vector<uint32_t> GetCode(int lines)
    {
        vector<uint32_t> res;

        for(int i=0; i<lines; i++)
        {
            string str;
            cin >> str;
            uint32_t value;
            value = stoi(str.c_str(), 0, 2);
            res.push_back(value);
        }

        return res;
    }

    void VmExecute()
    {
        int n;
        cout << "lines of the code:" << endl;
        cout << " >> " ;
        cin >> n;
        cout << n << " lines of bi code" << endl;

        RegAndMemory rm;
        vector<uint32_t> code = GetCode(n);
        rm.LoadCode(code);

        while(1)
        {
            system("cls");
            cout << rm.toString() << endl;
            cout << "press enter to run to next step" << endl;
            getchar();

            int res = rm.Execute();
            if(res == 0)
            {
                break;
            }
        }

        system("cls");
        cout << rm.toString() << endl;
        cout << "code run to the end, press enter to continue..." << endl;
        getchar();

        this->interface_state = 0;
    }

    void run()
    {
        ConvertInterface ci;
        while(1)
        {
            switch (this->interface_state)
            {
                case UserInterface::NULLSTATE:
                    this->printUI();
                    break;

                case UserInterface::STATE_ONE:
                    this->AsmConvert();
                    break;

                case UserInterface::STATE_TWO:
                    this->BiConvert();
                    break;

                case UserInterface::STATE_THREE:
                    this->VmExecute();
                    break;

                case UserInterface::STATE_FOUR:
                    ci.run();
                    this->interface_state = 0;
                    break;

                default:
                    break;
            }
        }
    }

};


int main()
{
    Instruction::InitMap();
    // test_andi_instruction();
    // test_ori_instruction();
    //test_xori_instruction();1
    // test_jal_instruction();
    // test_beq_instruction();
    // test_jr_instruction();
    // running_test();
    // AsmConvert();
    UserInterface ui;
    ui.run();

    // uint32_t value = 0b11101;

    // cout << Instruction::getValueFromBits(value, 4, 0) << endl;

    // RegAndMemory rm;
    // cout << rm.toString() << endl;

    return 0;
}
