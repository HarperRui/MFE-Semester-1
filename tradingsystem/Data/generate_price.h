//
// Created by Harper RUI on 12/22/22.
//
//generate prices.txt
//--------
//code | price | spread
//--------

//Create 1,000,000 prices for each security (so a total of 7,000,000 prices across all 7 securities).
// The file should create prices which oscillate between 99 and 101,
// moving by the smallest increment each time up from 99 and then down from 101 (bearing in mind that US Treasuries trade in 1/256th increments).
//The bid/offer spread should oscillate between 1/128 and 1/64.

#ifndef DATA_GENERATE_PRICE_H
#define DATA_GENERATE_PRICE_H
#include "Bond_info.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
using namespace std;

class Generate_Price{
public:
    Generate_Price();
    //generate price
    void run(int N, int kind);

};

Generate_Price::Generate_Price(){}

void Generate_Price::run(int N, int kind=7){
    //N: the number of price for each security
    //kind: the number of securities
    ofstream file;
    file.open("../prices.txt");
    vector<int> count(7,0); // count the number of price for each security
    for(int i = 0; i < kind * N; i++){
        int j = rand() % kind;
        //if j-th bond has enough data
        while(count[j]==N){
            j = rand() % kind;
        }

        string code = bond_code[j];
        count[j]++;

        // moving by the smallest increment each time up from 99 and then down from 101
        string a = to_string(rand()%2+99);
        string b = to_string(rand()%32);
        string c = to_string(rand()%8);

        if(stoi(b)<10){
            b = "0"+b;
        }
        if(stoi(c)==4){
            c = "+";
        }

        string price = a + "-" + b + c;

        //The bid/offer spread should oscillate between 1/128 and 1/64.
        string spread;
        int rdn = rand()%4;
        if(rdn==0){
            spread = "0-001";
        }else if (rdn==1){
            spread = "0-002";
        }else if (rdn==2){
            spread = "0-003";
        }else {
            spread = "0-00+";
        }
        //cout << code <<","<<price<<","<<spread<<endl;
        file << code <<","<<price<<","<<spread<<endl;

    }
    cout<<"-------- prices.txt finished updating --------" <<endl;
    file.close();

}
#endif //DATA_GENERATE_PRICE_H
