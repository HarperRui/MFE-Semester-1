//
// Created by Harper RUI on 12/22/22.
//

#ifndef TRADINGSYSTEM_BONDALGOEXECUTIONSERVICE_H
#define TRADINGSYSTEM_BONDALGOEXECUTIONSERVICE_H
#include <iostream>
#include "executionservice.hpp"
#include <random>
#include "marketdataservice.hpp"
using namespace std;



template<typename T>
class BondAlgoExecutionService: public Service<string, AlgoExecution<T>>{
private:
    map<string, AlgoExecution<T>> algo_map;
    vector<ServiceListener<AlgoExecution<T>>*> listeners;
public:
    BondAlgoExecutionService();

    // Get data on our service given a key
    virtual AlgoExecution<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(AlgoExecution<T> &data)  override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<AlgoExecution<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<AlgoExecution<T>>* >& GetListeners() const override;

    // update information
    void update_orderbook(OrderBook<T> & order_book);

};


template<typename T>
class BondAlgoExecutionListener: public ServiceListener<OrderBook<T>>{
private:
    BondAlgoExecutionService<T>*  bond_algo_exe_service;
public:
    BondAlgoExecutionListener(BondAlgoExecutionService<T>* service);
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(OrderBook<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(OrderBook<T> &data) override{};

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(OrderBook<T> &data) override{};

};






template<typename T>
BondAlgoExecutionService<T>::BondAlgoExecutionService(){
    algo_map = map<string, AlgoExecution<T>>();
}

// Get data on our service given a key
template<typename T>
AlgoExecution<T>& BondAlgoExecutionService<T>::GetData(string key) {
    return algo_map[key];
}

// The callback that a Connector should invoke for any new or updated data
template<typename T>
void BondAlgoExecutionService<T>::OnMessage(AlgoExecution<T> &data) {

}

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondAlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecution<T>> *listener) {
    listeners.push_back(listeners);
}

// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<AlgoExecution<T>>* >& BondAlgoExecutionService<T>::GetListeners() const {
    return listeners;
}

// update information
template<typename T>
void BondAlgoExecutionService<T>::update_orderbook(OrderBook<T> & order_book){
    string bond_code = order_book.GetProduct().GetProductId();
    // if it in algo_map, update its price
    if(auto i= algo_map.find(bond_code) != algo_map.end()){
        (i->second).Run(order_book);
    } else{
        auto exe_order = ExecutionOrder<T>(order_book.GetProduct(), BID,"orderID",LIMIT,0,0,0,"parentID",true);
        algo_map.insert(pair<string,AlgoExecution<T>>(bond_code,AlgoExecution<T>(exe_order)));
        update_orderbook(order_book);
        return;
    }

    for(auto& i:listeners){
        i->ProcessAdd(i->second);
    }

}


template<typename T>
BondAlgoExecutionListener<T>::BondAlgoExecutionListener(BondAlgoExecutionService<T>* service){
    bond_algo_exe_service = service;
}

template<typename T>
void BondAlgoExecutionListener<T>::ProcessAdd(OrderBook<T> &data) {
    bond_algo_exe_service->update_orderbook(data);
}

#endif //TRADINGSYSTEM_BONDALGOEXECUTIONSERVICE_H
