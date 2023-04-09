
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

class RegAndMemory
{

public:
    uint32_t reg[REG_NUM];

    char memo[MEMO_SIZE];

    uint32_t pc;

    RegAndMemory()
    {
        for (int i = 0; i < REG_NUM; i++)
        {
            reg[i] = 0;
        }
        memset(memo, 0, MEMO_SIZE);
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
            res += "\n";
        }

        int counter = 0;
        for (int i = 0; i < MEMO_SIZE; i++)
        {
            itoa(memo[i], temps, 16);
            res += temps;
            res += "   ";
            counter++;
            if (counter == 10)
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

    static int getValueFromBits(uint32_t bi_code, int index2, int index1)
    {
        int res = 0;
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

        for (int i = 0; i <= index2 - index1; i++)
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

        map_of_asm["add"] = 0b000000;
        map_of_asm["addi"] = 0b001000;
        map_of_asm["andi"] = 0b001100;
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
        switch (map_of_asm[this->splt_instruction[0]])
        {
        case 0b000000: // add

            if (this->splt_instruction[0] == "jr") {
                AsmTypeConvert_jr();
            }
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
            break;
        }
    }

    void toCompeleteType()
    {
        switch (this->instruction_type)
        {
        case Instruction::InstructionType::NULLTYPE:
            cout << "NULLTYPE fail to complete" << endl;
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
        for (string stp : splt_instruction)
        {
            res += stp + " ";
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

map<string, int> Instruction::map_of_reg_num = map<string, int>();

map<int, string> Instruction::map_of_reg_str = map<int, string>();

map<string, uint32_t> Instruction::map_of_asm = map<string, uint32_t>();

map<uint32_t, string> Instruction::map_of_opcode = map<uint32_t, string>();

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
    string ori_asm = "beq $s0, $s0, 8";
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

int main()
{
    // test_andi_instruction();
    // test_ori_instruction();
    //test_xori_instruction();
    //test_jal_instruction();
    //test_beq_instruction();
    test_jr_instruction();
    return 0;
}