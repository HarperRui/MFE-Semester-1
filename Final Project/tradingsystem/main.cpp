#include <iostream>
#include <string>
#include <map>

#include "soa.hpp"
#include "products.hpp"
#include "positionservice.hpp"
#include "pricingservice.hpp"
#include "marketdataservice.hpp"
#include "executionservice.hpp"
#include "inquiryservice.hpp"
#include "historicaldataservice.hpp"
#include "riskservice.hpp"
#include "streamingservice.hpp"
#include "tradebookingservice.hpp"
#include "GUIService.h"
#include "BondAlgoStreamingService.h"
#include "BondAlgoExecutionService.h"
#include "./Data/generate_trade.h"
#include "./Data/generate_price.h"
#include "./Data/generate_market_data.h"
#include "./Data/generate_inquiry.h"
#include "./Data/Bond_info.h"
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;

int main() {
    cout<<"Generate raw data..."<<endl;
    //generate prices.txt
    //--------
    //code | price | spread
    //--------
    Generate_Price generate_price = Generate_Price();
    generate_price.run(1000000);

    //generate trades.txt
    //--------
    //code | trader_id | price | book | num | direction
    //--------
    Generate_Trade generate_trade = Generate_Trade();
    generate_trade.run(10);


    //generate marketdata.txt
    //--------
    //code | price |  num | direction
    //--------
    Generate_Market_Data generate_market_data = Generate_Market_Data();
    generate_market_data.run(1000000);


    //generate inquiries.txt
    //--------
    //code | price |  num | direction | RECEIVED
    //--------
    Generate_Inquiry generate_inquiry = Generate_Inquiry();
    generate_inquiry.run(10);
    cout<<"Finished generate raw data..."<<endl;

    PricingService<Bond> pricing_service;
    BondTradeBookingService<Bond> trade_booking_service;
    BondPositionService<Bond> position_service;
    BondRiskService<Bond> risk_ervice;
    BondMarketDataService<Bond> market_data_service;
    BondAlgoExecutionService<Bond> algo_execution_service;
    BondAlgoStreamingService<Bond> algo_streaming_service;
    GUIService<Bond> gui_service;
    BondExecutionService<Bond> execution_service;
    BondStreamingService<Bond> streaming_service;
    BondInquiryService<Bond> inquiry_service;
     //the number of different kind bonds
    int kind = 7;

    BondAlgoStreamingServiceListener<Bond>* algo_streaming_service_listener =new BondAlgoStreamingServiceListener<Bond>(algo_streaming_service);

    pricing_service->AddListener(algo_streaming_service_listener);

    cout << boost::posix_time::second_clock::local_time() << "Price Data is Running..." << endl;
    ifstream price("prices.txt");
    pricing_service.GetConnector()->Subscribe(price);
    cout << "Finished Price Data" << endl;
    cout << boost::posix_time::second_clock::local_time() << "Trade Data is Running..." << endl;
    ifstream trade("trades.txt");
    trade_booking_service.GetConnector()->Subscribe(trade);
    cout  << "Finished Trade Data" << endl;

    cout << boost::posix_time::second_clock::local_time() << "Market Data is Running..." << endl;
    ifstream market("marketdata.txt");
    market_data_service.GetConnector()->Subscribe(market);
    cout << "Finished Market Data" << endl;

    cout << boost::posix_time::second_clock::local_time() << "Inquiry Data is Running..." << endl;
    ifstream inquiry("inquiries.txt");
    inquiry_service.GetConnector()->Subscribe(inquiry);
    cout  << "Finished Inquiry Data" << endl;

    cout<<"-------- END--------"<<endl;
}
