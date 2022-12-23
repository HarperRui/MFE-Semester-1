//
// Created by Harper RUI on 12/22/22.
//

//Develop a bond trading system for US Treasuries with seven securities: 2Y, 3Y, 5Y, 7Y, 10Y, 20Y, and 30Y.
//Look up the CUSIPS, coupons, and maturity dates for each security. Ticker is T.

#ifndef DATA_BOND_INFO_H
#define DATA_BOND_INFO_H
#include <vector>
#include <iostream>
#include "boost/date_time/gregorian/gregorian.hpp"
using namespace std;
using namespace boost::gregorian;

// source: https://www.treasurydirect.gov/auctions/announcements-data-results/

//CUSIPS
vector<string> bond_code{
    "91282CFX4", //2Y
    "91282CFW6", //3Y
    "91282CFZ9", //5Y
    "91282CFY2", //7Y
    "91282CFV8", //10Y
    "912810TM0", //20Y
    "912810TL2" //30Y
};

//coupons
vector<float> bond_coupon{
    0.04505, //2Y
    0.04093, //3Y
    0.03974,//5Y
    0.03890,//7Y
    0.04125,//10Y
    0.03935,//20Y
    0.03513//30Y
};
//maturity dates
vector<date> bond_maturity{
    date(2024, Nov, 30), //2Y
    date(2025, Nov, 15), //3Y
    date(2027, Nov, 30), //5Y
    date(2029, Nov, 30), //7Y
    date(2032, Nov, 15),//10Y
    date(2042, Nov, 15),//20Y
    date(2052, Nov, 15),//30Y
};
//risks
map<string, float> bond_risk{
    {"91282CFX4",0.02}, //2Y
    {"91282CFW6",0.03}, //3Y
    {"91282CFZ9",0.05}, //5Y
    {"91282CFY2",0.07}, //7Y
    {"91282CFV8",0.09}, //10Y
    {"912810TM0",0.2}, //20Y
    {"912810TL2",0.2} //30Y
    };

#endif //DATA_BOND_INFO_H
