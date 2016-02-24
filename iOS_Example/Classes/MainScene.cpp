#include "MainScene.h"
#include "BLEComm.h"

USING_NS_CC;
#define CONNECT 1
#define DISCONNECT 2
#define LIGHT_ON 3
#define LIGHT_OFF 4
using namespace std;
Scene* MainScene::createScene()
{
    auto scene = Scene::create();
    auto layer = MainScene::create();
    scene->addChild(layer);
    return scene;
}

bool MainScene::init()
{
	if (!Layer::init()){return false;}
	Size visibleSize = Director::getInstance()->getVisibleSize();

	//Create the four buttons.
	auto connect_button = Sprite::create("button.png");
	connect_button->setPosition(Vec2(100, 100));
	connect_button->setTag(CONNECT);
	{
		auto label=Label::createWithTTF("CONN", "fonts/Marker Felt.ttf", 24);
		label->setPosition(Vec2(connect_button->getContentSize().width/2-8, connect_button->getContentSize().height/2+8));
		connect_button->addChild(label);
	}
	this->addChild(connect_button);

	auto light_on_button=Sprite::create("button.png");
	light_on_button->setPosition(Vec2(200, 100));
	light_on_button->setTag(LIGHT_ON);
	{
		auto label=Label::createWithTTF("ON", "fonts/Marker Felt.ttf", 24);
		label->setPosition(Vec2(light_on_button->getContentSize().width/2-8, light_on_button->getContentSize().height/2+8));
		light_on_button->addChild(label);
	}
	this->addChild(light_on_button);

	auto light_off_button = Sprite::create("button.png");
	light_off_button->setPosition(Vec2(300, 100));
	light_off_button->setTag(LIGHT_OFF);
	{
		auto label=Label::createWithTTF("OFF", "fonts/Marker Felt.ttf", 24);
		label->setPosition(Vec2(light_off_button->getContentSize().width/2-8, light_off_button->getContentSize().height/2+8));
		light_off_button->addChild(label);
	}
	this->addChild(light_off_button);

	auto disconnect_button = Sprite::create("button.png");
	disconnect_button->setPosition(Vec2(400, 100));
	disconnect_button->setTag(DISCONNECT);
	{
		auto label=Label::createWithTTF("DISCON", "fonts/Marker Felt.ttf", 24);
		label->setPosition(Vec2(disconnect_button->getContentSize().width/2-8, disconnect_button->getContentSize().height/2+8));
		disconnect_button->addChild(label);
	}
	this->addChild(disconnect_button);
	
	//Create the listeners for the four buttons
	EventListenerTouchOneByOne *listener=cocos2d::EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan=[=](Touch* touch, Event* event)
	{
		return this->onTouchStart(touch, event);
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, connect_button);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener->clone(), light_on_button);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener->clone(), light_off_button);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener->clone(), disconnect_button);

	status=Label::createWithTTF("Status", "fonts/Marker Felt.ttf", 24);
	status->setPosition(Vec2(visibleSize.width/2, visibleSize.height - status->getContentSize().height));
	this->addChild(status, 1);
	scheduleUpdate();
	//Here is where we set the two callbacks for status and data in
	using namespace std::placeholders; //Some esoteric c++ to enable the _1
	ble.setStatusCallback(std::bind(&MainScene::StatusChanged, this, _1));
	ble.setDataCallback(std::bind(&MainScene::DataIn, this, _1));
	return true;
}
bool MainScene::onTouchStart(cocos2d::Touch* touch, cocos2d::Event* event)
{
	//If any of the four buttons are clicked then act upon them.
	auto target = static_cast<Node*>(event->getCurrentTarget());
	Point locationInNode=target->convertToNodeSpace(touch->getLocation());
	Size s=target->getContentSize();
	Rect rect=Rect(0, 0, s.width, s.height);
	if(rect.containsPoint(locationInNode))
	{
		if(target->getTag()==CONNECT)
		{
			status->setString("Scanning...");
			ble.Connect("CC41-A");//We are trying to find and connect to a device with this name. CC41-A is the default ID found on these devices.
		}
		else if(target->getTag()==LIGHT_ON)
		{
			ble.Send("HELLO\n");//The arduino is expecting a HELLO or GOODBYE command terminated with a newline
		}
		else if(target->getTag()==LIGHT_OFF)
		{
			ble.Send("GOODBYE\n");//The arduino is expecting a HELLO or GOODBYE command terminated with a newline
		}
		else if(target->getTag()==DISCONNECT)
		{
			ble.DisConnect();//Can you guess what this does?
		}
		return true;
	}
	return false;
}
void MainScene::DataIn(string in_data)
{
	status->setString(in_data);
}
void MainScene::StatusChanged(BLEComm::STATUS in_status)
{
	switch(in_status)
	{
		case BLEComm::STATUS::CONNECTED:
			status->setString("Connected");
			break;
		case BLEComm::STATUS::DISCONNECTED:
			status->setString("Disconnected");
			break;
		case BLEComm::STATUS::ERROR:
			status->setString("ERROR");
			break;
		case BLEComm::STATUS::FOUND_NAMED_MODULE:
			status->setString("Found Module");
			break;
		case BLEComm::STATUS::NOTHING:
			status->setString("This should never happen!!!");
			break;
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