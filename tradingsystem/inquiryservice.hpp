/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "tradebookingservice.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

  // ctor for an inquiry
  Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);

  // Get the inquiry ID
  const string& GetInquiryId() const;

  // Get the product
  const T& GetProduct() const;

  // Get the side on the inquiry
  Side GetSide() const;

  // Get the quantity that the client is inquiring for
  long GetQuantity() const;

  // Get the price that we have responded back with
  double GetPrice() const;

  // Get the current state on the inquiry
  InquiryState GetState() const;

private:
  string inquiryId;
  T product;
  Side side;
  long quantity;
  double price;
  InquiryState state;

};

/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{

public:

  // Send a quote back to the client
  void SendQuote(const string &inquiryId, double price) = 0;

  // Reject an inquiry from the client
  void RejectInquiry(const string &inquiryId) = 0;

};

template<typename T>
class BondInquiryService:public InquiryService<T>{
private:
    map<string, Inquiry<T>> inquiry_map;
    vector<ServiceListener<Inquiry<T>>*> listeners;

public:
    BondInquiryService();
    // Get data on our service given a key
    virtual Inquiry<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(Inquiry<T> &data)  override;

    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<Inquiry<T>> *listener) override;

    // Get all listeners on the Service.
    virtual const vector< ServiceListener<Inquiry<T>>* >& GetListeners() const override;

    // Send a quote back to the client
    void SendQuote(const string &inquiryId, double price) override {};

    // Reject an inquiry from the client
    void RejectInquiry(const string &inquiryId) override {};
};

template<typename T>
class BondInquiryConnector: public Connector<Inquiry<T>>{
private:
    BondInquiryService<T>* bond_inquiry_service;
    BondProductService* bond_product_service;
public:
    BondInquiryConnector(BondInquiryService<T>* inquiry_service, BondProductService* product_service);
    virtual void Publish(Inquiry<T>& data) override {};
    void Subscribe(ifstream& data);
};







template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
  product(_product)
{
  inquiryId = _inquiryId;
  side = _side;
  quantity = _quantity;
  price = _price;
  state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
  return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
  return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
  return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
  return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
  return state;
}


template<typename T>
BondInquiryService<T>::BondInquiryService(){
    inquiry_map = map<string, Inquiry<T>>();
}
// Get data on our service given a key
template<typename T>
Inquiry<T>& BondInquiryService<T>::GetData(string key){
    return inquiry_map[key];
}

// The callback that a Connector should invoke for any new or updated data
template<typename T>
void BondInquiryService<T>::OnMessage(Inquiry<T> &data) {
    data.SetState(data.GetPrice(), DONE);
    inquiry_map.insert(pair<string,Inquiry<T> >(data.GetInquiryId(),data));
    for(auto& i:listeners) {
        i->ProcessAdd(data);
    }
}

// Add a listener to the Service for callbacks on add, remove, and update events
// for data to the Service.
template<typename T>
void BondInquiryService<T>::AddListener(ServiceListener<Inquiry<T>> *listener){
    listeners.push_back(listener);
}

// Get all listeners on the Service.
template<typename T>
const vector< ServiceListener<Inquiry<T>>* >& BondInquiryService<T>::GetListeners() const {
    return listeners;
}


template<typename T>
BondInquiryConnector<T>::BondInquiryConnector(BondInquiryService<T>* inquiry_service, BondProductService* product_service) {
    bond_product_service = product_service;
    bond_inquiry_service = inquiry_service;
}
template<typename T>
void BondInquiryConnector<T>::Subscribe(ifstream& data){
    //generate inquiries.txt
    //--------
    //code | price |  num | direction | RECEIVED
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
        double price = transform_data_to_price(vec_s[1]);
        long num = stol(vec_s[2]);
        string direction = vec_s[3];
        Side side;
        if (direction =="BUY"){
            side = BUY;
        }else{
            side = SELL;
        }
        InquiryState state = RECEIVED;
        auto bond = bond_product_service->GetData(bond_code);
        Inquiry<Bond> inquiry(bond_code, bond, side, num, price, state);
        bond_inquiry_service -> OnMessage(inquiry);
    }
}

#endif


