/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Breman Thuraisingham
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP
#include <iostream>
#include "products.hpp"
#include "GUIService.h"
#include "positionservice.hpp"

enum ServiceType { POSITION, RISK, EXECUTION, STREAMING, INQUIRY };

using namespace std;

/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : Service<string,T>
{

public:

  // Persist data to a store
  void PersistData(string persistKey, const T& data) = 0;

};



template<typename T>
class BondHistoricalDataConnector;




template<typename T>
class BondHistoricalDataListener;




template<typename T>
class HistoricalPositionConnector: public Connector<Position<T> >{
public:
    // ctor
    HistoricalPositionConnector(){};
    virtual void Publish(Position<T> &data) override;
};

template<typename T>
void HistoricalPositionConnector<T>::Publish(Position<T> &data) {
    auto bond=data.GetProduct();
    auto position=data.GetAggregatePosition();

    ofstream out;
    out.open("positions.txt",ios::app);
    out<<bond.GetProductId()<<","<<position<<endl;
}





//template<typename T>
//void BondHistoricalPositionService<T>::PersistData(string persistKey, Position<T>& data) {
//    string bond_code=data.GetProduct().GetProductId();
//    if(data_map.find(bond_code)!=data_map.end())
//        data_map.erase(bond_code);
//    data_map.insert(pair<string,Position<T> >(bond_code,data));
//    connector->Publish(data);
//}



#endif
