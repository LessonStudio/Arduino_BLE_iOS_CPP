//
//  BLE.cpp
//  BLETest
//
//  Created by Donovan on 2016-02-20.
//
//

//Warning. If you have more than one module of the same name, then bad things will probably happen. A UUID would be a healthier way to do this.
#include "BLEComm.h"
#include <map>

//First we pack ALL the objective C here out of the way of the rest of the application.

//Each of the below functions pretty much operates in the order presented.
//At the bottom with have the function definitions for the C++ class.
//So, the bottom class is what the C++ will interact with and that will noodle with the ObjectiveC above.

//What is basically happening is that we scan for devices, get a callback when devices are found. If the named device is the one we want, we then
//connect to that device. Then we ask for services, we get a callback of the one we want and then store that "characteristic" for our serial style
//communications.

//BLE can do so much more than this. This is the most elemental and boiled down version of communicating with these basic BLE modules.

//For instance we don't need to have an arduino at all. The BLE module has a nice stack of pins that can be used to do pretty much most of
//the common IO features of arduino pins. It has SPI and 10 I/O pins. In theory you can program the module as it has a pile of memory but for
//this demonstration we are going to only use it as a very dumb pipe.

//To rename your module you will have to use your serial monitor to manually type:
//SET NAME AT+NAME=DONOVAN
//to change it to DONOVAN, for example.



#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
@interface BLE : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>//Our Objective-c class with its delegates
@property (strong, nonatomic) CBCentralManager* centralManager;//This is the big bahunga of core bluetooth
@property (strong, nonatomic) CBPeripheral* selected;//This is the module we care about.
@property (strong, nonatomic) NSMutableArray *peripherals;//This is part of the boiler plate of CoreBluetooth
@property (nonatomic) std::map<std::string,CBPeripheral*> teeth; //An associateive array of modules found
@property (nonatomic) std::string module_name;//This is the module we are looking for
@property(nonatomic) void *parent_function;
-(void) setParent:(void *)parent_function;//This is where we set the C++ function for our known callbacks
-(void) start:(std::string) module_name;//This is the module we are looking for so start looking for it and connect when found
-(void) connect;//Guess what this one does
-(void) disconnect;//Nobody knows what this does, or do we?
-(void) send:(std::string)out_data;//Send a string out the pipe
@end

@implementation BLE
@synthesize centralManager; //This is the core of corebluetooth
@synthesize teeth;//This is a associative array of the modules found.
@synthesize selected;//This is the module we want to connect to.
@synthesize module_name;//This is the name of the module we want to love
-(void)setParent:(void *)parent_function
{
	self.parent_function=parent_function;
}
-(void) start:(std::string) in_module_name;
{
	self.module_name=in_module_name;
	selected=NULL;
	self.centralManager=[[CBCentralManager alloc] initWithDelegate:self queue:nil];//This kicks the module into gear which will soon trigger centralManagerDidUpdateState
	self.peripherals=[[NSMutableArray alloc] init];
}
-(void)connect
{
	[self.centralManager stopScan];
	CBPeripheral *peripheral=teeth[self.module_name];
	if(peripheral==NULL)
	{
		((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::ERROR);
		return;
	}
	[self.centralManager connectPeripheral:peripheral options:nil];
}
-(void)centralManagerDidUpdateState:(CBCentralManager *)central
{
	if (central.state == CBCentralManagerStatePoweredOn)
	{
		[self.centralManager scanForPeripheralsWithServices:nil options:nil];//We can't start scanning until we have got the OK.
		//Interestingly enough on the desktop you can just start with this. On the iPhone and iPad you must wait until centralManagerDidUpdateState
	}
	else
	{
		((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::ERROR);
	}
}
-(void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
	//This is when some device is found.
	//It can be triggered repeatedly by the same device when still scanning.
	if(peripheral.name==NULL){return;}//In my house there is something else identifying as NULL and I don't know what it is.
	std::string found_module=std::string([peripheral.name UTF8String]);
	teeth[found_module]=peripheral;
	if(self.selected==NULL && found_module==self.module_name)
	{
		self.selected=peripheral;
		((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::FOUND_NAMED_MODULE);
	}
}
//These next functions all pretty much speak for themselves
-(void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
	((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::CONNECTED);
	[peripheral setDelegate:self];
	[peripheral discoverServices:nil];
}
-(void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error
{
	((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::DISCONNECTED);
}
-(void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error
{
	((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::ERROR);
}
//Just don't even look at these few. For a simple pipe just trust that they will work their magic.
-(void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
	for(int i = 0; i < peripheral.services.count; i++)
	{
		CBService * service = [peripheral.services objectAtIndex:i];
		[peripheral discoverCharacteristics:nil forService:service];
	}
}
- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
	for (CBCharacteristic * character in [service characteristics])
	{
		[self.selected discoverDescriptorsForCharacteristic:character];
	}
}
//The last two functions lead up to this one. It is where we are looking for the "FFE1" which is the TX RX capacity of the BLE module.
- (void)peripheral:(CBPeripheral *)peripheral didDiscoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
	std::string boop=[[[characteristic UUID] UUIDString] UTF8String];
	if(boop=="FFE1")
	{
		[self.selected setNotifyValue:true forCharacteristic:characteristic];
	}
}
- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
	if(error)
	{
		((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::ERROR);
	}
}
//This is fired off when data comes in from the module.
- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
	NSString* newStr = [[NSString alloc] initWithData:[characteristic value] encoding:NSUTF8StringEncoding];
	if(newStr==nil)//Getting strange characters can do this.
	{
		((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::ERROR);
		return;
	}
	std::string new_data=[newStr UTF8String];
	((BLEComm *)self.parent_function)->DataArrived(new_data);
	if(error)
	{
		((BLEComm *)self.parent_function)->SetStatus(BLEComm::STATUS::ERROR);
		return;
	}
}
-(void)disconnect
{
	CBPeripheral *peripheral=teeth[self.module_name];
	[self.centralManager cancelPeripheralConnection:peripheral];
}
//When we send data we just pump it out in a fire and forget fashion. There are ways to get a confirmation from the bluetooth.
-(void)send:(std::string)out_data
{
	for (CBService * service in [self.selected services])
	{
		for (CBCharacteristic * characteristic in [service characteristics])
		{
			NSString *str = [NSString stringWithCString:out_data.c_str() encoding:[NSString defaultCStringEncoding]];
			[self.selected writeValue:[str dataUsingEncoding:NSUTF8StringEncoding] forCharacteristic:characteristic type:CBCharacteristicWriteWithoutResponse];
		}
	}
}
@end

//Here we have the C++ class function definitions.
using namespace std;
//This both creates the objective 
void BLEComm::StartScanning()
{
	if(ble==nullptr)//If needed create a new Objective C BLE class.
	{
		ble=[[[BLE alloc] init] retain];
		[(BLE *)ble setParent_function:this];
	}
	[(BLE *)ble start:module_name];//Then start scanning which will triggger a connect
}
void BLEComm::Connect(string in_module_name)
{
	module_name=in_module_name;
	connect=true;
	StartScanning();//With connect==true the scan will connect if it finds the module.
}
void BLEComm::DisConnect()
{
	connect=false;//Don't trigger an automatic reconnect
	[(BLE *)ble disconnect];
}
void BLEComm::Send(string out_data)
{
	[(BLE *)ble send:out_data];//Fire and forget this string
}
void BLEComm::setStatusCallback(function<void(STATUS)> callback)
{
	status_callback=callback;
}
void BLEComm::SetStatus(STATUS status)
{
	if(status==FOUND_NAMED_MODULE && connect==true)
	{
		[(BLE *)ble connect];
	}
	if(status==STATUS::DISCONNECTED && connect==true)
	{
		StartScanning();
	}
	if(status_callback)
	{
		status_callback(status);//This all boils down to this function being fired if there is a status change
	}
}
void BLEComm::setDataCallback(function<void(string)> callback)
{
	data_callback=callback;
}
void BLEComm::DataArrived(string out_data)
{
	if(data_callback)
	{
		data_callback(out_data);
	}
}
/*
                         CMM
                      $MMMMM$
                    $MMMMMMM
                  $MMMMMMM  MMMMM
                $MMMMMMM  MMMMMMMMM
              $MMMMMMM  MMMMMMMMMMMMM
            $MMMMMMM  MMMMMMMMMMMMMMMMM
          $MMMMMMM  MMMMMMMMMMMMMMMMMMMMM
        $MMMMMMM  MMMMMMMMMMMM   MMMMMMMMMM
      $MMMMMMM     ?MMMMMMMMMMM    MMMMMMMMMM
    $MMMMMMM  MMMM   ?MMMMMMMMMMM    MMMMMMMMMM
  CMMMMMMM  MMMMMMM$   ?MMMMMMMMMMM    MMMMMMMMMM
  MMMMMMMM  MMMMMMMMMM    MMMMMMMMMMM    MMMMMMMM
    MMMMMMMM  MMMMMMMMMM   ?MMMMMMMMMMM    MMMM
      MMMMMMMM  MMMMMMMMMM   ?MMMMMMMMMMM
        MMMMMMMM  MMMMMMMMMM   MMMMMMMMMMMM
          MMMMMMMM  MMMMMMMMMMMMMMMMMMMMM
            MMMMMMMM  MMMMMMMMMMMMMMMMM
              MMMMMMMM  MMMMMMMMMMMMM
                MMMMMMMM  MMMMMMMMM
                  MMMMMMMM  MMMMM
                    MMMMMMMM
                      MMMMMMM
                        MMM
*/