//
// Created by Harper RUI on 12/23/22.
//
//
//generate inquiries.txt
//--------
//code | price |  num | direction | RECEIVED
//--------



#ifndef DATA_GENERATE_INQUIRY_H
#define DATA_GENERATE_INQUIRY_H
#include "Bond_info.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
using namespace std;


class Generate_Inquiry{
public:
    Generate_Inquiry();
    //generate trade
    void run(int N, int kind);

};

Generate_Inquiry::Generate_Inquiry(){}

void Generate_Inquiry::run(int N, int kind=7){
    //N: the number of price for each security
    //kind: the number of securities
    ofstream file;
    file.open("../inquiries.txt");

    vector<int> count(7,0); // count the number of price for each security
    for(int i = 0; i < kind * N; i++) {
        int j = rand() % kind;
        //if j-th bond has enough data
        while (count[j] == N) {
            j = rand() % kind;
        }

        string code = bond_code[j];
        count[j]++;

        // moving by the smallest increment each time up from 99 and then down from 101
        string a = to_string(rand() % 2 + 99);
        string b = to_string(rand() % 32);
        string c = to_string(rand() % 8);

        if (stoi(b) < 10) {
            b = "0" + b;
        }
        if (stoi(c) == 4) {
            c = "+";
        }

        string price = a + "-" + b + c;
        string num = to_string((rand() % 4 + 1) * 1000000);
        vector<string> all_direction{"BUY", "SELL"};
        string side = all_direction[rand() % 2];
        file << code << "," << price << "," << num << "," << side << "," << "RECEIVED "<< endl;
    }

    cout<<"-------- inquiries.txt finished updating --------" <<endl;
    file.close();
}
#endif //DATA_GENERATE_INQUIRY_H