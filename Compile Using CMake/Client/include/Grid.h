#ifndef _GRID_H_
#define _GRID_H_

#include <Collision.h>
#include <SoundManager.h>

class Grid
{
private:
    Grid *mNeighbour[8];
    unsigned short mX;
    unsigned short mY;
    Real mXStart;
    Real mYStart;
    StaticGeometry *mGeometry;
    SceneNode *mSceneNode;
    SceneNode *mRootNode;
    std::vector<PhysicsCollision*> mCollList;
    std::vector<PhysicsCollision*> mWaterCollList;
    std::vector<GateCollision*> mGateCollList;
    std::vector<WaterPlane*> mWaterList;
    std::vector<pair<String,Vector3> > mSoundList;
    std::vector<ISound*> mActiveSoundList;
    bool mActive;
public:
    Grid(const unsigned short &x, const unsigned short &y)
    {
        mX = x;
        mY = y;
        mXStart = x*GRID_WIDTH;
        mYStart = y*GRID_WIDTH;
        for(int i=0;i<8;i++)mNeighbour[i] = 0;
        mActive = false;
        clear();
    }
    ~Grid()
    {
        clear();
    }
    void clear()
    {
        mGeometry = 0;
        mSceneNode = 0;
        mRootNode = 0;
        mCollList.clear();
        mWaterList.clear();
        mGateCollList.clear();
        mSoundList.clear();
        mActiveSoundList.clear();
    }
    void addCollision(Collision *coll)
    {
        if(coll->isWater()&&coll->isPhysics())mWaterCollList.push_back(static_cast<PhysicsCollision*>(coll));
        else if(coll->isPhysics())mCollList.push_back(static_cast<PhysicsCollision*>(coll));
        else if(coll->isGate())mGateCollList.push_back(static_cast<GateCollision*>(coll));
    }
    void addWaterPlane(WaterPlane *water)
    {
        mWaterList.push_back(water);
    }
    void setStaticGeometry(StaticGeometry *geom)
    {
        mGeometry = geom;
    }
    void setRenderingDistance(const Real &maxDist)
    {
        mGeometry->setRenderingDistance(maxDist);
    }
    void setSceneNode(SceneNode *node)
    {
        mSceneNode = node;
    }
    void setRootNode(SceneNode *node)
    {
        mRootNode = node;
    }
    void addEntity(Entity *ent, const Vector3 &position, const Quaternion &orientation = Quaternion::IDENTITY, const Vector3 &scale = Vector3::UNIT_SCALE)
    {
        mGeometry->addEntity(ent,position,orientation,scale);
    }
    void addMovableObject(MovableObject *object, const Vector3 &position=Vector3::ZERO, const Quaternion &orientation = Quaternion::IDENTITY, const Vector3 &scale = Vector3::UNIT_SCALE, const bool &isLarge=false)
    {
        SceneNode *node = (isLarge ? mRootNode->createChildSceneNode() : mSceneNode->createChildSceneNode());
        node->attachObject(object);
        node->setPosition(position);
        node->setOrientation(orientation);
        node->setScale(scale);
    }
    void addSound(const String &filename, const Vector3 &position)
    {
        mSoundList.push_back(pair<String,Vector3>(filename,position));
    }
    void buildGeometry()
    {
        mGeometry->setCastShadows(true);
        mGeometry->build();
    }
    void setCastShadows(const bool &flag)
    {
        mGeometry->setCastShadows(flag);
    }
    void activate()
    {
        mActive = true;
        //mGeometry->setVisible(true);
        mSceneNode->setVisible(true);

        //Play sounds
        for(std::vector<pair<String,Vector3> >::iterator it=mSoundList.begin(); it!=mSoundList.end(); it++)
        {
            const pair<String,Vector3> soundInfo = *it;

            ISound *sound = SoundManager::getSingletonPtr()->play3DSound(soundInfo.first.c_str(),soundInfo.second,true);
            if(sound)mActiveSoundList.push_back(sound);
        }
    }
    void deactivate()
    {
        mActive = false;
        //mGeometry->setVisible(false);
        mSceneNode->setVisible(false);

        //Stop sounds
        while(!mActiveSoundList.empty())
        {
            ISound *sound = mActiveSoundList.back();
            mActiveSoundList.pop_back();
            SoundManager::getSingletonPtr()->stopLoopedSound(sound);
        }
    }
    void activateNeighbours(const unsigned char &dir=0)
    {
        switch(dir)
        {
            //North
            case 1:
                for(int i=0;i<2;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                if(mNeighbour[7])mNeighbour[7]->activate();
                break;
            //Northeast
            case 2:
                for(int i=0;i<4;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                if(mNeighbour[7])mNeighbour[7]->activate();
                break;
            //East
            case 3:
                for(int i=1;i<4;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                break;
            //Southeast
            case 4:
                for(int i=1;i<6;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                break;
            //South
            case 5:
                for(int i=3;i<6;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                break;
            //Southwest
            case 6:
                for(int i=3;i<8;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                break;
            //West
            case 7:
                for(int i=5;i<8;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                break;
            //Northwest
            case 8:
                for(int i=5;i<8;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                for(int i=0;i<2;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                break;
            default:
                for(int i=0;i<8;i++)if(mNeighbour[i])mNeighbour[i]->activate();
                break;
        }
    }
    void deactivateNeighbours(const unsigned char &dir=0)
    {
        switch(dir)
        {
            //North
            case 5:
                for(int i=0;i<2;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                if(mNeighbour[7])mNeighbour[7]->deactivate();
                break;
            //Northeast
            case 6:
                for(int i=0;i<4;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                if(mNeighbour[7])mNeighbour[7]->deactivate();
                break;
            //East
            case 7:
                for(int i=1;i<4;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                break;
            //Southeast
            case 8:
                for(int i=1;i<6;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                break;
            //South
            case 1:
                for(int i=3;i<6;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                break;
            //Southwest
            case 2:
                for(int i=3;i<8;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                break;
            //West
            case 3:
                for(int i=5;i<8;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                break;
            //Northwest
            case 4:
                for(int i=5;i<8;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                for(int i=0;i<2;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                break;
            default:
                for(int i=0;i<8;i++)if(mNeighbour[i])mNeighbour[i]->deactivate();
                break;
        }
    }
    Grid *getNeighbour(const unsigned char &i)
    {
        return mNeighbour[i];
    }
    void setNeighbour(const unsigned char &i, Grid* grid)
    {
        mNeighbour[i] = grid;
    }
    const bool isInGrid(const Real &x, const Real &y)
    {
        return (x>=mXStart && x<mXStart+GRID_WIDTH && y>=mYStart && y<mYStart+GRID_WIDTH);
    }
    const unsigned short getX()
    {
        return mX;
    }
    const unsigned short getY()
    {
        return mY;
    }
    Grid* getContainingNeighbour(const unsigned short &x, const unsigned short &y)
    {
        if(mX-1 == x)
        {
            if(mY-1 == y)return mNeighbour[7];
            if(mY == y)return mNeighbour[6];
            if(mY+1 == y)return mNeighbour[5];
        }
        if(mX == x)
        {
            if(mY-1 == y)return mNeighbour[0];
            if(mY+1 == y)return mNeighbour[4];
        }
        if(mX+1 == x)
        {
            if(mY-1 == y)return mNeighbour[1];
            if(mY == y)return mNeighbour[2];
            if(mY+1 == y)return mNeighbour[3];
        }
        return 0;
    }
    const Real getHighestCollision(const Ray &ray, const bool &includeNeighbours=true)
    {
        Real highest = 0;
        for(std::vector<PhysicsCollision*>::iterator i=mCollList.begin(); i!=mCollList.end(); i++)
        {
            PhysicsCollision *coll = *i;
            const pair<bool,Real> result = coll->collides(ray);
            if(result.first)
            {
                const Real height = ray.getPoint(result.second).y;
                if(height>highest)highest = height;
            }
        }
        if(includeNeighbours)
        {
            for(int i=0;i<8;i++)
                if(mNeighbour[i])
                {
                    const Real result = mNeighbour[i]->getHighestCollision(ray,false);
                    if(result>highest)highest = result;
                }
        }
        return highest;
    }
    const Vector3 getCollisionPenalty(const Vector3 &point, const Real &headHeight, std::vector<PhysicsCollision*> *groundList=0, const bool *isJumping=0, bool *killJump=0, const bool &includeNeighbours=true)
    {
        Vector3 displacement = Vector3::ZERO;
        for(std::vector<PhysicsCollision*>::iterator i=mCollList.begin(); i!=mCollList.end(); i++)
        {
            PhysicsCollision *coll = *i;
            const pair<bool,Vector3> result = coll->collides(point,headHeight);
            //Collided
            if(result.first)
            {
                //Unit head is above collision (for box) or feet is above mid height (for sphere)
                if(coll->standOnColl(point.y,headHeight))
                {
                    if(groundList)groundList->push_back(coll);
                }
                //Unit is jumping and feet is below collision
                else if(killJump && isJumping && *isJumping && point.y < coll->getBottomHeight())*killJump = true;
                else displacement += result.second;
            }
            //Is below unit
            else if(groundList && coll->collides(Ray(point,Vector3::NEGATIVE_UNIT_Y)).first)groundList->push_back(coll);
        }
        if(includeNeighbours)
        {
            for(int i=0;i<8;i++)
                if(mNeighbour[i])displacement += mNeighbour[i]->getCollisionPenalty(point,headHeight,groundList,isJumping,killJump,false);
        }
        return displacement;
    }
    const bool getWaterCollision(const Vector3 &point, const Real &headHeight, bool *isSwimming=0, bool *headBelowWater=0, const bool &includeNeighbours=true)
    {
        //Check water planes
        for(std::vector<WaterPlane*>::iterator i=mWaterList.begin(); i!=mWaterList.end(); i++)
        {
            WaterPlane *water = *i;
            const Vector3 center = water->mCenter;
            const Vector2 size = water->mSize;
            const Real minX = center.x - size.x*0.5f;
            const Real minZ = center.z - size.y*0.5f;
            const Real maxX = center.x + size.x*0.5f;
            const Real maxZ = center.z + size.y*0.5f;
            if(point.y<=center.y && point.x>=minX && point.x<=maxX && point.z>=minZ && point.z<=maxZ)
            {
                if(isSwimming)*isSwimming = (point.y+headHeight<=center.y);
                if(headBelowWater)*headBelowWater = (point.y+headHeight*1.2f<=center.y);
                return true;
            }
        }
        //Check water collboxes
        for(std::vector<PhysicsCollision*>::iterator i=mWaterCollList.begin(); i!=mWaterCollList.end(); i++)
        {
            PhysicsCollision *coll = *i;
            if(coll->collides(point+Vector3(0,headHeight,0)))
            {
                if(isSwimming)*isSwimming = true;
                if(headBelowWater)*headBelowWater = coll->collides(point+Vector3(0,headHeight*1.2f,0));
                return true;
            }
        }
        if(includeNeighbours)
        {
            for(int i=0;i<8;i++)
                if(mNeighbour[i] && mNeighbour[i]->getWaterCollision(point,headHeight,isSwimming,headBelowWater,false))return true;
        }
        return false;
    }
    const GateInfo getGateHit(const Vector3 &center, const Real &halfsize, const bool &includeNeighbours=true)
    {
        for(std::vector<GateCollision*>::iterator i=mGateCollList.begin(); i!=mGateCollList.end(); i++)
        {
            GateCollision *coll = *i;
            if(coll->intersects(center,halfsize))return coll->getMapInfo();
        }
        if(includeNeighbours)return getGateHit(center,halfsize,false);
        return GateInfo();
    }
    const GateInfo getGateNearHit(const Vector3 &center, const Real &halfsize, const bool &includeNeighbours=true)
    {
        for(std::vector<GateCollision*>::iterator i=mGateCollList.begin(); i!=mGateCollList.end(); i++)
        {
            GateCollision *coll = *i;
            if(coll->isNear(center,halfsize))return coll->getMapInfo();
        }
        if(includeNeighbours)return getGateNearHit(center,halfsize,false);
        return GateInfo();
    }
    const unsigned char compareDirection(Grid *grid)
    {
        const int dX = grid->mX - mX;
        const int dY = grid->mY - mY;
        if(Math::Abs(dX)>1 || Math::Abs(dY)>1 || (dX==0&&dY==0))return 0;
        if(dY<0)
        {
            if(dX<0)return 8;
            if(dX==0)return 1;
            if(dX>0)return 2;
        }
        if(dY==0)
        {
            if(dX<0)return 7;
            if(dX>0)return 3;
        }
        if(dY>0)
        {
            if(dX<0)return 6;
            if(dX==0)return 5;
            if(dX>0)return 4;
        }
        return 0;
    }
};

#endif
