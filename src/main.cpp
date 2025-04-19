#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
using namespace std;

enum class TokenType
{
    exit,
    int_lit,
    semi
};

struct Token
{
    TokenType type;
    optional<string> value;
};

vector<Token> tokenize(const string &str)
{
    vector<Token> tokens;

    string buff;

    for (int i = 0; i < str.length(); i++)
    {
        char c = str[i];

        if (isalpha(c))
        {
            buff.push_back(c);
            i++;
            while (isalnum(str.at(i)))
            {
                buff.push_back(str.at(i));
                i++;
            }
            i--;

            if (buff == "exit")
            {
                tokens.push_back({.type = TokenType::exit});
                buff.clear();
                continue;
            }
            else
            {
                cerr << "You messed up" << endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (isdigit(c))
        {
            buff.push_back(c);
            i++;
            while (isdigit(str.at(i)))
            {
                buff.push_back(str.at(i));
                i++;
            }
            i--;

            tokens.push_back({.type = TokenType::int_lit, .value = buff});
            buff.clear();
        }
        else if (c == ';')
        {
            tokens.push_back({.type = TokenType::semi});
        }

        else if (isspace(c))
        {
            continue;
        }
        else
        {
            cerr << "You messed up" << endl;
            exit(EXIT_FAILURE);
        }
    }
    return tokens;
}

string tokens_to_asm(vector<Token> &tokens)
{
    stringstream output;
    output << "global _start\n_start:\n";

    for (int i = 0; i < tokens.size(); i++)
    {
        Token token = tokens[i];
        if (token.type == TokenType::exit)
        {
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::int_lit)
            {
                if (i + 2 < tokens.size() && tokens[i + 2].type == TokenType::semi)
                {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens[i + 1].value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }
    return output.str();
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
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
    vector<Token> tokens = tokenize(contents);

    {
        fstream file("../out.asm", ios::out);
        file<<tokens_to_asm(tokens);
    }

    system("nasm -felf64 ../out.asm");
    system("ld -o out ../out.o");

    return EXIT_SUCCESS;
}