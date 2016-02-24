//
//  BLE.hpp
//  BLETest
//
//  Created by Donovan on 2016-02-20.
//
//

#ifndef BLE_hpp
#define BLE_hpp
//Why am I not using namespace std? I don't like putting it in commonly used headers that then force everybody to use it. It has caused fights in the past.
class BLEComm
{
public:
	enum STATUS
	{
		NOTHING,
		FOUND_NAMED_MODULE,
		CONNECTED,
		DISCONNECTED,
		ERROR
	};
private:
	void* ble=nullptr;//Let's keep objective C out of the header.
	std::string module_name;//This is the name of the module that we will be looking for. A UUID is better but I am keeping this simple.
	bool connect=false;//This sets if a found module should be connected to.
	void StartScanning();
	std::function<void(STATUS)> status_callback;
	std::function<void(std::string)> data_callback;
public:
	void Connect(std::string module_name);
	void DisConnect();
	void SetStatus(STATUS);//Delegate function
	void DataArrived(std::string out_data);//Delegate function
	void Send(std::string out_data);
	void setStatusCallback(std::function<void(STATUS)> callback);//Set your status changed function here
	void setDataCallback(std::function<void(std::string)> callback);//Set your data came in function here.
};

#endif /* BLE_hpp */
