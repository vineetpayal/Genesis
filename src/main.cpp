#include<iostream>
using namespace std;

int main(int argc, char* argv[]){

    if(argc != 2){
        cerr<<"Incorrect usage. Correct usage is..."<<endl;
        cerr<<"geny <filename>.gy"<<endl;
        return EXIT_FAILURE;
    }

    cout<<"hello world"<<endl;
    return EXIT_SUCCESS;
}