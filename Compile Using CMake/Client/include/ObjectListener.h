#ifndef _OBJECTLISTENER_H_
#define _OBJECTLISTENER_H_

class ObjectListenerObject
{
public:
    enum ObjectType
    {
        OBJECTTYPE_OBJECTLISTENEROBJECT,
        OBJECTTYPE_OBJECT,
        OBJECTTYPE_ANIMATED,
        OBJECTTYPE_MOVABLE,
        OBJECTTYPE_CONTROLLABLE,
        OBJECTTYPE_LIVING,
        OBJECTTYPE_UNIT
    };
    virtual const unsigned char getType()
    {
        return OBJECTTYPE_OBJECT;
    }
    virtual SceneNode* getSceneNode()
    {
        return 0;
    }
};

class ObjectListener
{
public:
    enum
    {
        OBJECT_INVISIBLE,
        OBJECT_VISIBLE,
        OBJECT_DESTROYED,
        CONTROLLABLE_STARTED,
        CONTROLLABLE_STOPPED,
        UNIT_SPECIALACTION_STOPPED
    };
    ObjectListener(){}
    virtual ~ObjectListener(){}
    virtual const bool objectEvent(ObjectListenerObject *object, const unsigned char &event) = 0;
    virtual const bool controllableEvent(ObjectListenerObject *object, const unsigned char &event) = 0;
    virtual const bool unitEvent(ObjectListenerObject *object, const unsigned char &event) = 0;
};

#endif
