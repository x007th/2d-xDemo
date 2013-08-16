//
//  GameHUD.h
//  MyTowerDefense2D
//
//  Created by su xinde on 13-6-8.
//
//

#ifndef __MyTowerDefense2D__GameHUD__
#define __MyTowerDefense2D__GameHUD__

#include "cocos2d.h"

class GameHUD : public cocos2d::CCLayer {
public:
	~GameHUD();
	bool init();
    
	bool ccTouchBegan(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
	void ccTouchMoved(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
	void ccTouchEnded(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
    
	static GameHUD* sharedHUD();
    
	void updateBaseHp(int amount);
	void updateResourcesNom();
	void updateResources(int amount);
	void updateWaveCount();
    
    
	CC_SYNTHESIZE(int, resources, Resources);
	CC_SYNTHESIZE(float, baseHpPercentage, BaseHpPercentage);
    
protected:
	cocos2d::CCSprite* background;
	cocos2d::CCSprite* selSpriteRange;
	cocos2d::CCSprite* selSprite;
	cocos2d::CCArray* movableSprites;
    
	cocos2d::CCLabelTTF* resourceLabel;
	cocos2d::CCLabelTTF* waveCountLabel;
    
	cocos2d::CCProgressTimer* healthBar;
};


#endif /* defined(__MyTowerDefense2D__GameHUD__) */
