#include <iostream>
#include <map>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <bitset>
#include <sstream>
#include <math.h>

using namespace std;

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

int main()
{
    ConvertInterface ci;
    ci.run();
    // IntTypeTest();
    // FloatTypeTest();

    return 0;
}
