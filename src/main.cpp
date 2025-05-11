#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include "tokenization.hpp"
#include "parser.hpp"
#include "generation.hpp"
#include "arena.hpp"
using namespace std;

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
    vector<Token> tokens = tokenizer.tokenize();

    //Parsing
    Parser parser(move(tokens));
    optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        cerr << "Invalid program" << endl;
        exit(EXIT_FAILURE);
    }

    //Intermediate Code Generation
    Generator generator(prog.value()); {
        fstream file("../out.asm", ios::out);
        file << generator.gen_prog();
    }

    system("nasm -felf64 ../out.asm");
    system("ld -o out ../out.o");

    return EXIT_SUCCESS;
}
