#ifndef _COLLISION_H_
#define _COLLISION_H_

class Collision
{
private:
    bool mIsWater;
public:
    Collision()
    {
        mIsWater = false;
    }
    virtual ~Collision()
    {
    }
    virtual const Vector3 getCenter() = 0;
    virtual const Vector3 getSize() = 0;
    virtual const bool isBox()
    {
        return false;
    }
    virtual const bool isSphere()
    {
        return false;
    }
    virtual const bool isCircle()
    {
        return false;
    }
    virtual const bool isPhysics()
    {
        return false;
    }
    virtual const bool isGate()
    {
        return false;
    }
    void setIsWater(const bool &flag)
    {
        mIsWater = flag;
    }
    const bool isWater()
    {
        return mIsWater;
    }
    enum QueryFlags
    {
        WORLDOBJECT_MASK = 1<<0,
        WORLDCOLLISION_MASK = 1<<1,
        WORLDBILLBOARD_MASK = 1<<2,
        WORLDPARTICLE_MASK = 1<<3,
        WORLDLIGHT_MASK = 1<<4,
        WORLDWATER_MASK = 1<<5,
        WORLDGATE_MASK = 1<<6,
        UNIT_MASK = 1<<7,
        MESH_COLL_MASK = 1<<8
    };
};

class PhysicsCollision : public Collision
{
public:
    PhysicsCollision() : Collision()
    {
    }
    virtual ~PhysicsCollision()
    {
    }
    virtual const bool collides(Collision *coll) = 0;
    virtual const pair<bool,Real> collides(const Ray &ray) = 0;
    virtual const pair<bool,Vector3> collides(const Vector3 &point, const Real &headHeight) = 0;
    virtual const bool collides(const Vector3 &point) = 0;
    virtual const Real getTopHeight() = 0;
    virtual const Real getBottomHeight() = 0;
    virtual const bool standOnColl(const Real &base, const Real &height) = 0;
    const bool isPhysics()
    {
        return true;
    }
};

class CollBox : public PhysicsCollision
{
private:
    AxisAlignedBox mBox;
    Vector3 mCenter;
    Vector3 mSize;
public:
    CollBox(const Vector3 &center, const Vector3 &size) : PhysicsCollision()
    {
        mCenter = center;
        mSize = size;
        mBox.setMinimum(Vector3(center.x-size.x*0.5f,center.y,center.z-size.z*0.5f));
        mBox.setMaximum(Vector3(center.x+size.x*0.5f,center.y+size.y,center.z+size.z*0.5f));
    }
    const Vector3 getCenter()
    {
        return mCenter;
    }
    const Vector3 getSize()
    {
        return mSize;
    }
    const AxisAlignedBox getBox()
    {
        return mBox;
    }
    const bool collides(Collision *coll);
    const pair<bool,Real> collides(const Ray &ray)
    {
        return ray.intersects(mBox);
    }
    const pair<bool,Vector3> collides(const Vector3 &point, const Real &headHeight)
    {
        const bool flag = mBox.intersects(AxisAlignedBox(point,point+Vector3(0,headHeight,0)));
        Vector3 force = Vector3::ZERO;
        if(flag)
        {
            const Real minX = mBox.getMinimum().x - point.x;
            const Real minY = mBox.getMinimum().y - point.y;
            const Real minZ = mBox.getMinimum().z - point.z;
            const Real maxX = mBox.getMaximum().x - point.x;
            const Real maxY = mBox.getMaximum().y - point.y;
            const Real maxZ = mBox.getMaximum().z - point.z;
            const Real nearestX = (Math::Abs(minX)<=Math::Abs(maxX) ? minX : maxX);
            const Real nearestY = (Math::Abs(minY)<=Math::Abs(maxY) ? minY : maxY);
            const Real nearestZ = (Math::Abs(minZ)<=Math::Abs(maxZ) ? minZ : maxZ);
            if(Math::Abs(nearestX)<=Math::Abs(nearestZ))
            {
                if(Math::Abs(nearestX)<=Math::Abs(nearestY))force = Vector3(nearestX,0,0);
                else force = Vector3(0,nearestY,0);
            }
            else
            {
                if(Math::Abs(nearestY)<=Math::Abs(nearestZ))force = Vector3(0,nearestY,0);
                else force = Vector3(0,0,nearestZ);
            }
        }
        return pair<bool,Vector3>(flag,force);
    }
    const bool collides(const Vector3 &point)
    {
        return mBox.intersects(point);
    }
    const bool isBox()
    {
        return true;
    }
    const Real getTopHeight()
    {
        return mCenter.y + mSize.y;
    }
    const Real getBottomHeight()
    {
        return mCenter.y;
    }
    const bool standOnColl(const Real &base, const Real &height)
    {
        return (base+height > getTopHeight());
    }
};

class CollSphere : public PhysicsCollision
{
private:
    Sphere mSphere;
public:
    CollSphere(const Vector3 &center, const Real &size) : PhysicsCollision()
    {
        mSphere.setCenter(center);
        mSphere.setRadius(size);
    }
    const Sphere getSphere()
    {
        return mSphere;
    }
    const Vector3 getCenter()
    {
        return mSphere.getCenter();
    }
    const Vector3 getSize()
    {
        return Vector3::UNIT_SCALE*mSphere.getRadius();
    }
    const bool collides(Collision *coll)
    {
        if(coll->isBox())
        {
            return mSphere.intersects(static_cast<CollBox*>(coll)->getBox());
        }
        if(coll->isSphere())
        {
            return mSphere.intersects(static_cast<CollSphere*>(coll)->getSphere());
        }
        return false;
    }
    const pair<bool,Real> collides(const Ray &ray)
    {
        //Conditional fix for ray-sphere intersection bug
        if(ray.getDirection()!=Vector3::NEGATIVE_UNIT_Y || ray.getOrigin().y>=mSphere.getCenter().y)return ray.intersects(mSphere);
        return pair<bool,Real>(false,0);
    }
    const pair<bool,Vector3> collides(const Vector3 &point, const Real &headHeight)
    {
        const bool flag = mSphere.intersects(AxisAlignedBox(point,point+Vector3(0,headHeight,0)));
        Vector3 force = Vector3::ZERO;
        if(flag)
        {
            Real r = mSphere.getRadius();

            if(point.y > mSphere.getCenter().y)
            {
                const Real y = Math::Abs(point.y - mSphere.getCenter().y);
                //Radius of circle at unit's feet
                r = Math::Sqrt(mSphere.getRadius()*mSphere.getRadius() - y*y);
            }
            else if(point.y+headHeight < mSphere.getCenter().y)
            {
                const Real y = Math::Abs(point.y + headHeight - mSphere.getCenter().y);
                //Radius of circle at unit's head
                r = Math::Sqrt(mSphere.getRadius()*mSphere.getRadius() - y*y);
            }

            const Real x = point.x - mSphere.getCenter().x;
            const Real z = point.z - mSphere.getCenter().z;
            if(Math::Abs(x)>0.1 && Math::Abs(z)>0.1)
            {
                Radian angle = Degree(0);
                if(z!=0)angle = Math::ATan(x/z);
                else angle = Degree(x>=0?90:270);
                if(x<0 && z>=0)angle += Degree(360);
                if(z<0)angle += Degree(180);

                force = Vector3(Math::Sin(angle)*r-x,0,Math::Cos(angle)*r-z);
            }
        }
        return pair<bool,Vector3>(flag,force);
    }
    const bool collides(const Vector3 &point)
    {
        return mSphere.intersects(point);
    }
    const bool isSphere()
    {
        return true;
    }
    const Real getTopHeight()
    {
        return mSphere.getCenter().y + mSphere.getRadius();
    }
    const Real getBottomHeight()
    {
        return mSphere.getCenter().y - mSphere.getRadius();
    }
    const bool standOnColl(const Real &base, const Real &height)
    {
        return (base > mSphere.getCenter().y+mSphere.getRadius()*0.8f);
    }
};

const bool CollBox::collides(Collision *coll)
{
    if(coll->isBox())
    {
        return mBox.intersects(static_cast<CollBox*>(coll)->getBox());
    }
    if(coll->isSphere())
    {
        return mBox.intersects(static_cast<CollSphere*>(coll)->getSphere());
    }
    return false;
}

struct WaterPlane
{
    Vector3 mCenter;
    Vector2 mSize;
    Entity *mEnt;
    bool mIsReflectMat;
    WaterPlane(const Vector3 &center, const Vector2 &size, Entity *ent, const bool &isReflectMat)
    {
        mCenter = center;
        mSize = size;
        mEnt = ent;
        mIsReflectMat = isReflectMat;
    }
};

class FlatCircle : public Collision
{
protected:
    Plane mPlane;
    Sphere mSphere;
public:
    FlatCircle(const Vector3 &center, const Real &radius, const Vector3 &normal) : Collision()
    {
        mSphere.setCenter(center);
        mSphere.setRadius(radius);
        mPlane.normal = normal;
        mPlane.d = 0;
        mPlane.d = -mPlane.getDistance(center);
    }
    virtual ~FlatCircle()
    {
    }
    const Vector3 getCenter()
    {
        return mSphere.getCenter();
    }
    const Vector3 getSize()
    {
        return Vector3::UNIT_SCALE*mSphere.getRadius();
    }
    const Vector3 getNormal()
    {
        return mPlane.normal;
    }
    const bool intersects(const Vector3 &center, const Real &halfsize)
    {
        const AxisAlignedBox box(center.x-halfsize,center.y,center.z-halfsize,center.x+halfsize,center.y+halfsize*2,center.z+halfsize);
        return (box.intersects(mSphere) && box.intersects(mPlane));
    }
    const bool isCircle()
    {
        return true;
    }
};

struct GateInfo
{
    String mMap;
    Vector3 mPosition;
    Real mYaw;
    GateInfo()
    {
        mMap = "";
        mPosition = Vector3::ZERO;
        mYaw = 0;
    }
};

class GateCollision : public FlatCircle
{
private:
    GateInfo mMapInfo;
public:
    GateCollision(const GateInfo &map, const Vector3 &center, const Real &radius, const Vector3 &normal) : FlatCircle(center,radius,normal)
    {
        mMapInfo = map;
    }
    ~GateCollision()
    {
    }
    const GateInfo getMapInfo()
    {
        return mMapInfo;
    }
    const bool isGate()
    {
        return true;
    }
    const bool isNear(const Vector3 &center, const Real &halfsize)
    {
        const AxisAlignedBox box(center.x-halfsize,center.y,center.z-halfsize,center.x+halfsize,center.y+halfsize*2,center.z+halfsize);
        return box.intersects(Sphere(mSphere.getCenter(),3*mSphere.getRadius()));
    }
};

#endif
