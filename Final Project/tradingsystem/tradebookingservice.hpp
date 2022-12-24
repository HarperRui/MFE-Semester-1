/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 *
 * @author Breman Thuraisingham
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include "executionservice.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

  // ctor for a trade
  Trade() = default;
  Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

  // Get the product
  const T& GetProduct() const;

  // Get the trade ID
  const string& GetTradeId() const;

  // Get the mid price
  double GetPrice() const;

  // Get the book
  const string& GetBook() const;

  // Get the quantity
  long GetQuantity() const;

  // Get the side
  Side GetSide() const;

private:
  T product;
  string tradeId;
  double price;
  string book;
  long quantity;
  Side side;

};

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string,Trade <T> >
{

public:

  // Book the trade
  virtual void BookTrade(const Trade<T> &trade) = 0;

};

//This should read data from trades.txt
// (again, with a separate process reading from the file and publishing via socket into the trading system,
// which populates via a Connector into the BondTradeBookingService).
template<typename T>
class BondTradeBookingService: public TradeBookingService<Trade <T>>{
private:
    map<string, Trade<T>> trade_map;
    vector<ServiceListener<Trade<T>>*> listeners;
public:
    BondTradeBookingService();

    // Get data on our service given a key
    virtual Trade<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(Trade<T> &data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<Trade<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<Trade<T>>* >& GetListeners() const override;
    //Book the trade
    virtual void BookTrade(Trade<T> trade) override;
};


template<typename T>
class BondTradeBookingServiceConnector: public Connector<Trade<T>>{
private:
    BondTradeBookingService<T>* bond_trade_booking_service;
    BondProductService* bond_product_service;
public:
    BondTradeBookingServiceConnector(BondTradeBookingService<T>* trade_service, BondProductService* product_service);
    ~BondTradeBookingServiceConnector();
    virtual void Publish(Trade<T>& data) override {};
    void Subscribe(ifstream& data);
};

template<typename T>
class BondTradeBookingServiceListener: public ServiceListener<ExecutionOrder<T>>{
private:
    BondTradeBookingService<T>* bond_trade_booking_service;
    long count;
public:
    BondTradeBookingServiceListener(BondTradeBookingService<T>* service);
    ~BondTradeBookingServiceListener();
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(ExecutionOrder<T>& data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(ExecutionOrder<T>& data) override {};

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(ExecutionOrder<T>& data) override {};
};






template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
  product(_product)
{
  tradeId = _tradeId;
  price = _price;
  book = _book;
  quantity = _quantity;
  side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
  return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
  return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
  return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
  return side;
}

template<typename T>
void TradeBookingService<T>::BookTrade(const Trade<T> &trade)
{
}


template<typename T>
BondTradeBookingService<T>::BondTradeBookingService(){
    trade_map = map<string, Trade<T>>();
}

// Get data on our service given a key
template<typename T>
Trade<T>& BondTradeBookingService<T>::GetData(string key){
    return trade_map[key];
}

// The callback that a Connector should invoke for any new or updated data
template<typename T>
void BondTradeBookingService<T>::OnMessage(Trade<T> &data){
    string bond_code = data.GetProduct().GetProductID(); //********
    if(trade_map.find(bond_code)!=trade_map.end()){
        trade_map.erase(bond_code);
    }
    trade_map.insert(pair<string, Trade<T>>(bond_code, data));
    BookTrade(data);
}

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondTradeBookingService<T>::AddListener(ServiceListener<Trade<T>> *listener){
    listeners.push_back(listener);
}

// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<Trade<T>>* >& BondTradeBookingService<T>::GetListeners() const{
    return listeners;
}
//Book the trade
template<typename T>
void BondTradeBookingService<T>::BookTrade(Trade<T> trade){
    for (auto& i:listeners){
        i->ProcessAdd(trade);
    }
}





template<typename T>
BondTradeBookingServiceConnector<T>::BondTradeBookingServiceConnector(BondTradeBookingService<T>* trade_service, BondProductService* product_service) {
    bond_trade_booking_service = trade_service;
    bond_product_service = product_service;
}
template<typename T>
BondTradeBookingServiceConnector<T>::~BondTradeBookingServiceConnector(){}


template<typename T>
void BondTradeBookingServiceConnector<T>::Subscribe(ifstream& data){
    //generate trades.txt
    //--------
    //code | trader_id | price | book | num | direction
    //--------
    string info;
    while(getline(data, info)){
    stringstream info_stream(info);
    vector<string> vec_s;
    string s;
    while(getline(info_stream,s,',')){
        vec_s.push_back(s);
    }
    string bond_code = vec_s[0];
    string trader_id = vec_s[1];
    double price = transform_data_to_price(vec_s[2]);
    string book = vec_s[3];
    long num = stol(vec_s[4]);
    string direction = vec_s[5];
    Side side;
    if (direction =="BUY"){
        side = BUY;
    }else{
        side = SELL;
    }
    auto bond = bond_product_service->GetData(bond_code);
    Trade<T> trade(bond,trader_id, price, book, num, side);
    bond_trade_booking_service->OnMessage(trade);
    }
}



template<typename T>
BondTradeBookingServiceListener<T>::BondTradeBookingServiceListener(BondTradeBookingService<T>* service){
    bond_trade_booking_service = service;
    count = 0;
}

template<typename T>
BondTradeBookingServiceListener<T>::~BondTradeBookingServiceListener(){}

// Listener callback to process an add event to the Service
template<typename T>
void BondTradeBookingServiceListener<T>::ProcessAdd(ExecutionOrder<T>& data){
    count++;
    T bond = data.GetProduct();
    string trade_id = "execution_order";
    double price = data.GetPrice();
    string book = "execution_book";
    auto num = data.GetVisibleQuantity() + data.GetHiddenQuantity();
    Side side;
    if(data.GetSide()==OFFER){
        side=BUY;
    }else{
        side=SELL;
    }
    //generate trades.txt
    //--------
    //code | trader_id | price | book | num | direction
    //--------
    Trade<T> trade(bond, trade_id, price, book, num, side);
    bond_trade_booking_service->OnMessage(trade);
    bond_trade_booking_service->BookTrade(trade);

}

#endif
