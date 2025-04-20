#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include "tokenization.hpp"
using namespace std;


vector<Token> tokenize(const string &str) {
}

string tokens_to_asm(vector<Token> &tokens) {
    stringstream output;
    output << "global _start\n_start:\n";

    for (int i = 0; i < tokens.size(); i++) {
        Token token = tokens[i];
        if (token.type == TokenType::exit) {
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::int_lit) {
                if (i + 2 < tokens.size() && tokens[i + 2].type == TokenType::semi) {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens[i + 1].value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }
    return output.str();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Incorrect usage. Correct usage is..." << endl;
        cerr << "geny <filename>.gn" << endl;
        return EXIT_FAILURE;
    }

    // reading from our source file
    fstream input(argv[1], ios::in);
    stringstream contents_stream;
    contents_stream << input.rdbuf();
    input.close();
    string contents = contents_stream.str();
    cout << contents << endl;

    // lexical analysis
    Tokenizer tokenizer(move(contents));
    vector<Token> tokens = tokenizer.tokenize(); {
        fstream file("../out.asm", ios::out);
        file << tokens_to_asm(tokens);
    }

    system("nasm -felf64 ../out.asm");
    system("ld -o out ../out.o");

    return EXIT_SUCCESS;
}
