#ifndef __MainScene_SCENE_H__
#define __MainScene_SCENE_H__

#include "cocos2d.h"
#include "BLEComm.h"
class MainScene : public cocos2d::Layer
{
private:
	cocos2d::Label *status;
	BLEComm ble;//This is the magical class that connects to the BLE

public:
	static cocos2d::Scene* createScene();

	virtual bool init();
	bool onTouchStart(cocos2d::Touch* touch, cocos2d::Event* event);
	void StatusChanged(BLEComm::STATUS);
	void DataIn(std::string in_data);
	CREATE_FUNC(MainScene);
};

#endif // __MainScene_SCENE_H__
