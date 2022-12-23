//
// Created by Harper RUI on 12/22/22.
//

#ifndef TRADINGSYSTEM_BONDALGOSTREAMINGSERVICE_H
#define TRADINGSYSTEM_BONDALGOSTREAMINGSERVICE_H

#include <iostream>
#include "products.hpp"
#include "pricingservice.hpp"
#include "streamingservice.hpp"
#include "BondAlgoExecutionService.h"
#include <random>
using namespace std;



template<typename T>
class BondAlgoStreamingService: public Service<string, AlgoStreaming<T>>{
private:
    map<string, AlgoStreaming<T>> algo_map;
    vector<ServiceListener<AlgoStreaming<T>>*> listeners;
public:
    BondAlgoStreamingService();

    // Get data on our service given a key
    virtual AlgoStreaming<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(AlgoStreaming<T> &data)  override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<AlgoStreaming<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<AlgoStreaming<T>>* >& GetListeners() const override;

    // update price
    void update_price(Price<T> & price);

};


template<typename T>
class BondAlgoStreamingServiceListener: public ServiceListener<Price<T>>{
private:
    BondAlgoStreamingService<T>*  bond_algo_stream_service;
public:
    BondAlgoStreamingServiceListener(BondAlgoStreamingService<T>* service);
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(Price<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(Price<T> &data) override{};

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(Price<T> &data) override{};

};


template<typename T>
AlgoStreaming<T>::AlgoStreaming(PriceStream<T>& stream) {
    price_stream = stream;
}
template<typename T>
void AlgoStreaming<T>::Run(Price<T> price){
    auto bond = price.GetProduct();
    // not this PriceStream to update
    if(bond.GetProductId() == price_stream.GetProduct().GetProductId()) {
        auto mid = price.GetMid();
        auto spread = price.GetBidOfferSpread();
        auto bid = mid - spread / 2.0;
        auto ask = mid + spread / 2.0;

        // when the spread is at its tightest (i.e. 1/128th)
        if (spread <= 1.0 / 128.0) {
            auto visible_num = (rand()%2+1) * 1000000;
            PriceStreamOrder order_bid(bid, visible_num, 2*visible_num, BID);
            PriceStreamOrder order_ask(ask, visible_num, 2*visible_num, OFFER);
            price_stream = PriceStream<T>(bond, order_bid, order_ask);
        } else {
            PriceStreamOrder order_bid(bid, 0, 0, BID);
            PriceStreamOrder order_ask(ask, 0, 0, OFFER);
            price_stream = PriceStream<T>(bond, order_bid, order_ask);

        }

    }
}




template<typename T>
BondAlgoStreamingService<T>::BondAlgoStreamingService(){
    algo_map = map<string,AlgoStreaming<T>>();
}


// Get data on our service given a key
template<typename T>
AlgoStreaming<T>& BondAlgoStreamingService<T>::GetData(string key){
    return algo_map[key];
}

// The callback that a Connector should invoke for any new or updated data
template<typename T>
void BondAlgoStreamingService<T>::OnMessage(AlgoStreaming<T> &data) {

}

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondAlgoStreamingService<T>::AddListener(ServiceListener<AlgoStreaming<T>> *listener) {
    listeners.push_back(listener);
}

// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<AlgoStreaming<T>>* >& BondAlgoStreamingService<T>::GetListeners() const {
    return listeners;
}

// update price
template<typename T>
void BondAlgoStreamingService<T>::update_price(Price<T> & price){
    auto bond_code=price.GetProduct().GetProductId();

    // if it in algo_map, update its price
    if(auto i = algo_map.find(bond_code) !=algo_map.end()){
        (i->second).Run(price);
    } else{
        PriceStreamOrder ps_bid(0, 0, 0, BID);
        PriceStreamOrder ps_ask(0, 0, 0, OFFER);
        PriceStream<Bond> ps(price.GetProduct(), ps_bid, ps_ask);
        algo_map.insert(pair<string,PriceStream<Bond> >(bond_code,ps));
        update_price(price);
        return;
    }

    // notify the listeners
    for(auto& i:listeners){
        i->ProcessAdd(i->second);
    }
}


template<typename T>
BondAlgoStreamingServiceListener<T>::BondAlgoStreamingServiceListener(BondAlgoStreamingService<T>* service){
    bond_algo_stream_service = service;
}
// Listener callback to process an add event to the Service
template<typename T>
void BondAlgoStreamingServiceListener<T>::ProcessAdd(Price<T> &data) {
    bond_algo_stream_service ->update_price(data);
}



#endif //TRADINGSYSTEM_BONDALGOSTREAMINGSERVICE_H
