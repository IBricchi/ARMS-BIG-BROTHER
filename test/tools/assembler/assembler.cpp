#include <iostream>
#include <vector>
#include <unordered_map>

#include "scanner.h"
#include "scanner.cpp"

using namespace std;

int main(){
    Scanner scanner = Scanner();
    unordered_map<string, uint32_t> labels{};

    string line;
    bool scanError = false;
    // scan file using scanner
    for(getline(cin, line); !cin.fail(); getline(cin, line)){
        if(line == "asd") break;
        scanner.scanLine(line);
        scanError |= scanner.error;
        scanner.error = false;
    }
    // assign label tokens to label map
    for(Token label : *scanner.getLabelsAddr()){
        if(labels.find(label.name) != labels.end()){
            scanError = true;
            cerr << "Error: [Line " << label.line << "] Label '" << label.name << "' has already been assigned." << endl;
            continue;
        }
        labels.emplace(label.name, label.line);
    }
    for(auto it = scanner.getTokensAddr()->begin(); it != scanner.getTokensAddr()->end(); it++){
        if(it->label != ""){
            // check if label exists
            if(labels.find(it->label) == labels.end()){
                scanError = true;
                cerr << "Error: [Line " << it->line << "] Label '" << it->label << "' doesn't exist." << endl;
            }
            else{
                uint32_t memLine = labels.at(it->label);
                uint32_t op = it->data;
                op >>= 26;
                if(op == 0b000010 || op == 0b000011){ // if jump pattern
                    uint32_t checkLineSize = memLine << 6;
                    checkLineSize >>= 6;
                    if(checkLineSize != memLine){  
                        scanError = true;
                        cerr << "Error: [Line " << it->line << "] Label '" << it->label << "' points to address too large to store in J or JAL instruction. Max size is 26 bits, label points to: 0x" << hex << memLine << "." << endl;
                    }
                    else{
                        it->data |= memLine;
                    }
                }
                else{
                    uint32_t jumpDiff = (memLine - (it->memLine + 4)) >> 2;
                    uint16_t checkLineSize = jumpDiff;
                    uint32_t checkLineSizeSignExtended = jumpDiff | (0xffff0000 * jumpDiff>>15 + 0xffff * !jumpDiff>>1);
                    if(checkLineSizeSignExtended != jumpDiff){  
                        scanError = true;
                        cerr << "Error: [Line " << it->line << "] Label '" << it->label << "' is too far away from current instructions, jump is too large. Max jump size stored in 16 bits, calculated difference of: 0x" << hex << jumpDiff << "." << endl;
                    }
                    else{
                        it->data |= memLine;
                    }
                }
            }
        }
    }

    for(Token instr : *scanner.getTokensAddr()){
        cout << "0x" << hex << instr.data << endl;
    }
}

