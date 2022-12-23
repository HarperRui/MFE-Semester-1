/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // ctor for a position
  Position(const T &_product);

  // Get the product
  const T& GetProduct() const;

  // Get the position quantity
  long GetPosition(string &book);

  // Get the aggregate position
  long GetAggregatePosition();
//  // update the position
//  void AddPosition(string& book, long position);

private:
  T product;
  map<string,long> positions;

};


/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{

public:

  // Add a trade to the service
  virtual void AddTrade(const Trade<T> &trade) = 0;

};


//The BondPositionService does not need a Connector
// since data should flow via ServiceListener from the BondTradeBookingService.
// The BondPositionService should be linked to a BondRiskService via a ServiceListenher
// and send all positions to the BondRiskService via the AddPosition() method
// (note that the BondPositionService should not have an explicit reference to the BondRiskService though or versa
// – link them through a ServiceListener).
template<typename T>
class BondPositionService: public PositionService<T>{
private:
    map<string, Position<T>> position_map;
    vector<ServiceListener<Position<T>>*> listeners;
public:
    //ctor
    BondPositionService();
    // Get data on our service given a key
    virtual Position<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(Position<T> &data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<Position<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<Position<T>>* >& GetListeners() const override;

    // Add a trade to the service
    virtual void AddTrade(const Trade<T> &trade) override;

};


template<typename T>
class BondPositionServiceListener: public ServiceListener<Trade<T>>{
private:
    BondPositionService<T>*  bond_position_service;
public:
    BondPositionServiceListener(BondPositionService<T>* service);
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(Trade<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(Trade<T> &data) override{};

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(Trade<T> &data) override{};

};



template<typename T>
Position<T>::Position(const T &_product) :
  product(_product)
{
}

template<typename T>
const T& Position<T>::GetProduct() const
{
  return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
  return positions[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
  // No-op implementation - should be filled out for implementations
  return 0;
}

//template<typename T>
//void Position<T>::AddPosition(string& book, long position) {
//    positions[book] += position;
//}

//ctor
template<typename T>
BondPositionService<T>::BondPositionService(){
    position_map = map<string, Position<T>>();
}
// Get data on our service given a key
template<typename T>
Position<T>& BondPositionService<T>::GetData(string key){
    return position_map[key];
}

// The callback that a Connector should invoke for any new or updated data
template<typename T>
void BondPositionService<T>::OnMessage(Position<T> &data){
    position_map[data.GetProduct().GetProductID()] = data;
}

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondPositionService<T>::AddListener(ServiceListener<Position<T>> *listener) {
    listeners.push_back(listener);
}

// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<Position<T>>* >& BondPositionService<T>::GetListeners() const{
    return listeners;
}
// Add a trade to the service
template<typename T>
void BondPositionService<T>::AddTrade(const Trade<T> &trade){
    T bond = trade.GetProduct();
    string bond_code = bond.GetProductID();
    double price = trade.GetPrice();
    string book = trade.GetBook();
    auto num = trade.GetQuantity();
    Side side = trade.GetSide();
    auto i = position_map.find(bond_code);
    if (side != BUY){
        num = -num;
    }

    if(i!=position_map.end()){
        (i->second).AddPosition(trade.GetBook(), num);
    }else{
        position_map.insert(pair<string, Position<T>>(bond_code, Position<T>(bond)));
        AddTrade(trade);
    }

    for(auto& each:listeners){
        each->ProcessAdd(i->second);
    }

}


template<typename T>
BondPositionServiceListener<T>:: BondPositionServiceListener(BondPositionService<T>* service){
    bond_position_service = service;
};

// Listener callback to process an add event to the Service
template<typename T>
void BondPositionServiceListener<T>:: ProcessAdd(Trade<T> &data) {
    bond_position_service->AddTrade(data);
}



#endif
