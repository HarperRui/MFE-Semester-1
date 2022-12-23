/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"
//#include "BondAlgoExecutionService.h"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOrder
{

public:

  // ctor for an order
  ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the order ID
  const string& GetOrderId() const;

  // Get the order type on this order
  OrderType GetOrderType() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity
  long GetHiddenQuantity() const;

  // Get the parent order ID
  const string& GetParentOrderId() const;

  // Is child order?
  bool IsChildOrder() const;

  // Get the side
  PricingSide GetSide() const;

private:
  T product;
  PricingSide side;
  string orderId;
  OrderType orderType;
  double price;
  double visibleQuantity;
  double hiddenQuantity;
  string parentOrderId;
  bool isChildOrder;

};

/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class ExecutionService : public Service<string,ExecutionOrder <T> >
{

public:

  // Execute an order on a market
  void ExecuteOrder(const ExecutionOrder<T>& order, Market market) = 0;

};


template<typename T>
class AlgoExecution{
private:
    ExecutionOrder<T> execution_order;

public:
    // ctor
    AlgoExecution(ExecutionOrder<T> order);
    // Update the information
    void Run(OrderBook<T> order_book);
    // Get the execution order
    ExecutionOrder<T> GetExecutionOrder() const;

};






template<typename T>
class BondExecutionServiceConnector: public Connector<ExecutionOrder<T>>{
public:
    BondExecutionServiceConnector();
    virtual void Publish(ExecutionOrder<T>& data) override;
};

//The BondExecutionService does not need an input Connector
// since data should flow via ServiceListener from the BondAlgoExecutionService.
// Each execution should result in a trade into the BondTradeBookingService via ServiceListener on BondExectionService
// – cycle through the books above in order TRSY1, TRSY2, TRSY3.
// The BondExecutionService should use a Connector to publish executions via socket into a separate process
// which listens to the executions on the socket via its own Connector and prints them when it receives them.
template<typename T>
class BondExecutionService: public ExecutionService<T>{
private:
    map<string, ExecutionOrder<T>> execution_map;
    vector<ServiceListener<ExecutionOrder<T>>*> listeners;
    BondExecutionServiceConnector<T>* bond_execution_service_connector;
public:
    BondExecutionService(BondExecutionServiceConnector<T>* connector);
    // Get data on our service given a key
    virtual ExecutionOrder<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(ExecutionOrder<T> &data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<ExecutionOrder<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<ExecutionOrder<T>>* >& GetListeners() const override;

    void AddAlgoExecution(AlgoExecution<T>& algo);


};


template<typename T>
class BondExecutionServiceListener: public ServiceListener<AlgoExecution<T>>{
private:
    BondExecutionService<T>* bond_execution_service;
public:
    BondExecutionServiceListener(ExecutionService<T>* service);
    ~BondExecutionServiceListener();
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(AlgoExecution<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(AlgoExecution<T> &data)  override {};

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(AlgoExecution<T> &data) override {};
};

//template<typename T>
//class BondExecutionServiceConnector:public Connector<ExecutionOrder<T> >{
//public:
//    // ctor
//    BondExecutionServiceConnector(){};
//    virtual void Publish(ExecutionOrder<T>& data) override;
//    virtual void ExecuteOrder(ExecutionOrder<T>, Market market) override;
//
//};


template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
  product(_product)
{
  side = _side;
  orderId = _orderId;
  orderType = _orderType;
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  parentOrderId = _parentOrderId;
  isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
  return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
  return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
  return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
  return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
  return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
  return isChildOrder;
}


template<typename T>
PricingSide ExecutionOrder<T>::GetSide() const {
    return side;
}

//ctor
template<typename T>
AlgoExecution<T>::AlgoExecution(ExecutionOrder<T> order){
    execution_order = order;
}
// Update the information
template<typename T>
void AlgoExecution<T>::Run(OrderBook<T> order_book){
    auto bond=order_book.GetProduct();
    if(bond.GetProductId() != execution_order.GetProduct().GetProductId()) return;
    PricingSide pside;
    if(rand()%2==0){
        pside = BID;
    }else{
        pside=OFFER;
    }
    auto bid_order=order_book.GetBidStack().begin();
    auto ask_order=order_book.GetOfferStack().begin();

    double price;
    long visiable_num = 0,hidden_num;
    if(pside==BID){
        price=bid_order->GetPrice();
        if(ask_order->GetPrice()-bid_order->GetPrice()<1.5/128.0)
            visiable_num = bid_order->GetQuantity();
        hidden_num = 2 * visiable_num;
    }
    else{
        price = ask_order->GetPrice();
        if(ask_order->GetPrice()-bid_order->GetPrice()<1.5/128.0)
            visiable_num = ask_order->GetQuantity();
        hidden_num= 2 * visiable_num;
    }
    execution_order = ExecutionOrder<T>(bond,pside,"orderID",LIMIT,price,visiable_num,hidden_num,"parentID",true);
}

// Get the execution order
template<typename T>
ExecutionOrder<T> AlgoExecution<T>::GetExecutionOrder() const{
    return execution_order;
}


template<typename T>
BondExecutionServiceConnector<T>::BondExecutionServiceConnector(){};

template<typename T>
void BondExecutionServiceConnector<T>::Publish(ExecutionOrder<T>& data) {
    T bond = data.GetProduct();
    string side;
    chrono::milliseconds time = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
    cout<<time.count()<<", "<< bond.GetProduct().GetProductID()<<", "<<data.GetPrice() <<", "<<data.GetSide()<<
    ", "<<data.GetVisibleQuantity()<<", "<<data.GetHiddenQuantity()<<endl;
}


template<typename T>
BondExecutionService<T>::BondExecutionService(BondExecutionServiceConnector<T>* connector){
    bond_execution_service_connector = connector;
}
    // Get data on our service given a key
template<typename T>
ExecutionOrder<T>& BondExecutionService<T>::GetData(string key) {
    return execution_map[key];
}

template<typename T>
void BondExecutionService<T>::OnMessage(ExecutionOrder<T> &data) {
    execution_map[data.GetProduct().GetProductID()] = data;
}

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondExecutionService<T>::AddListener(ServiceListener<ExecutionOrder<T>> *listener) {
    listeners.push_back(listener);
}

template<typename T>
void BondExecutionService<T>::AddAlgoExecution(AlgoExecution<T>& algo){
    auto execution_order = algo.GetExecutionOrder();
    string bond_code = execution_order.GetProduct().GetProductId();

    if(execution_map.find(bond_code)!=execution_map.end())
        execution_map.erase(bond_code);
    execution_map.insert(pair<string,ExecutionOrder<T> >(bond_code,execution_order));

    for(auto& i:listeners) {
        i->ProcessAdd(execution_order);
    }


}




// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<ExecutionOrder<T>>* >& BondExecutionService<T>::GetListeners() const {
    return listeners;
};



template<typename T>
BondExecutionServiceListener<T>::BondExecutionServiceListener(ExecutionService<T>* service) {
    bond_execution_service = service;
}

template<typename T>
BondExecutionServiceListener<T>::~BondExecutionServiceListener(){}

// Listener callback to process an add event to the Service
template<typename T>
void BondExecutionServiceListener<T>::ProcessAdd(AlgoExecution<T> &data) {
    ExecutionOrder<T>* execution_order = data.GetExecutionOrder();
    bond_execution_service->OnMessage(*execution_order); //***
    bond_execution_service->ExecuteOrder(*execution_order);
}
//template<typename T>
//void BondExecutionServiceConnector<T>::Publish(ExecutionOrder<Bond>& data){
//    auto bond = data.GetProduct();
//    cout<<bond.GetProductId()<<", "<<data.GetPrice()<<endl;
//
//}
//template<typename T>
//void BondExecutionService<T>::ExecuteOrder(ExecutionOrder<Bond> &order, Market market){
//    bond_execution_service_connector->Publish(order);
//}

#endif
