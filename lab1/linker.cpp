#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>

using namespace std;

const int MAX_INT = 1 << 30;
const string NULL_STR = "NULL";
string filename;
int line = 1, offset = 1, line_num, line_offset, last_line, last_offset;
ifstream file;

char getChar();
string getToken();
int readInt();
string readSymbol();
char readMARIE();

void pass1();
void pass2();

void __parseerror(int errcode);
void printSymbolTable();
void printMemoryMap(int instr_num, int opcode, int operand, int errcode, string sym);

void rule2(string sym);
bool rule3(string sym);
void rule4();
void rule5(string sym, int &relative_address, int num);
bool rule6(int symbol_num);
void rule7(int module_num);
bool rule8(int absolute_add);
bool rule9(int relative_add, int module_num);
bool rule10(int immediate);
bool rule11(int opcode);
bool rule12(int module_num);

void testToken();
bool isnumber(char ch);
bool isalnum(char ch);
bool isalpha(char ch);
long long strToLong(string st);

class SymbolSet {
private:
    vector<string> name, uselist;
    vector<int> relative_address, absolute_address, module_num, used_by_external;
    map<string, int> index;
    set<string> duplicated_symbol, used;
public:
    SymbolSet() {
        name = vector<string>(0);
        uselist = vector<string>(0);
        relative_address = vector<int>(0);
        absolute_address = vector<int>(0);
        module_num = vector<int>(0);
        index = map<string, int>();
        duplicated_symbol = set<string>();
        used = set<string>();
        used_by_external = vector<int>(0);
    }
    void createSymbol(string sym, int r_address, int abs_address, int num) {
        index[sym] = name.size();
        name.push_back(sym);
        relative_address.push_back(r_address);
        absolute_address.push_back(abs_address);
        module_num.push_back(num);
    }
    void resetUselist() {
        uselist = vector<string>(0);
        used_by_external = vector<int>(0);
    }
    int symbolCount() {return name.size();}
    int uselistSize() {return uselist.size();}
    int absoluteAddress(string sym) {return absolute_address[index[sym]];}
    int getModuleNum(string sym) {return module_num[index[sym]];}
    string symbolName(int num) {return name[num];}
    string symbolNameInUselist(int num) {return uselist[num];}
    bool isDefined(string sym) {return index.count(sym) > 0;}
    bool isDuplicated(string sym) {return duplicated_symbol.count(sym) > 0;}
    bool isUsed(string sym) {return used.count(sym) > 0;}
    bool isUsedByExternal(int index) {return used_by_external[index] > 0;}
    void addDuplicate(string sym) {duplicated_symbol.insert(sym);}
    void addUselist(string sym) {
        uselist.push_back(sym);
        used_by_external.push_back(0);
    }
    void setUsed(string sym) {used.insert(sym);}
    void setUsedByExternal(int index) {used_by_external[index] = 1;}
};

class ModuleSet {
private:
    vector<int> base_address;
    vector<int> size;
public:
    ModuleSet() {
        base_address = vector<int>(0);
        size = vector<int>(0);
    }
    void createModule(int length) {
        if (size.empty()) {
            base_address.push_back(0);
            size.push_back(length);
        }
        else {
            base_address.push_back(base_address[size.size() - 1] + size[size.size()-1]);
            size.push_back(length);
        }
    }
    int moduleCount() {return size.size();}
    int moduleSize(int num) {return size[num];}
    int moduleAddress(int num) {return base_address[num];}
};

ModuleSet Module;
SymbolSet Symbol;

char getChar() { // get a char from input file stream
    char ch;
    file.get(ch);
    if (isalnum(ch) || ch == ' ' || ch == '\t')
        offset  = offset + 1;
    else if (!file.eof() && ch == '\n') {
        last_line = line;
        last_offset = offset;
        line = line + 1;
        offset = 1;
    }
    return ch;
}

string getToken() { // get alphabets and numbers to form a token string
    char ch;
    string token;
    while (! file.eof()) {
        ch = getChar();
        if (isalnum(ch)) {
            if (token.empty()) {
                line_num = line;
                line_offset = offset-1;
            }
            token.push_back(ch);
        }
        else if (! token.empty())
            return token;
    }
    if (token.empty()) {
        line_num = line;
        line_offset = offset;
        return NULL_STR;
    }
    return NULL_STR;
}

int readInt() {
    string tok = getToken();
    if (tok == NULL_STR)
        return -1;
    for (int i=0; i<tok.length(); i++)
        if (! isnumber(tok[i]))
            __parseerror(0);
    long long num = strToLong(tok);
    if (num >= MAX_INT)
        __parseerror(0);
    return int(num);
}

string readSymbol() {
    string tok = getToken();
    if (tok == NULL_STR)
        __parseerror(1);
    if (! isalpha(tok[0]))
        __parseerror(1);
    for (int i=1; i<tok.length(); i++)
        if (! isalnum(tok[i]))
            __parseerror(1);
    if (tok.length() > 16)
        __parseerror(3);
    return tok;
}

char readMARIE() {
    string tok = getToken();
    if (tok == NULL_STR)
        __parseerror(2);
    if (tok.length() > 1)
        __parseerror(2);
    char ch = tok[0];
    if (ch != 'M' && ch != 'A' && ch != 'R' && ch != 'I' && ch != 'E')
        __parseerror(2);
    return ch;
}

void pass1() {
    int total_instcount = 0;
    vector<pair<string, int> > def_symbol_list;
    while (true) {
        def_symbol_list = vector<pair<string, int> >(0);

        int defcount = readInt();
        if (defcount < 0)
            break;
        if (defcount > 16)
            __parseerror(4);
        for (int i=0; i<defcount; i++) {
            string sym = readSymbol();
            int relative_address = readInt();
            def_symbol_list.push_back(make_pair(sym, relative_address));
        }

        int usecount = readInt();
        if (usecount > 16)
            __parseerror(5);
        for (int i=0; i<usecount; i++) {
            string sym = readSymbol();
        }

        int instcount = readInt();
        total_instcount = total_instcount + instcount;
        if (total_instcount > 512)
            __parseerror(6);
        for (int i=0; i<instcount; i++) {
            char address_mode = readMARIE();
            int operand = readInt();
        }

        Module.createModule(instcount);
        for (int i=0; i<def_symbol_list.size(); i++) {
            string sym = def_symbol_list[i].first;
            int relative_address = def_symbol_list[i].second;
            int num = Module.moduleCount() - 1;
            rule5(sym, relative_address, num);
            if (! Symbol.isDefined(sym)) {
                int module_base_address = Module.moduleAddress(num);
                int abs_address = module_base_address + relative_address;
                Symbol.createSymbol(sym, relative_address, abs_address, num);
            }
        }
    }
    printSymbolTable();
}


void pass2() {
    printf("Memory Map\n");
    int module_num = 0, instr_num = 0;
    while (true) {
        int defcount = readInt();
        if (defcount < 0)
            break;
        for (int i=0; i<defcount; i++) {
            string sym = readSymbol();
            int relative_address = readInt();
        }

        int usecount = readInt();
        Symbol.resetUselist();
        for (int i=0; i<usecount; i++) {
            string sym = readSymbol();
            Symbol.addUselist(sym);
        }

        int instcount = readInt();
        for (int i=0; i<instcount; i++) {
            char addrmode = readMARIE();
            int instr = readInt();
            int opcode = instr / 1000, operand = instr % 1000;
            int errcode = 0;
            string errorsym = "";

            if (rule11(opcode)) {
                opcode = 9, operand = 999;
                errcode = 11;
            }
            else if (addrmode == 'M') {
                int target_num = operand;
                if (rule12(target_num)) {
                    target_num = 0;
                    errcode = 12;
                }
                operand = Module.moduleAddress(target_num);
            }
            else if (addrmode == 'R') {
                int relative_add = operand;
                if (rule9(relative_add, module_num)) {
                    relative_add = 0;
                    errcode = 9;
                }
                operand = Module.moduleAddress(module_num) + relative_add;
            }
            else if (addrmode == 'I') {
                int immediate = operand;
                if (rule10(immediate)) {
                    immediate = 999;
                    errcode = 10;
                }
                operand = immediate;
            }
            else if (addrmode == 'A') {
                int absolute_add = operand;
                if (rule8(absolute_add)) {
                    absolute_add = 0;
                    errcode = 8;
                }
                operand = absolute_add;
            }
            else if (addrmode == 'E') {
                int symbol_num = operand;
                if (rule6(symbol_num)) {
                    operand = Module.moduleAddress(module_num);
                    errcode = 6;
                }
                else {
                    string sym = Symbol.symbolNameInUselist(symbol_num);
                    Symbol.setUsed(sym);
                    Symbol.setUsedByExternal(symbol_num);
                    if (rule3(sym)) {
                        operand = 0;
                        errcode = 3;
                        errorsym = sym;
                    }
                    else {
                        operand = Symbol.absoluteAddress(sym);
                    }
                }
            }
            printMemoryMap(instr_num, opcode, operand, errcode, errorsym);
            instr_num = instr_num + 1;
        }
        rule7(module_num);
        module_num = module_num + 1;
    }
    printf("\n");
    rule4();
}

void __parseerror(int errcode) {
    static string errstr[] = {
            "NUM_EXPECTED", // Number expect, anything >= 2^30 is not a number either
            "SYM_EXPECTED", // Symbol Expected
            "MARIE_EXPECTED", // Addressing Expected which is M/A/R/I/E
            "SYM_TOO_LONG", // Symbol Name is too long
            "TOO_MANY_DEF_IN_MODULE", // > 16
            "TOO_MANY_USE_IN_MODULE", // > 16
            "TOO_MANY_INSTR", // total num_instr exceeds memory size (512)
    };
    if (file.eof() && line_offset == 1)
        printf("Parse Error line %d offset %d: %s\n", last_line, last_offset, errstr[errcode].c_str());
    else
        printf("Parse Error line %d offset %d: %s\n", line_num, line_offset, errstr[errcode].c_str());
    exit(1);
}

void printSymbolTable() {
    printf("Symbol Table\n");
    int total = Symbol.symbolCount();
    for (int i=0; i<total; i++) {
        string sym = Symbol.symbolName(i);
        int abs_address = Symbol.absoluteAddress(sym);
        printf("%s=%d", sym.c_str(), abs_address);
        rule2(sym);
        printf("\n");
    }
    printf("\n");
}

void printMemoryMap(int instr_num, int opcode, int operand, int errcode, string sym) {
    int instr = opcode*1000 + operand;
    printf("%03d: %04d", instr_num, instr);
    switch (errcode) {
        case 0: printf("\n"); break;
        case 3: printf(" Error: %s is not defined; zero used\n", sym.c_str()); break;
        case 6: printf(" Error: External operand exceeds length of uselist; treated as relative=0\n"); break;
        case 8: printf(" Error: Absolute address exceeds machine size; zero used\n"); break;
        case 9: printf(" Error: Relative address exceeds module size; relative zero used\n"); break;
        case 10: printf(" Error: Illegal immediate operand; treated as 999\n"); break;
        case 11: printf(" Error: Illegal opcode; treated as 9999\n"); break;
        case 12: printf(" Error: Illegal module operand ; treated as module=0\n"); break;
        default: break;
    }
}

void rule2(string sym) {
    if (Symbol.isDuplicated(sym))
        printf(" Error: This variable is multiple times defined; first value used");
}

bool rule3(string sym) {
    return (! Symbol.isDefined(sym));
}

void rule4() {
    int symbol_size = Symbol.symbolCount();
    for (int i=0; i<symbol_size; i++) {
        string sym = Symbol.symbolName(i);
        if (! Symbol.isUsed(sym)) {
            int module_num = Symbol.getModuleNum(sym);
            printf("Warning: Module %d: %s was defined but never used\n", module_num, sym.c_str());
        }
    }
}

void rule5(string sym, int &relative_address, int num) {
    int module_size = Module.moduleSize(num);
    if (!Symbol.isDefined(sym)) {
        if (relative_address >= module_size) {
            printf("Warning: Module %d: %s=%d valid=[0..%d] assume zero relative\n", num, sym.c_str(), relative_address, module_size-1);
            relative_address = 0;
        }
    }
    else {
        Symbol.addDuplicate(sym);
        printf("Warning: Module %d: %s redefinition ignored\n", num, sym.c_str());
    }
}

bool rule6(int symbol_num) {
    return symbol_num >= Symbol.uselistSize();
}

void rule7(int module_num) {
    int uselist_size = Symbol.uselistSize();
    for (int i=0; i<uselist_size; i++) {
        string sym = Symbol.symbolNameInUselist(i);
        if (! Symbol.isUsedByExternal(i))
            printf("Warning: Module %d: uselist[%d]=%s was not used\n", module_num, i, sym.c_str());
    }
}

bool rule8(int absolute_add) {
    return absolute_add >= 512;
}

bool rule9(int relative_add, int module_num) {
    return relative_add >= Module.moduleSize(module_num);
}

bool rule10(int immediate) {
    return immediate >= 900;
}

bool rule11(int opcode) {
    return opcode >= 10;
}

bool rule12(int module_num) {
    return module_num >= Module.moduleCount();
}

void testToken() {
    string tok;
    while ((tok = getToken()) != NULL_STR) {
        printf("token=<%s> position=%d:%d\n", tok.c_str(), line_num, line_offset);
    }
    printf("EOF position %d:%d\n", line_num, line_offset);
}

bool isnumber(char ch) {
    return (ch >= '0' && ch <= '9');
}

bool isalpha(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool isalnum(char ch) {
    return isnumber(ch) || isalpha(ch);
}

long long strToLong(string s){
    long long val = 0;
    for (int i=0; i<s.length(); i++)
        val = val * 10 + (long long)(s[i] - '0');
    return val;
}

int main(int argc, char* argv[]) {
    if (argc !=2 || argv[1] == NULL) {
        printf("Please enter the file name\n");
        return 0;
    }
    filename = argv[1];
//    filename = "../sample/input-9";
    Module = ModuleSet();
    Symbol = SymbolSet();

    file.open(filename.c_str());
    if (file.is_open()) pass1();
    else printf("Fail to open file\n");
    file.close();

    file.open(filename.c_str());
    if (file.is_open()) pass2();
    else printf("Fail to open file\n");
    file.close();

    return 0;
}