/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include <unordered_map>
#include <fstream>

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

  // ctor for an order
  Order(double _price, long _quantity, PricingSide _side);

  // Get the price on the order
  double GetPrice() const;

  // Get the quantity on the order
  long GetQuantity() const;

  // Get the side on the order
  PricingSide GetSide() const;

private:
  double price;
  long quantity;
  PricingSide side;

};

/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

  // ctor for bid/offer
  BidOffer(const Order &_bidOrder, const Order &_offerOrder);

  // Get the bid order
  const Order& GetBidOrder() const;

  // Get the offer order
  const Order& GetOfferOrder() const;

private:
  Order bidOrder;
  Order offerOrder;

};

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

  // ctor for the order book
  OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);

  // Get the product
  const T& GetProduct() const;

  // Get the bid stack
  const vector<Order>& GetBidStack() const;

  // Get the offer stack
  const vector<Order>& GetOfferStack() const;

private:
  T product;
  vector<Order> bidStack;
  vector<Order> offerStack;

};

/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{

public:

  // Get the best bid/offer order
  virtual const BidOffer& GetBestBidOffer(const string &productId) = 0;

  // Aggregate the order book
  virtual const OrderBook<T>& AggregateDepth(const string &productId) = 0;

};


template<typename T>
class BondMarketDataService: public MarketDataService<T>{
private:
    map<string, OrderBook<T>> order_map;
    vector<ServiceListener<OrderBook<T>>*> listeners;
public:
    BondMarketDataService();

    // Get data on our service given a key
    virtual OrderBook<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(OrderBook<T> &data)  override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<OrderBook<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<OrderBook<T>>* >& GetListeners() const override;

    // Get the best bid/offer order
    virtual BidOffer GetBestBidOffer(const string &productId) ;

    // Aggregate the order book
    virtual const OrderBook<T>& AggregateDepth(const string &productId)override;

};

template<typename T>
class BondMarketDataServiceConnector: public Connector<OrderBook<T>>{
private:
        BondMarketDataServiceConnector<T>* bond_market_data_service;
        BondProductService* bond_product_service;
public:
        BondMarketDataServiceConnector(BondMarketDataService<T>* market_service, BondProductService* product_service);
        ~BondMarketDataServiceConnector();
        virtual void Publish(OrderBook<T>& data) override {};
        void Subscribe(ifstream& data);
};





Order::Order(double _price, long _quantity, PricingSide _side)
{
  price = _price;
  quantity = _quantity;
  side = _side;
}

double Order::GetPrice() const
{
  return price;
}
 
long Order::GetQuantity() const
{
  return quantity;
}
 
PricingSide Order::GetSide() const
{
  return side;
}

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
  bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
  return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
  return offerOrder;
}

template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
  product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
  return product;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const
{
  return bidStack;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const
{
  return offerStack;
}




template<typename T>
BondMarketDataService<T>::BondMarketDataService(){
    order_map = map<string, OrderBook<T> >();

}

// Get data on our service given a key
template<typename T>
OrderBook<T>& BondMarketDataService<T>::GetData(string key) {
    return order_map[key];
}

// The callback that a Connector should invoke for any new or updated data
template<typename T>
void BondMarketDataService<T>::OnMessage(OrderBook<T> &data) {
    string key = data.GetProduct().GetProductId();
    //if key in order_map
    if(order_map.find(key) !=order_map.end())
        order_map.erase(key);
    order_map.insert(pair<string, OrderBook<T> >(key,data));

    auto bestOrder=GetBestBidOffer(key);
    vector<Order> bid,ask;
    bid.push_back(bestOrder.GetBidOrder());
    ask.push_back(bestOrder.GetOfferOrder());
    OrderBook<T> best_bid_ask_book=OrderBook<Bond>(data.GetProduct(),bid,ask);

    for(auto& i:listeners){
        i->ProcessAdd(best_bid_ask_book);
    }
}

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondMarketDataService<T>::AddListener(ServiceListener<OrderBook<T>> *listener){
    listeners.push_back(listener);
}

// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<OrderBook<T>>* >& BondMarketDataService<T>::GetListeners() const{
    return listeners;
}

// Get the best bid/offer order
template<typename T>
BidOffer BondMarketDataService<T>::GetBestBidOffer(const string &productId) {
    auto order_book = order_map[productId];
    auto bid_stack = order_book.GetBidStack(); auto ask_stack = order_book.GetOfferStack();
    auto best_bid = bid_stack[0]; auto best_ask = ask_stack[0];
    for(auto& i:ask_stack) {
        if (i.GetPrice() < best_ask.GetPrice()) {
            best_ask = i;
        }
    }
    for(auto& i:bid_stack) {
        if(i.GetPrice() > best_bid.GetPrice()){
            best_bid = i;
        }
    }
    return BidOffer(best_bid, best_ask);

}

// Aggregate the order book
template<typename T>
const OrderBook<T>& BondMarketDataService<T>::AggregateDepth(const string &productId){
    OrderBook<T> order_book = order_map[productId];
    auto bid_stack = order_book.GetBidStack(); auto ask_stack = order_book.GetOfferStack();
    auto best_bid = bid_stack[0]; auto best_ask = ask_stack[0];
    unordered_map<double, long> bid_info, ask_info;
    vector<Order> new_bid_stack, new_ask_stack;
    //update ask depth
    for(auto& i:ask_stack) {
        if (ask_info.find(i.GetPrice()!=ask_info.end())) {
            ask_info[i.GetPrice()] += i.GetQuantity();
        }else{
            ask_info[i.GetPrice()] = i.GetQuantity();
        }
    }
    //save result
    for(auto& i:ask_stack){
        new_ask_stack.push_back(Order(i.first, i.second, OFFER));
    }
    //update bid depth
    for(auto& i:bid_stack) {
        if (bid_info.find(i.GetPrice()!=bid_info.end())) {
            bid_info[i.GetPrice()] += i.GetQuantity();
        }else{
            bid_info[i.GetPrice()] = i.GetQuantity();
        }
    }
    //save result
    for(auto& i:bid_stack){
        new_bid_stack.push_back(Order(i.first, i.second, BID));
    }
    return OrderBook<T>(order_book.GetProduct(), new_bid_stack, new_ask_stack);

}

////convert input data to price
//double transform_data_to_price(string& s) {
//    double ans;
//    int len = s.size();
//    if (s[len - 1] == '+') {
//        s[len - 1] = '4';
//    }
//    ans = stoi(s.substr(0, len - 4)) + stoi(s.substr(len - 3, 2))/32.0 + stoi(s.substr(len - 1,1))/256.0;
//
//    return ans;
//}



template<typename T>
BondMarketDataServiceConnector<T>::BondMarketDataServiceConnector(BondMarketDataService<T>* market_service, BondProductService* product_service) {
    bond_market_data_service = market_service;
    bond_product_service = product_service;
}

template<typename T>
BondMarketDataServiceConnector<T>::~BondMarketDataServiceConnector(){}


template<typename T>
void BondMarketDataServiceConnector<T>::Subscribe(ifstream& data){
    //generate marketdata.txt
    //--------
    //code | price |  num | direction
    //--------

    string info;
    vector<Order> bid_stack,ask_stack;
    while(getline(data, info)){
        stringstream info_stream(info);
        vector<string> vec_s;
        string s;
        while(getline(info_stream,s,',')){
            vec_s.push_back(s);
        }
        string bond_code = vec_s[0];
        double price = transform_data_to_price(vec_s[1]);
        long num = stol(vec_s[2]);
        auto direction = vec_s[3];
        auto bond = bond_product_service->GetData(bond_code);

        if (direction =="BID"){
            bid_stack.push_back(Order(price,num,BID));
        }else{
            ask_stack.push_back(Order(price,num,OFFER));
        }

        OrderBook<Bond> order_book(bond, bid_stack, ask_stack);
        bond_market_data_service -> OnMessage(order_book);
    }
}
#endif



