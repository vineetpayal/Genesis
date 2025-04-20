#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include "tokenization.hpp"
#include "parser.hpp"
#include "generation.hpp"
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
    optional<NodeExit> tree = parser.parse();

    if (!tree.has_value()) {
        cerr << "No exit statement found" << endl;
        exit(EXIT_FAILURE);
    }

    //Intermediate Code Generation
    Generator generator(tree.value()); {
        fstream file("../out.asm", ios::out);
        file << generator.generate();
    }

    system("nasm -felf64 ../out.asm");
    system("ld -o out ../out.o");

    return EXIT_SUCCESS;
}
