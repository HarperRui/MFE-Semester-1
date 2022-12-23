//
// Created by Harper RUI on 12/21/22.
//

#ifndef TRADINGSYSTEM_GUISERVICE_H
#define TRADINGSYSTEM_GUISERVICE_H
#include "soa.hpp"
#include "pricingservice.hpp"
#include "products.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;

template<typename T>
class GUIServiceConnector: public Connector<T>{
public:
    GUIServiceConnector(){};
    // Publish data to the Connector
    virtual void Publish(T& data) override;
};





template<typename T>
class ModifyPriceByTime: public Price<T>{
private:
    boost::posix_time::ptime time;
public:
    ModifyPriceByTime(boost::posix_time::ptime input_time,Price<T> price);
    boost::posix_time::ptime GetTime();
};



template<typename T>
class GUIService: public  PricingService<T>{
private:
    GUIServiceConnector<T>* gui_connector;
    boost::posix_time::time_duration throtte_time;
    boost::posix_time::ptime last_time;

public:
    GUIService(GUIServiceConnector<T>* connector);
    // Get data on our service given a key
    virtual Price<T>& GetData(string key) override {};

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(Price<T> &data)  override {};

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<Price<T>> *listener) override {};

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<Price<T>>* >& GetListeners() const override {};

    void send_throtte(Price<T> &data);

};









template<typename T>
ModifyPriceByTime<T>::ModifyPriceByTime(boost::posix_time::ptime input_time,Price<T> price): Price<T>(price){
    time = input_time;
}

template<typename T>
boost::posix_time::ptime ModifyPriceByTime<T>::GetTime(){
    return time;
}


// Publish data to the Connector
template<typename T>
void GUIServiceConnector<T>::Publish(T& data) {
    auto bond=data.GetProduct();
    auto mid=data.GetMid();
    auto spread=data.GetBidOfferSpread();
    auto time=data.GetTime();
    ofstream out;
    out.open("gui.txt",ios::app);
    out << time <<"," << bond.GetProductId() <<"," << mid << "," << spread << endl;
}




template<typename T>
GUIService<T>::GUIService(GUIServiceConnector<T>* connector){
    gui_connector = connector;
    //Define the GUIService with a 300 millisecond throttle
    throtte_time = boost::posix_time::millisec(3);
    last_time = boost::posix_time::microsec_clock::local_time();
}

template<typename T>
void GUIService<T>::send_throtte(Price<T> &data){
    boost::posix_time::ptime current = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration time_diff = current - last_time;
    if(time_diff > throtte_time){
        last_time = current;
        auto ts_price = ModifyPriceByTime<T>(current, data);
        gui_connector->Publish(ts_price);
    }
}






#endif //TRADINGSYSTEM_GUISERVICE_H



