//
//  MainLayer.cpp
//  MyTowerDefense2D
//
//  Created by su xinde on 13-6-8.
//
//

#include "MainLayer.h"
#include "TileData.h"
#include "GameMediator.h"
#include "GameHUD.h"
#include "Tower.h"
#include "Enemy.h"
#include "Wave.h"
#include "ProjectTile.h"
#include "Lightning.h"

using namespace cocos2d;

MainLayer::~MainLayer(){
	CC_SAFE_RELEASE_NULL(tileDataArray);
}

CCScene* MainLayer::scene()
{
	CCScene * scene = NULL;
	do
	{
		// 'scene' is an autorelease object
		scene = CCScene::create();
		CC_BREAK_IF(! scene);
        
		// 'layer' is an autorelease object
		MainLayer *layer = MainLayer::create();
		CC_BREAK_IF(! layer);
        
		// add layer as a child to scene
		scene->addChild(layer);
        
		GameHUD* myGameHUD = GameHUD::sharedHUD();
		scene->addChild(myGameHUD, 2);
        
		GameMediator* m = GameMediator::sharedMediator();
		m->setGameLayer(layer);
		m->setGameHUDLayer(myGameHUD);
        
	} while (0);
    
	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool MainLayer::init()
{
	bool bRet = false;
	do
	{
		//////////////////////////////////////////////////////////////////////////
		// super init first
		//////////////////////////////////////////////////////////////////////////
        
		CC_BREAK_IF(! CCLayer::init());
        
		//////////////////////////////////////////////////////////////////////////
		// add your codes below...
		//////////////////////////////////////////////////////////////////////////
		this->setTouchEnabled(true);
        
		background = CCSprite::create("map.jpg");
		background->setAnchorPoint(ccp(0,0));
		background->setPosition(CCPointZero);
		this->addChild(background);
        
        
		createTileMap();
        
		gameHUD = GameHUD::sharedHUD();
        
		currentLevel = 0;
		addWaves();
        
		m_emitter = CCParticleRain::create();
		//m_emitter->retain();
		this->addChild(m_emitter, 10);
		m_emitter->setLife(4);
        
		m_emitter->setTexture( CCTextureCache::sharedTextureCache()->addImage("fire.png") );
        
		Lightning* l = Lightning::create(ccp(160,maxHeight), ccp(200, 20));
		l->setVisible(false);
		this->addChild(l, 1, 999);
		this->schedule(schedule_selector(MainLayer::strikeLight), 5.0f);
        
		this->schedule(schedule_selector(MainLayer::gameLogic), 0.1f);
        
		bRet = true;
	} while (0);
    
	return bRet;
}

void MainLayer::ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent){
	
}

void MainLayer::ccTouchesMoved(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent){
	CCTouch* pTouch = (CCTouch*)pTouches->anyObject();
	CCPoint touchPoint = pTouch->locationInView();
	touchPoint = CCDirector::sharedDirector()->convertToGL(touchPoint);
    
	CCPoint oldPoint = pTouch->previousLocationInView();
	oldPoint = CCDirector::sharedDirector()->convertToGL(oldPoint);
    
	CCPoint translation = ccpSub(touchPoint, oldPoint);
	CCPoint newPos = ccpAdd(this->getPosition(), translation);
	CCPoint oldPos = this->getPosition();
	this->setPosition(boundLayerPos(newPos));
    
	m_emitter->setPosition(ccpSub(m_emitter->getPosition(), ccpSub(boundLayerPos(newPos), oldPos)));
}

void MainLayer::ccTouchesEnded(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent){
    
}


CCPoint MainLayer::boundLayerPos(cocos2d::CCPoint newPos){
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	CCPoint retval = newPos;
	retval.x = MIN(retval.x, 0);
	retval.x = MAX(retval.x, -background->getContentSize().width+winSize.width);
	retval.y = MIN(50, retval.y);
	retval.y = MAX(-background->getContentSize().height+winSize.height, retval.y);
	return retval;
}

void MainLayer::createTileMap(){
	tileDataArray = CCArray::create();
	tileDataArray->retain();
    
    maxWidth = background->getContentSize().width - TILE_WIDTH - TILE_WIDTH * 0.5f;
    maxHeight = background->getContentSize().height - TILE_HEIGHT * 2- TILE_HEIGHT * 0.5f;
    
	maxTileWidth = maxWidth / TILE_WIDTH;
	maxTileHeight = maxHeight / TILE_HEIGHT;
    
	for(int i = 0; i < maxTileWidth; i++){
		for(int j = 0; j < maxTileHeight; j++){
			TileData* td = TileData::create(ccp(i, j));
			tileDataArray->addObject(td);
		}
	}
}

CCPoint MainLayer::tileCoordForPosition(cocos2d::CCPoint position){
	int x, y;
	if(position.x < TILE_WIDTH){
		x = 0;
	}else if(position.x > maxWidth){
		x = maxTileWidth - 1;
	}else{
		x = (position.x - TILE_WIDTH) / TILE_WIDTH;
	}
    
	if(position.y < TILE_HEIGHT * 2){
		y = 0;
	}else if(position.y > maxHeight){
		y = maxTileHeight - 1;
	}else{
		y = (position.y - TILE_HEIGHT * 2) / TILE_HEIGHT;
	}
    
	return ccp(x, y);
}

CCPoint MainLayer::positionForTileCoord(cocos2d::CCPoint position){
	float x, y;
	x = TILE_WIDTH + position.x * TILE_WIDTH + TILE_WIDTH * 0.5f;
	y = TILE_HEIGHT * 2 + position.y * TILE_HEIGHT + TILE_HEIGHT * 0.5f;
    
	return ccp(x, y);
}

bool MainLayer::canBuildOnTilePosition(cocos2d::CCPoint pos){
	pos = ccpAdd(pos, ccp(0, 30));
	CCPoint towerLoc = this->tileCoordForPosition(pos);
    
	if(CCPoint::CCPointEqualToPoint(towerLoc, ccp(0, maxTileHeight / 2)) ||
       CCPoint::CCPointEqualToPoint(towerLoc, ccp(maxTileWidth  - 1, maxTileHeight / 2))){
        return false;
	}
    
	TileData* td = getTileData(towerLoc);
	if(td->getIsUsed() || td->getIsThroughing()){
		return false;
	}
    
	bool canBuild = false;
	for(int y =0; y < maxTileHeight; y++){
		if(y == towerLoc.y)
			continue;
		TileData* t = getTileData(ccp(towerLoc.x, y));
		if(t->getIsUsed() == false){
			canBuild = true;
			break;
		}
	}
    
	return canBuild;
}

TileData* MainLayer::getTileData(CCPoint tileCoord){
	CCObject* temp;
	CCARRAY_FOREACH(tileDataArray, temp){
		TileData* td = (TileData*)temp;
		if(CCPoint::CCPointEqualToPoint(td->getPosition(), tileCoord)){
			return td;
		}
	}
	return NULL;
}

void MainLayer::addTower(CCPoint pos, int towerTag){
	pos = ccpAdd(pos, ccp(0, 30));
	GameMediator* m = GameMediator::sharedMediator();
	Tower* target = NULL;
	CCPoint towerLoc = this->tileCoordForPosition(pos);
    
	
	if(canBuildOnTilePosition(ccpSub(pos, ccp(0, 30)))){
		switch(towerTag){
            case 1:
                if(gameHUD->getResources() >= 25){
                    target = MachineGunTower::create();
                    gameHUD->updateResources(-25);
                }else{
                    return;
                }
                
                break;
            case 2:
                if(gameHUD->getResources() >= 25){
                    target = FreezeTower::create();
                    gameHUD->updateResources(-25);
                }else{
                    return;
                }
                break;
            case 3:
                if(gameHUD->getResources() >= 25){
                    target = CannonTower::create();
                    gameHUD->updateResources(-25);
                }else{
                    return;
                }
                break;
            case 4:
                if(gameHUD->getResources() >= 25){
                    target = MutilTower::create();
                    gameHUD->updateResources(-25);
                }else{
                    return;
                }
                break;
            default:
                break;
		}
		target->setPosition(positionForTileCoord(towerLoc));
		this->addChild(target, 5);
        
		m->getTowers()->addObject(target);
        
		TileData* td = getTileData(towerLoc);
		td->setIsUsed(true);
	}
}

CCArray* MainLayer::getTilesNextToTile(cocos2d::CCPoint tileCoord){
	CCArray* tiles = CCArray::createWithCapacity(4);
    
	if(tileCoord.y + 1 < maxTileHeight){
		tiles->addObject(CCString::createWithFormat("{%f, %f}", tileCoord.x, tileCoord.y + 1));
	}
	if(tileCoord.x + 1 < maxTileWidth){
		tiles->addObject(CCString::createWithFormat("{%f, %f}", tileCoord.x + 1, tileCoord.y));
	}
	if(tileCoord.y - 1 >= 0){
		tiles->addObject(CCString::createWithFormat("{%f, %f}", tileCoord.x, tileCoord.y - 1));
	}
	if(tileCoord.x - 1 >= 0){
		tiles->addObject(CCString::createWithFormat("{%f, %f}", tileCoord.x - 1, tileCoord.y));
	}
	return tiles;
}

void MainLayer::addWaves(){
	GameMediator* m = GameMediator::sharedMediator();
    
	Wave* wave = Wave::create(2.0f, 10, 0);
	m->getWaves()->addObject(wave);
	wave = NULL;
    
	wave = Wave::create(1.7f, 5, 15);
	m->getWaves()->addObject(wave);
	wave = NULL;
    
	wave = Wave::create(1.5f, 25, 25);
	m->getWaves()->addObject(wave);
	wave = NULL;
    
	wave = Wave::create(1.3f, 40, 30);
	m->getWaves()->addObject(wave);
	wave = NULL;
    
	wave = Wave::create(1.2f, 40, 60);
	m->getWaves()->addObject(wave);
	wave = NULL;
}

Wave* MainLayer::getCurrentWave(){
	GameMediator* m = GameMediator::sharedMediator();
	Wave* wave = (Wave*)m->getWaves()->objectAtIndex(currentLevel);
	return wave;
}

Wave* MainLayer::getNextWave(){
	GameMediator* m = GameMediator::sharedMediator();
	currentLevel ++;
    
	if(currentLevel >=5){
		CCLog("you have reached the end of the game!");
		currentLevel = 0;
        this->unscheduleAllSelectors();
        return NULL;
	}
	Wave* wave = (Wave*)m->getWaves()->objectAtIndex(currentLevel);
	return wave;
}

void MainLayer::addTarget(){
	GameMediator* m = GameMediator::sharedMediator();
    
	Wave* wave = this->getCurrentWave();
	if(wave->getRedEnemys() == 0 && wave->getGreenEnemys() == 0){
		return;
	}
    
	Enemy* target = NULL;
	if(rand() % 2 == 0){
		if(wave->getRedEnemys() > 0){
			target = FastRedEnemy::create();
			wave->setRedEnemys(wave->getRedEnemys() - 1);
		}else if(wave->getGreenEnemys() > 0){
			target = StrongGreenEnemy::create();
			wave->setGreenEnemys(wave->getGreenEnemys() - 1);
		}
        
	}else{
		if(wave->getGreenEnemys() > 0){
			target = StrongGreenEnemy::create();
			wave->setGreenEnemys(wave->getGreenEnemys() - 1);
		}else if(wave->getRedEnemys() > 0){
			target = FastRedEnemy::create();
			wave->setRedEnemys(wave->getRedEnemys() - 1);
		}
	}
    
	this->addChild(target, 1);
	m->getTargets()->addObject(target);
}

void MainLayer::gameLogic(float dt){
	GameMediator* m = GameMediator::sharedMediator();
    
	Wave* wave = this->getCurrentWave();
    
	if(m->getTargets()->count() == 0 && wave->getRedEnemys()<= 0 && wave->getGreenEnemys() <= 0){
		this->getNextWave();
		gameHUD->updateWaveCount();
	}
    
	static long lastTimeTargetAdded = 0;
	long now = millisecondNow();
	if(lastTimeTargetAdded == 0 || now - lastTimeTargetAdded >= wave->getSpawnRate() * 1000){
		this->addTarget();
		lastTimeTargetAdded = now;
	}
}

bool MainLayer::isOutOfMap(cocos2d::CCPoint pos){
	if(CCRect::CCRectContainsPoint(background->boundingBox(), pos)){
		return false;
	}
	return true;
}

void MainLayer::removeTower(Tower* tower){
	GameMediator* gm = GameMediator::sharedMediator();
	gm->getTowers()->removeObject(tower);
    
	CCPoint coordPos = tileCoordForPosition(tower->getPosition());
	TileData* td = getTileData(coordPos);
	td->setIsUsed(false);
    
	this->removeChild(tower, true);
}

void MainLayer::strikeLight(float dt){
	Lightning *l = (Lightning *)this->getChildByTag(999);
    
	srand(time(NULL));
	CCRANDOM_0_1();
    
	//random position
	l->setStrikePoint(ccp(20 + CCRANDOM_0_1() * background->getContentSize().width, background->getContentSize().height));
	l->setStrikePoint2(ccp(20 + CCRANDOM_0_1() * background->getContentSize().width, 10));
	l->setStrikePoint3(ccp(20 + CCRANDOM_0_1() * background->getContentSize().width, 10));
    
	//random color
	l->setColor(ccc3(CCRANDOM_0_1() * 255, CCRANDOM_0_1() * 255, CCRANDOM_0_1() * 255));
    
	//random style
	l->setDisplacement(100 + CCRANDOM_0_1() * 200);
	l->setMinDisplacement(4 + CCRANDOM_0_1() * 10);
	l->setLighteningWidth(2.0f);
	l->setSplit(true);
    
	//call strike
	l->strikeRandom();
}

