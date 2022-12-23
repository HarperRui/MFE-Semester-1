/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "marketdataservice.hpp"
//#include "BondAlgoStreamingService.h"

/**
 * A price stream order with price and quantity (visible and hidden)
 */
class PriceStreamOrder
{

public:

  // ctor for an order
  PriceStreamOrder() = default;
  PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

  // The side on this order
  PricingSide GetSide() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity on this order
  long GetHiddenQuantity() const;

private:
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  PricingSide side;

};

/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream
{

public:

  // ctor
  PriceStream(){};

  PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the bid order
  const PriceStreamOrder& GetBidOrder() const;

  // Get the offer order
  const PriceStreamOrder& GetOfferOrder() const;

private:
  T product;
  PriceStreamOrder bidOrder;
  PriceStreamOrder offerOrder;

};

/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> >
{

public:

  // Publish two-way prices
  void PublishPrice(const PriceStream<T>& priceStream) = 0;

};

template<typename T>
class AlgoStreaming{
private:
    PriceStream<T> price_stream;

public:
    AlgoStreaming(PriceStream<T>& stream);
    void Run(Price<T> price);
    PriceStream<T> GetPriceStreaming() const;

};


template<typename T>
class BondStreamingServiceConnector:public Connector<PriceStream<T>>{
public:
    BondStreamingServiceConnector()=default;
    virtual void Publish(PriceStream<T>& data) override;
};

template<typename T>
class BondStreamingService: public Service<string, StreamingService<T>>{
private:
    map<string, PriceStream<T>> stream_map;
    vector<ServiceListener<PriceStream<T>>*> listeners;
    BondStreamingServiceConnector<T>*  bond_streaming_service_connector;
public:
    BondStreamingService(){};
    BondStreamingService(BondStreamingServiceConnector<T>* connector);

    // Get data on our service given a key
    virtual PriceStream<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(PriceStream<T> &data)  override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<PriceStream<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<PriceStream<T>>* >& GetListeners() const override;

    void update_algo(AlgoStreaming<T> & algo);
    void PublishPrice(PriceStream<T> &data);

};


template<typename T>
class BondStreamingServiceListener: public ServiceListener<AlgoStreaming<T>>{
private:
    BondStreamingService<T>*  bond_stream_service;
public:
    BondStreamingServiceListener(BondStreamingService<T>* service);
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(AlgoStreaming<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(AlgoStreaming<T> &data) override{};

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(AlgoStreaming<T> &data) override{};

};

PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  side = _side;
}

double PriceStreamOrder::GetPrice() const
{
  return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
  return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder) :
  product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
  return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
  return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
  return offerOrder;
}

template<typename T>
PriceStream<T> AlgoStreaming<T>::GetPriceStreaming() const{
    return price_stream;
}




template<typename T>
BondStreamingService<T>::BondStreamingService(BondStreamingServiceConnector<T>* connector){
    stream_map = map<string, PriceStream<T>>();
    bond_streaming_service_connector = connector;
}

// Get data on our service given a key
template<typename T>
PriceStream<T>& BondStreamingService<T>::GetData(string key){
    return stream_map[key];
}

// The callback that a Connector should invoke for any new or updated data
template<typename T>
void BondStreamingService<T>::OnMessage(PriceStream<T> &data) {};

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondStreamingService<T>::AddListener(ServiceListener<PriceStream<T>> *listener) {
    listeners.push_back(listener);
}

// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<PriceStream<T>>* >& BondStreamingService<T>::GetListeners() const{
    return listeners;
}
template<typename T>
void BondStreamingService<T>::update_algo(AlgoStreaming<T> & algo){
    auto pstream = algo.GetPriceStream();
    string bond_code =pstream.GetProduct().GetProductId();
    if(stream_map.find(bond_code)!=stream_map.end())
        stream_map.erase(bond_code);
    stream_map.insert(pair<string,PriceStream<T> >(bond_code,pstream));
    for(auto& i:listeners){
        i->ProcessAdd(pstream);
    }
}

template<typename T>
void BondStreamingService<T>::PublishPrice(PriceStream<T> &data){
    bond_streaming_service_connector->Publish(data);
}


template<typename T>
void BondStreamingServiceConnectorPublish(PriceStream<T>& data) {
    auto bond = data.GetProduct();
    auto bid_order = data.GetBidOrder();
    auto ask_order = data.GetOfferOrder();

    cout << bond.GetProductId()<<", Bid_Order: "<< bid_order.GetPrice()<<bid_order.GetVisibleQuantity() << bid_order.GetHiddenQuantity()<<
    ", Ask_Order: "<<ask_order.GetPrice()<<ask_order.GetVisibleQuantity() <<ask_order.GetHiddenQuantity()<<endl;
    cout<<"-----------------"<<endl;

}


template<typename T>
BondStreamingServiceListener<T>::BondStreamingServiceListener(BondStreamingService<T>* service){
    bond_stream_service = service;
}
// Listener callback to process an add event to the Service
template<typename T>
void BondStreamingServiceListener<T>::ProcessAdd(AlgoStreaming<T> &data) {
    bond_stream_service->update_algo(data);
    auto ps = data.GetPriceStreaming();
    bond_stream_service->PublishPrice(ps);
}

#endif
