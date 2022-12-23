/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
using namespace std;
/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:

  // ctor for a price
  Price() = default;
  Price(const T &_product, double _mid, double _bidOfferSpread);

  // Get the product
  const T& GetProduct() const;

  // Get the mid price
  double GetMid() const;

  // Get the bid/offer spread around the mid
  double GetBidOfferSpread() const;

private:
  const T& product;
  double mid;
  double bidOfferSpread;

};

template<typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) :
        product(_product)
{
    mid = _mid;
    bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
    return product;
}

template<typename T>
double Price<T>::GetMid() const
{
    return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
    return bidOfferSpread;
}

////convert input data to price
//double transform_data_to_price(string& s) {
//    double ans;
//    int len = s.size();
//    if (s[len - 1] == '+') {
//        s[len - 1] = '4';
//    }
//    ans = stoi(s.substr(0, len - 4)) + stoi(s.substr(len - 3, 2))/32.0 + stoi(s.substr( len - 1,1))/256.0;
//
//    return ans;
//}


template<typename T>
class BondPricingConnector;


/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string,Price <T> >
{
private:
    map<string, Price<T>> price_map;
    vector<ServiceListener<Price<T>>*> listeners;
    BondPricingConnector<T>* bond_pricing_connector;
public:
    PricingService();
    ~PricingService();
    // Get data on our service given a key
    Price<T>& GetData(string key) = 0;

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Price<T> &data);

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<Price<T>> *listener);

    // Get all listeners on the Service.
    const vector< ServiceListener<Price<T>>* >& GetListeners() const;
    // Get its connector
    BondPricingConnector<T>* GetConnector();
};

template<typename T>
PricingService<T>::PricingService(){
    price_map = map<string, Price<T>>();
    listeners = vector<ServiceListener<Price<T>>*>();
    bond_pricing_connector = new BondPricingConnector<T>(this);
}

template<typename T>
PricingService<T>::~PricingService(){}

template<typename T>
Price<T>& PricingService<T>::GetData(string key){
    return price_map[key];
}

template<typename T>
void PricingService<T>::OnMessage(Price<T>& bond_data)
{
    price_map[bond_data.GetProduct()] = bond_data;

    for (auto& l : listeners)
    {
        l->ProcessAdd(bond_data);
    }
}

template<typename T>
void PricingService<T>::AddListener(ServiceListener<Price<T>>* listener)
{
    listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<Price<T>>*>& PricingService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
BondPricingConnector<T>* PricingService<T>::GetConnector()
{
    return bond_pricing_connector;
}







template<typename  T>
class BondPricingConnector: public Connector<Price<T>>{
private:
    PricingService<T>* price_service;
public:
    BondPricingConnector();
    BondPricingConnector(PricingService<T>* service);
    ~BondPricingConnector();
    //Publish() method on the Connector publishes data to the connectivity source and can be invoked from a Service
    void Publish(Price<T>& data);
    void Subscribe(ifstream& data);
};

template<typename T>
BondPricingConnector<T>::BondPricingConnector(PricingService<T>* service){
    price_service = service;
}

template<typename T>
BondPricingConnector<T>::~BondPricingConnector(){}


template<typename T>
void BondPricingConnector<T>::Publish(Price<T>& data) {}

template<typename T>
void BondPricingConnector<T>::Subscribe(ifstream& data) {
    string info;
    while(getline(data, info)){
        stringstream info_stream(info);
        vector<string> vec_s;
        string s;
        while(getline(info_stream,s,',')){
            vec_s.push_back(s);
        }
        string bond_code = vec_s[0];
        double price = transform_data_to_price(vec_s[1]);
        double spread =transform_data_to_price(vec_s[2]);

        Price<T> _price(bond_code, price, spread);
        price_service->OnMessage(_price);

    }
}






#endif
