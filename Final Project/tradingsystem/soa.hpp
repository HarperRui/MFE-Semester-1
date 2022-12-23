/**
 * soa.hpp
 * Definition of our Service Oriented Architecture (SOA) Service base class
 *
 * @author Breman Thuraisingham
 */

#ifndef SOA_HPP
#define SOA_HPP

#include <vector>
#include "products.hpp"
#include <map>

using namespace std;

/**
 * Definition of a generic base class ServiceListener to listen to add, update, and remve
 * events on a Service. This listener should be registered on a Service for the Service
 * to notify all listeners for these events.
 */
template<typename V>
class ServiceListener
{

public:

  // Listener callback to process an add event to the Service
  virtual void ProcessAdd(V &data) = 0;

  // Listener callback to process a remove event to the Service
  virtual void ProcessRemove(V &data) = 0;

  // Listener callback to process an update event to the Service
  virtual void ProcessUpdate(V &data) = 0;

};

/**
 * Definition of a generic base class Service.
 * Uses key generic type K and value generic type V.
 */
template<typename K, typename V>
class Service
{

public:

  // Get data on our service given a key
  virtual V& GetData(K key) = 0;

  // The callback that a Connector should invoke for any new or updated data
  virtual void OnMessage(V &data) = 0;

  // Add a listener to the Service for callbacks on add, remove, and update events
  // for data to the Service.
  virtual void AddListener(ServiceListener<V> *listener) = 0;

  // Get all listeners on the Service.
  virtual const vector< ServiceListener<V>* >& GetListeners() const = 0;

};  

/**
 * Definition of a Connector class.
 * This will invoke the Service.OnMessage() method for subscriber Connectors
 * to push data to the Service.
 * Services can invoke the Publish() method on this Service to publish data to the Connector
 * for a publisher Connector.
 * Note that a Connector can be publisher-only, subscriber-only, or both publisher and susbcriber.
 */
template<typename V>
class Connector
{

public:

  // Publish data to the Connector
  virtual void Publish(V &data) = 0;

};



// Store all bond information
class BondProductService:public Service<string, Bond>{
private:
    map<string, Bond> bond_map;
public:
    //ctor
    BondProductService();

    vector<Bond> GetBond(string& bond_code);
    void AddBond(Bond &bond);

    // Get data on our service given a key
    Bond& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Bond &data) override {};

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    void AddListener(ServiceListener<Bond> *listener) override {};

    // Get all listeners on the Service.
//    virtual const vector< ServiceListener<Bond>* >& GetListeners() const override{};

};





//ctor
BondProductService::BondProductService(){
    bond_map = map<string, Bond>();
}

vector<Bond> BondProductService::GetBond(string& bond_code){
    vector<Bond> vec;
    for(auto bond:bond_map){
        if(bond.second.GetTicker() == bond_code){
            vec.push_back(bond.second);
        }
    }
    return vec;
}

void BondProductService::AddBond(Bond &bond){
    bond_map.insert(pair<string, Bond>(bond.GetProductId(), bond));
}

// Get data on our service given a key
Bond& BondProductService::GetData(string key){
    return bond_map[key];
}

//const vector< ServiceListener<Bond>* >& BondProductService::GetListeners() const {
//    return;
//};
//convert input data to price
double transform_data_to_price(string& s) {
    double ans;
    int len = s.size();
    if (s[len - 1] == '+') {
        s[len - 1] = '4';
    }
    ans = stoi(s.substr(0, len - 4)) + stoi(s.substr(len - 3, 2))/32.0 + stoi(s.substr( len - 1,1))/256.0;

    return ans;
}

#endif
