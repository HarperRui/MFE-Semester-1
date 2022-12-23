//
// Created by Harper RUI on 12/22/22.
//
//generate trades.txt
//--------
//code | trader_id | price | book | num | direction
//--------

//Create 10 trades for each security (so a total of 70 trades across all 7 securities) in the file
// with the relevant trade attributes.
// Positions should be across books TRSY1, TRSY2, and TRSY3.
// The BondTradeBookingService should be linked to a BondPositionService via a ServiceListener
// and send all trades there via the AddTrade() method
// (note that the BondTradeBookingService should not have an explicit reference to the BondPositionService though or vice versa – link them through a ServiceListener).
// Trades for each security should alternate between BUY and SELL and cycle from 1000000, 2000000, 3000000, 4000000, and 5000000 for quantity,
// and then repeat back from 1000000. The price should oscillate between 99.0 (BUY) and 100.0 (SELL).

#ifndef DATA_GENERATE_TRADE_H
#define DATA_GENERATE_TRADE_H
#include "Bond_info.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
using namespace std;


class Generate_Trade{
public:
    Generate_Trade();
    //generate trade
    void run(int N, int kind);

};

Generate_Trade::Generate_Trade(){}

void Generate_Trade::run(int N, int kind=7){
    //N: the number of price for each security
    //kind: the number of securities
    ofstream file;
    file.open("../trades.txt");
    vector<int> count(7,0); // count the number of price for each security
    for(int i = 0; i < kind * N; i++){
        int j = rand() % kind;
        //if j-th bond has enough data
        while(count[j]==N){
            j = rand() % kind;
        }

        string code = bond_code[j];
        count[j]++;
        string trader_id = "Trader"+to_string(j);

        //Positions should be across books TRSY1, TRSY2, and TRSY3
        string book = "TRSY"+to_string(rand()%3+1);
        //cycle from 1000000, 2000000, 3000000, 4000000, and 5000000
        string num = to_string((rand()%5+1)*1000000);
        //Trades for each security should alternate between BUY and SELL
        // The price should oscillate between 99.0 (BUY) and 100.0 (SELL).
        vector<string> all_direction{"BUY", "SELL"};
        vector<string> all_prices{"99-000", "100-000"};
        int m = rand()%2;
        string direction = all_direction[m];
        string price = all_prices[m];

        file << code <<","<<trader_id<<","<<price<<","<<book<<","<<num<<","<<direction<<endl;

    }
    cout<<"-------- trades.txt finished updating --------" <<endl;
    file.close();

}



#endif //DATA_GENERATE_TRADE_H
