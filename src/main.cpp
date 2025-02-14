#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "codeGen.h"
#include "TAC_to_ASM.h"

using namespace std;

int main(int argc, char ** argv){
    if(argc <2){
        std::cerr << "Incorrect Usage. Correct usage is..." << std::endl;
        std::cerr << "edcomp <input.eco>" << std::endl;
        
        return EXIT_FAILURE; 
    }

    std::string contents;
    std::stringstream file_contents;
    
    {
        std::fstream input_file(argv[1], std::ios::in);
        file_contents <<  input_file.rdbuf();
        contents = file_contents.str();
    }

    // tokenize(contents);
    Lexer lexer(contents);
    // while(*lexer.BufferPtr){
    //     Token token;
    //     lexer.next(token);
    //     cout << TokenStr[token.type] << ": " << token.value.value_or("") << endl;
    // }

    Parser parser(lexer);
    ASTProgram *prog = parser.parse();
    // prog->print();
    string tempVar;
    std::vector<TAC> tacCode = prog->generateTAC(tempVar);
    
    for (auto &tac : tacCode) {
        tac.print();
    }
    ofstream outfile("aprog.S");

    // CodeGenerator codeGen(outfile);
    // codeGen.generate(prog);

    TACtoASM codeGen(outfile);
    codeGen.generateAssembly(tacCode);

    return EXIT_SUCCESS;
}