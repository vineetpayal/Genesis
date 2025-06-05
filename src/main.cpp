#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "generation.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "./geny ../<input.gn>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents; {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    } {
        Generator generator(prog.value());
        std::fstream file("../out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    system("nasm -f elf64 ../io.asm -o ../io.o");
    system("nasm -f elf64 ../out.asm -o ../out.o");

    system("ld -o out ../out.o ../io.o");

    system("./out");
    return EXIT_SUCCESS;
};
