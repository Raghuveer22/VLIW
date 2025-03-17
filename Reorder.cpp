#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <numeric>

using namespace std;

vector<string> split(const string& str, char delim) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

vector<vector<string> > Units = {
    {"ADD", "ADDC", "SUB", "SUBB"},//ADDER
    {"MUL"},//MULTIPLIER
    {"FADD"},//FPA
    {"FMUL"},//FPM
    {"AND", "OR", "NAND", "NOR", "XOR", "XNOR", "NOT", "BLS"}, //LOGICUNIT
    {"MOV", "LDR", "STR"} //MEM
};

vector<vector<string> > UnitsOpc = {
    {"0000", "0100", "1100", "1000"},
    {"0000"},
    {"0000"},
    {"0000"},
    {"0000", "0110", "0100", "1010", "0010", "1110", "1000", "1100"},
    {"01", "10", "11"}
};

map<string, string> Opc;

map<string, int> Delay;

class Node {
    public:
        string Reg;
        string Instruction;
        string InstructionAsWord;
        int Delay;  
        vector<string> Parents;
        vector<Node*> Children;
        vector<Node*> WAWdeps;
        vector<Node*> WARdeps;
        bool exc;
    
        Node(string reg, string ins) : Reg(reg), InstructionAsWord(ins), Delay(0), exc(false) {}
    
        void add_child(Node* child) { Children.push_back(child); }
        void add_parents(vector<string> parents) { Parents = parents; }
        void add_WAW(Node* dep) { WAWdeps.push_back(dep); }
        void add_WAR(Node* dep) { WARdeps.push_back(dep); }
        void add_ins(string ins) { 
            Instruction = ins; 
            Delay = ::Delay[ins];  
        }
        void printDependencies() {
            cout << "Reg Name: " << this->Reg << "\n";
            cout<<"Instruction:" << this->Instruction<<endl;
        
            cout << "WAR Dependencies:\n";
            for (auto* dep : this->WARdeps) {
                cout << "  -> " << dep->Reg << " (" << dep->Instruction << ")\n";
            }
        
            cout << "WAW Dependencies:\n";
            for (auto* dep : this->WAWdeps) {
                cout << "  -> " << dep->Reg << " (" << dep->Instruction << ")\n";
            }
        
            cout << "Children:\n";
            for (auto* child : this->Children) {
                cout << "  -> " << child->Reg << " (" << child->Instruction << ")\n";
            }
        }
};

vector<string> getUnit(string ins) {
    for (auto& unit : Units) {
        if (find(unit.begin(), unit.end(), ins) != unit.end()) {
            return unit;
        }
    }
    return vector<string>();
}

string toBin(int reg, int numDig = 5) {
    string binNum;
    while (reg > 0) {
        binNum = binNum + to_string(reg % 2);
        reg /= 2;
    }
    binNum = binNum + string(numDig - binNum.length(), '0');
    reverse(binNum.begin(), binNum.end());
    return binNum;
}

int main() {
    // Initialize opcode and delay maps
    vector<string> Un, UnOp;
    for (auto& unit : Units) Un.insert(Un.end(), unit.begin(), unit.end());
    for (auto& opc : UnitsOpc) UnOp.insert(UnOp.end(), opc.begin(), opc.end());
    for (size_t i = 0; i < Un.size(); i++) Opc[Un[i]] = UnOp[i];
    
    vector<string> ops = {"ADD", "ADDC", "SUB", "SUBB", "MUL", "FADD", "FMUL", 
                         "AND", "OR", "NAND", "NOR", "XOR", "XNOR", "NOT", "BLS", 
                         "MOV", "LDR", "STR"};
    vector<int> delays = {5, 5, 5, 5, 14, 5, 26, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    for (size_t i = 0; i < ops.size(); i++) Delay[ops[i]] = delays[i];

    // Read input file
    ifstream f("test.vsm");
    string asmContent((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    f.close();
    vector<string> Instructions = split(asmContent, '\n');

    // Parse instructions
    vector<string> Ins;
    vector<vector<string> > regs;
    for (auto& i : Instructions) {
        vector<string> parts = split(i, ' ');
        Ins.push_back(parts[0]);
        vector<string> regParts = split(parts[1], ',');
        if (find(Units[5].begin(), Units[5].end(), parts[0]) == Units[5].end()) {
            regs.push_back(regParts);
        } else {
            if (parts[1].find('#') != string::npos) {
                regParts.pop_back();
                regParts.push_back("R0");
            }
            regs.push_back(regParts);
        }
    }
   
    // Create node tables
    vector<Node> BegNodeTable;
    for (int i = 0; i < 32; i++) BegNodeTable.push_back(Node("R" + to_string(i), ""));
    // GlobalNodeTable stores the references for BegNode Table
    // clearing the begNodeTable affect the GlobalNodeTable but not vice versa
    vector<Node*> GlobalNodeTable;
    for (auto& node : BegNodeTable) GlobalNodeTable.push_back(&node);

    // Build dependency graph
    for (size_t i = 0; i < Ins.size(); i++) {
        vector<string> rparents = (Ins[i] != "MUL") ? 
            vector<string>(regs[i].begin() + 1, regs[i].end()) : 
            vector<string>(regs[i].begin() + 2, regs[i].end());
        string rname = regs[i][0];
       
        Node* N = new Node(rname, Instructions[i]);
        N->add_ins(Ins[i]);
        N->add_parents(rparents);

        for (auto& j : rparents) {
            GlobalNodeTable[stoi(j.substr(1))]->add_child(N);
        }


        // write after writing the previous value
        N->add_WAW(GlobalNodeTable[stoi(rname.substr(1))]);
        
        // in mul you will write in 2 
        if (Ins[i] == "MUL") N->add_WAW(GlobalNodeTable[stoi(regs[i][1].substr(1))]);
       
        auto& children = GlobalNodeTable[stoi(rname.substr(1))]->Children;

        for (auto* child : children) N->add_WAR(child);

        

        if (Ins[i] == "MUL") {
            auto& mulChildren = GlobalNodeTable[stoi(regs[i][1].substr(1))]->Children;
            for (auto* child : mulChildren) N->add_WAR(child);
        }
        
        while (find(N->WARdeps.begin(), N->WARdeps.end(), N) != N->WARdeps.end()) {
            N->WARdeps.erase(remove(N->WARdeps.begin(), N->WARdeps.end(), N), N->WARdeps.end());
        }

        GlobalNodeTable[stoi(rname.substr(1))] = N;
        if (Ins[i] == "MUL") 
        {
            GlobalNodeTable[stoi(regs[i][1].substr(1))] = N;
        }
    }
    
    // Scheduling
    vector<Node*> NextLv;
    vector<string> Packet;
    vector<vector<string> > PacketedIns;
    while (!BegNodeTable.empty()) {
        Node leader = BegNodeTable[0];
        BegNodeTable.erase(BegNodeTable.begin());
       
        bool UnitFree = true;
        for (auto& i : Packet) {
            if (getUnit(split(i, ' ')[0]) == getUnit(leader.Instruction)) {
                UnitFree = false;
                break;
            }
        }
       

        bool WAWCleared = leader.WAWdeps.empty() || all_of(
            leader.WAWdeps.begin(), 
            leader.WAWdeps.end(), 
            [](Node* n) { return n != nullptr && n->Delay == 0; }
        );
        bool WARCleared = leader.WARdeps.empty() || all_of(
            leader.WARdeps.begin(), 
            leader.WARdeps.end(), 
            [](Node* n) { return n != nullptr && n->exc; }
        );
        cout<<UnitFree<<" "<<WARCleared<<" "<<WAWCleared<<" "<<"hi"<<leader.Reg<<" "<<leader.Instruction<<" "<<leader.InstructionAsWord<<endl;
               
        if (leader.Parents.empty() && !leader.Instruction.empty() && !leader.exc && 
            WAWCleared && WARCleared && UnitFree) {
            leader.exc = true;
            cout << leader.InstructionAsWord << "(" << leader.Delay << ")\t";
            Packet.push_back(leader.InstructionAsWord);
        }

        if (leader.Parents.empty() && (!WAWCleared || !WARCleared || !UnitFree)) {
            NextLv.push_back(&leader);
        }
        else if (leader.Parents.empty() && leader.Delay != 0) {
            leader.Delay--;
            if (leader.Delay > 0) NextLv.push_back(&leader);
        }

        if (leader.Delay == 0) {
            for (auto* child : leader.Children) {
                NextLv.push_back(child);
                child->Parents.erase(
                    remove(child->Parents.begin(), child->Parents.end(), leader.Reg), 
                    child->Parents.end()
                );
            }
        }

        if (BegNodeTable.empty()) {
            set<Node*> uniqueNextLv(NextLv.begin(), NextLv.end());
            BegNodeTable.clear();  
            for (Node* node : uniqueNextLv) {
                BegNodeTable.push_back(*node); 
            }
            NextLv.clear();
            PacketedIns.push_back(Packet);
            Packet.clear();
            cout << endl;
        }
    }

    PacketedIns.erase(PacketedIns.begin()); // Remove first NOP packet

    // Reorder instructions
    vector<vector<string> > RepackedIns;
    for (auto& packet : PacketedIns) {
        vector<string> repack(6, "NOP");
        for (auto& ins : packet) {
            for (int k = 0; k < 6; k++) {
                if (find(Units[k].begin(), Units[k].end(), split(ins, ' ')[0]) != Units[k].end()) {
                    repack[k] = ins;
                }
            }
        }
        reverse(repack.begin(), repack.end());
        RepackedIns.push_back(repack);
    }

    for (auto& packet : RepackedIns) {
        for (auto& ins : packet) cout << ins << " ";
        cout << endl;
    }

    // Generate binary instructions
    vector<vector<string> > BinPackIns;
    for (auto& packet : RepackedIns) {
        vector<string> BinPack;
        for (auto& j : packet) {
            string BinIns;
            if (j != "NOP") {
                string ins = Opc[split(j, ' ')[0]];
                string regs;
                if (find(Units[5].begin(), Units[5].end(), split(j, ' ')[0]) == Units[5].end()) {
                    vector<string> regParts = split(split(j, ' ')[1], ',');
                    for (auto& k : regParts) regs += toBin(stoi(k.substr(1)));
                } else {
                    vector<int> tmp;
                    for (auto& k : split(split(j, ' ')[1], ',')) tmp.push_back(stoi(k.substr(1)));
                    if (ins == "01") regs = toBin(tmp[0]) + toBin(tmp[1], 25);
                    else regs = toBin(tmp[0]) + toBin(tmp[1], 10);
                }
                BinIns = ins + regs + string(32 - (ins + regs).length(), '0');
            } else {
                BinIns = string(32, '0');
            }
            BinPack.push_back(BinIns);
        }
        BinPackIns.push_back(BinPack);
    }

    vector<string> FinalBinIns;
    for (auto& packet : BinPackIns) {
        string combined = "192'b" + accumulate(packet.begin(), packet.end(), string(""));
        cout<<combined<<"\n";
        FinalBinIns.push_back(combined);
    }

    vector<string> VerilogAssign;
    for (size_t i = 0; i < FinalBinIns.size(); i++) {
        VerilogAssign.push_back("InstructionMem[" + to_string(i) + "]=" + FinalBinIns[i] + ";");
    }

    // Process testbench
    ifstream tb("Processor.v");
    string tbContent((istreambuf_iterator<char>(tb)), istreambuf_iterator<char>());
    tb.close();

    size_t pos = tbContent.find("//pyc_pushcode");
    string new_tb = tbContent.substr(0, pos);
    for (auto& line : VerilogAssign) new_tb += "\n\t\t" + line;
    new_tb += tbContent.substr(pos);

    ofstream out("NewProcessor.v");
    out << new_tb;
    out.close();

    cout << "Generated new testbench file..........\n";
/*
#ifdef _WIN32
    cout << "Unsupported OS!\n";
    return 1;
#else
    cout << "Compiling testbench...........\n";
    system("iverilog NewProcessor.v");

    cout << "Running testbench............\n";
    system("./a.out > output.txt");

    cout << "Parsing output................\n";
    ifstream dumpFile("output.txt");
    string dump((istreambuf_iterator<char>(dumpFile)), istreambuf_iterator<char>());
    dumpFile.close();

    vector<vector<string > > dumpLines;
    size_t begPos = 0;
    while ((begPos = dump.find("#BEG", begPos)) != string::npos) {
        size_t endPos = dump.find("#END", begPos);
        string section = dump.substr(begPos + 4, endPos - begPos - 4);
        dumpLines.push_back(split(section, '\n'));
        begPos = endPos;
    }
    dumpLines.erase(dumpLines.begin());
    vector<vector<int > > newDump;
    // Convert first line from strings to ints
    vector<int> firstLine;
    for (const auto& s : vector<string>(dumpLines[1].begin(), dumpLines[1].end() - 1)) {
        if (!s.empty()) {  // Check for empty strings to avoid stoi error
            firstLine.push_back(stoi(s));
        }
    }

    newDump.push_back(firstLine);
    
    for (size_t i = 1; i < dumpLines.size(); i++) {
        vector<string> curr(dumpLines[i].begin(), dumpLines[i].end() - 1);
        vector<string> prev(dumpLines[i-1].begin(), dumpLines[i-1].end() - 1);
        if (curr != prev) {
            vector<int> intLine;
            for (auto& s : curr) {
                if (!s.empty()) {  // Check for empty strings
                    intLine.push_back(stoi(s));
                }
            }
            newDump.push_back(intLine);
        }
    }
    cout << "Time \t PC \t Register Values\n";
    for (auto& line : newDump) {
        cout << line[0] << "\t" << line.back() << "\t";
        for (size_t i = 1; i < line.size() - 1; i++) cout << line[i] << " ";
        cout << endl;
    }

    system("rm NewProcessor.v a.out output.txt");
#endif 
*/

    return 0;
}