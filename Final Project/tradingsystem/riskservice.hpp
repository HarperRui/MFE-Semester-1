/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"
#include "./Data/Bond_info.h"

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{

public:

  // ctor for a PV01 value
  PV01(const T &_product, double _pv01, long _quantity);

  // Get the product on this PV01 value
  const T& GetProduct() const;

  // Get the PV01 value
  double GetPV01() const;

  // Get the quantity that this risk value is associated with
  long GetQuantity() const;

private:
  T product;
  double pv01;
  long quantity;

};

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{

public:

  // ctor for a bucket sector
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

private:
  vector<T> products;
  string name;

};

/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string,PV01 <T> >
{

public:

  // Add a position that the service will risk
  void AddPosition(Position<T> &position) = 0;

  // Get the bucketed risk for the bucket sector
  const PV01< BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T> &sector) const = 0;

};


template<typename T>
class BondRiskService: public RiskService<T>{
private:
    map<string, PV01<T>> pv_map;
    vector<ServiceListener<PV01<T>>*> listeners;
public:
    BondRiskService();
    // Get the bucketed risk for the bucket sector
    const PV01< BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T> &sector) const;

    // Get data on our service given a key
    virtual PV01<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(PV01<T> &data)  override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<PV01<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<PV01<T>>* >& GetListeners() const override;
};


template<typename T>
class BondRiskServiceListener:public ServiceListener<Position<T>>{
private:
    BondRiskService<T>* bond_risk_service;
public:
    BondRiskServiceListener(BondRiskService<T>* service);

    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(Position<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(Position<T> &data) override{};

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(Position<T> &data) override{};

};






template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) :
  product(_product)
{
  pv01 = _pv01;
  quantity = _quantity;
}

template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
  products(_products)
{
  name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
  return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
  return name;
}


template<typename T>
BondRiskService<T>::BondRiskService(){
    pv_map = map<string, PV01<T>>();
}

// Get the bucketed risk for the bucket sector
template<typename T>
const PV01< BucketedSector<T> >& BondRiskService<T>::GetBucketedRisk(const BucketedSector<T> &sector) const {
    BucketedSector<T> bond = sector;
    double total_pv01 = 0;
    long total_num = 1;
    vector<T> vec = sector.GetProducts();
    for(auto& i: vec){
        string code = i.GetProductId();
        total_pv01 += pv_map[code].GetPV01()*pv_map[code].GetQuantity();
    }
    return PV01<BucketedSector<T>>(bond, total_pv01, total_num);
}

// Get data on our service given a key
template<typename T>
PV01<T>& BondRiskService<T>::GetData(string key){
    return pv_map[key];
}

// The callback that a Connector should invoke for any new or updated data
template<typename T>
void BondRiskService<T>::OnMessage(PV01<T> &data){
    pv_map[data.GetProduct().GetProductId()] = data;
}

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondRiskService<T>::AddListener(ServiceListener<PV01<T>> *listener){
    listeners.push_back(listener);
}

// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<PV01<T>>* >& BondRiskService<T>::GetListeners() const {
    return listeners;
}


template<typename T>
BondRiskServiceListener<T>::BondRiskServiceListener(BondRiskService<T>* service) {
    bond_risk_service = service;
}

// Listener callback to process an add event to the Service
template<typename T>
void BondRiskServiceListener<T>::ProcessAdd(Position<T> &data) {
    bond_risk_service->AddPosition(data);
}



#endif
