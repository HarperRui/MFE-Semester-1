#include <iostream>
#include "Bond_info.h"
#include "generate_price.h"
#include "generate_trade.h"
#include "generate_market_data.h"
#include "generate_inquiry.h"
using namespace std;


int main() {
    cout << "Generating Data!" << std::endl;
    //generate prices.txt
    //--------
    //code | price | spread
    //--------
//    Generate_Price generate_price = Generate_Price();
//    generate_price.run(1000000);

    //generate trades.txt
    //--------
    //code | trader_id | price | book | num | direction
    //--------
//    Generate_Trade generate_trade = Generate_Trade();
//    generate_trade.run(10);


    //generate marketdata.txt
    //--------
    //code | price |  num | direction
    //--------
//    Generate_Market_Data generate_market_data = Generate_Market_Data();
//    generate_market_data.run(1000000);


    //generate inquiries.txt
    //--------
    //code | price |  num | direction | RECEIVED
    //--------
    Generate_Inquiry generate_inquiry = Generate_Inquiry();
    generate_inquiry.run(10);
    return 0;
}
