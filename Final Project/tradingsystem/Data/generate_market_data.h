//
// Created by Harper RUI on 12/23/22.
//

//
//generate marketdata.txt
//--------
//code | price |  num | direction
//--------

#ifndef DATA_GENERATE_MARKET_DATA_H
#define DATA_GENERATE_MARKET_DATA_H
#include "Bond_info.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
using namespace std;


class Generate_Market_Data{
public:
    Generate_Market_Data();
    //generate trade
    void run(int N, int kind);

};

Generate_Market_Data::Generate_Market_Data(){}

void Generate_Market_Data::run(int N, int kind=7){
    //N: the number of price for each security
    //kind: the number of securities
    ofstream file;
    file.open("../marketdata.txt");
    vector<int> count(7,0); // count the number of price for each security
    for(int i = 0; i < kind; i++){
//        int j = rand() % kind;
//        //if j-th bond has enough data
//        while(count[j]==N){
//            j = rand() % kind;
//        }

        string code = bond_code[i];

        //The top level should have a size of 10 million,
        // second level 20 million, 30 million for the third,
        // 40 million for the fourth, and 50 million for the fifth.
        for (int j=0; j< N/10; j++) {
            //The file should create mid prices which oscillate between 99 and 101
            for (int m = 1; m <= 5; m++) {
                string num = to_string(m * 1000000);
                //The file should create mid prices which oscillate between 99 and 101
                vector<string> vec_bid{"99-316", "99-315", "99-31+", "99-313", "99-312"};
                vector<string> vec_ask{"100-301", "100-302", "100-303", "100-30+", "100-315"};

                string bid_price = vec_bid[m - 1];
                string ask_price = vec_ask[m - 1];

                file << code << "," << bid_price << "," << num << "," << "BID" << endl;
                file << code << "," << ask_price << "," << num << "," << "OFFER" << endl;
                count[j] += 2;
            }
        }

    }
    cout<<"-------- marketdata.txt finished updating --------" <<endl;
    file.close();
}
#endif //DATA_GENERATE_MARKET_DATA_H
