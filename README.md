# Arduino BLE iOS C++
This simple example shows a cocos2d-x C++ on desktop, iPhone, or iPad program communicating with an Arduino via Bluetooth BLE 4.0 HC-10. Also known as Bluetooth Low Energy.

In this example I have boiled away everything except for that code required to open up a serial TX RX pipe to a BLE v4.0 module. 

>BLE is very very different than the typical Bluetooth modules such as the HC-05/HC-06. With those you establish a connection in a "traditional" way through the bluetooth setting on your machine. Then you just talk to that particular port and all will work. 
>
>The problem with that technology is that it requires two things. One is a bothersome connection, and the other is that it gobbles energy.
>
>With BLE the paradigm is very different. Each device just sort of exists waiting to be polled. It then offers up different services which can also be polled. Then you sort of subscribe to that service and events happen and whatnot. These things you subscribe to are called "characteristics".
>
>The classic device that people talk about is the heartbeat sensor. Obviously communications with such a device would be pretty one way. So you would subscribe to its heartbeat reports which it would then shout out when it felt like it. There are many sensors that are classic to the BLE such as temperature, etc. 
>
>The amount of energy that a BLE uses is so low that if properly designed into a device you could be looking at using a coin cell. 
>
>The core BLE module actually has SPI and 10 I/O pins. This would allow you to use it very much on its own. It can also be independently programmed so again it could be a capable core to an embedded system. For this tutorial we are just setting it up as s dumb pipe via the TX & RX service.

The default name for the module as delivered is typically C2541-A. This can be changed by sending an AT command

    AT+NAMEDONOVAN
Would change the module to DONOVAN. 

If you are using a real module you can send the AT command with the "No line ending" setting at 9600 baud in the Arduino Serial Monitor. If you are using a knockoff you will need to set it to "Carriage Return" before typing the AT commands. AT commands can be tested by just typing AT and hitting enter. It will say OK if you got it.

Cocos2d-x is not really nessessary and all the important code is in the files: **BLEComm.h** and **BLEComm.cpp**

I used Cocos2d-x because it is a great way to do C++ on the iPhone/iPad. 

>Quick plug for Cocos2d-x. I very much like it. Openframeworks.cc is quite good as well.
>
>Both of them allow you to easily write C++ code that is multi-platform (Windows, Mac, Linux, iPhone, iPad, Android, plus more...) With cocos2d-x you can generally access most features of the mobile devices but not yet bluetooth. 

To open up a pipe all that is needed is to instantiate a BLEComm object, setup the two callbacks, connect to a named module, and then start sending and receiving data.

So:

    BLEComm ble;
    ble.setStatusCallback(std::bind(&MainScene::StatusChanged, this, _1));
    ble.setDataCallback(std::bind(&MainScene::DataIn, this, _1));
    ble.Connect("DONOVAN");//To connect to a module named donovan
    ble.Send("HELLO\n");

And when you are done, you disconnect:

    ble.DisConnect();

###Arduino
The Arduino code is very simple. It is a Software serial connection to the RX and TX pins on the BLE module. The connection is just bouncing the monitor serial to the bluetooth and watching for the HELLO and GOODBYE commands from the bluetooth.

When it gets a command that is newline terminated it will see if it is one of the two. For HELLO it will turn the light on, and GOODBYE will turn the light off.

###The Circuit
There are 4 parts to the circuit:

* The first is the Arduino Nano. 
* Bluetooth BLE v4 HC-10
* FQP30n06L MOSFET
* APA104 (used as an LED).

Obviously the Nano and BLE module are connected via TX and RX(pins 3, 4), the MOSFET controls the power going to the APA104 and has its gate controlled by Pin 9. 

The reason I added the MOSFET(FQP30n06L) was that if I directly controlled an LED I could use just pin 9 plus a resistor which would be simpler. I had the APA104 handy and it is very bright. But it draws too much current for an Arduino pin(>40ma). Thus, I used a MOSFET. That same MOSFET could just as easily be controlling a fairly robust motor or other circuit. Also I could have controlled the APA104 via its colour data. But that would have added even more unnecessary complexity. My suggestion is to kill the entire MOSFET and just use an LED and a resistor. The APA104 is just such a wonderfully bright light.

The App can be run on a desktop or an iOS device. There are 4 crude green buttons labeled, CONN, ON, OFF, DISCON. 

They connect, turn light on, turn light off, and disconnect.

The text in the top will reflect the status or the data coming in.

>If you have any questions just email me:
>
>donovan( the at symbol )LessonStudio.com


*If you use this code then it would be nice to credit me and throw me a Github star.*
