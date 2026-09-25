#ifndef _COLLISIONMANAGER_H_
#define _COLLISIONMANAGER_H_

#include <Unit.h>
#include <GridManager.h>

class CollisionListener
{
public:
    CollisionListener(){}
    virtual ~CollisionListener(){}
    virtual void mapChange(const GateInfo &map){}
    virtual void nearGate(const String &name){}
};

class CollisionManager : public Singleton<CollisionManager>
{
private:
    SceneManager *mSceneMgr;
    GridManager *mGridMgr;
    RaySceneQuery *mRayQuery;
    RaySceneQuery *mMouseQuery;
    RaySceneQuery *mMeshQuery;
    std::vector<Collision*> mCollList;
    unsigned int mDebugEntityCounter;
    std::vector<PhysicsCollision*> mGroundList;
    bool mShowCollisions;
    CollisionListener *mListener;
    Plane mMeshCollPlane;
    bool mHasMeshCollPlane;
    Real mMeshCollCeiling[4];
    Real mCeilingHeight;
    Real mMapMinBound;
    Real mMapMaxBound;
public:
    CollisionManager()
    {
        mRayQuery = 0;
        mMouseQuery = 0;
        mMeshQuery = 0;
        mCollList.clear();
        createSphereMesh("CollSphere",1);
        createBoxMesh("CollBox",1);
        createCircleMesh("FlatCircle",1);
        mDebugEntityCounter = 0;
        mGroundList.clear();
        mListener = 0;
        mHasMeshCollPlane = false;
    }
    ~CollisionManager()
    {
        reset();
    }
    void init(SceneManager *sceneMgr)
    {
        mSceneMgr = sceneMgr;
        mRayQuery = mSceneMgr->createRayQuery(Ray());
        mMouseQuery = mSceneMgr->createRayQuery(Ray());
        mMeshQuery = mSceneMgr->createRayQuery(Ray());
        mMeshQuery->setQueryMask(Collision::MESH_COLL_MASK);
        mMeshQuery->setSortByDistance(true);
        mGridMgr = GridManager::getSingletonPtr();
        mShowCollisions = false;
        for(int i=0;i<4;i++)mMeshCollCeiling[i] = 10000;
        mCeilingHeight = 100000;
        mMapMinBound = 0;
        mMapMaxBound = 10000;
    }
    void reset()
    {
        while(!mCollList.empty())
        {
            Collision *coll = mCollList.back();
            mCollList.pop_back();
            delete coll;
        }
        if(mRayQuery)delete mRayQuery;
        if(mMouseQuery)delete mMouseQuery;
        if(mMeshQuery)delete mMeshQuery;
        mRayQuery = 0;
        mMouseQuery = 0;
        mMeshQuery = 0;
        mGroundList.clear();
        mDebugEntityCounter = 0;
        mListener = 0;
        mHasMeshCollPlane = false;
    }
    static CollisionManager* getSingletonPtr();
    static CollisionManager& getSingleton();
    void setListener(CollisionListener *listener)
    {
        mListener = listener;
    }
    void setCeilingHeight(const Real &height)
    {
        mCeilingHeight = height;
    }
    void setMapBounds(const Real &min, const Real &max)
    {
        mMapMinBound = min+10;
        mMapMaxBound = max-10;
    }
    void updateCollision(Movable *unit, const bool &updateGroundList=false)
    {
        Vector3 unitPosition = unit->getPosition();

        //Map bounds
        if(unitPosition.x<mMapMinBound)unit->addPenaltyDisplacement(Vector3(mMapMinBound - unitPosition.x,0,0));
        else if(unitPosition.x>mMapMaxBound)unit->addPenaltyDisplacement(Vector3(mMapMaxBound - unitPosition.x,0,0));
        if(unitPosition.z<mMapMinBound)unit->addPenaltyDisplacement(Vector3(0,0,mMapMinBound - unitPosition.z));
        else if(unitPosition.z>mMapMaxBound)unit->addPenaltyDisplacement(Vector3(0,0,mMapMaxBound - unitPosition.z));
        unitPosition = unit->getPositionAfterPenalty();

        Grid *activeGrid = mGridMgr->getGridContaining(unitPosition.x,unitPosition.z);
        if(!activeGrid)return;

        if(updateGroundList)mGroundList.clear();
        bool killJump = false;
        const bool isJumping = (!unit->isGrounded());
        const Vector3 unitVelocity = unit->getVelocity();
        const Real unitHeadHeight = unit->getHeadHeight();
        const Real unitFeetDistance = unit->getFrontFeetDistance();
        unit->addPenaltyDisplacement(activeGrid->getCollisionPenalty(unitPosition,unitHeadHeight,updateGroundList?&mGroundList:0,&isJumping,&killJump));
        unitPosition = unit->getPositionAfterPenalty();

        //Gate collision
        const GateInfo nearGateMap = activeGrid->getGateNearHit(unitPosition,unitFeetDistance);
        if(nearGateMap.mMap!="")
        {
            if(mListener)mListener->nearGate(nearGateMap.mMap);
            const GateInfo mapChange = activeGrid->getGateHit(unitPosition,unitFeetDistance);
            if(mapChange.mMap!="" && mListener)mListener->mapChange(mapChange);
        }

        //Mesh sideways collision
        if(mHasMeshCollPlane)
        {
            const Real angle = mMeshCollPlane.normal.angleBetween(Vector3(mMeshCollPlane.normal.x,0,mMeshCollPlane.normal.z)).valueDegrees();
            const Real d = mMeshCollPlane.getDistance(unitPosition+Vector3(0,unitHeadHeight,0));
            //Execute penalty displacement immediately to ensure next ray doesn't fall behind plane
            if(d<unitFeetDistance && (angle<60 || mMeshCollPlane.normal.y<0))
            {
                unit->setPosition(unitPosition + mMeshCollPlane.normal*(unitFeetDistance-d+1));
                unitPosition = unit->getPosition();
            }
        }
        Vector3 front;
        Plane frontPlane;
        const Vector3 moveDir = Vector3(unitVelocity.x,0,unitVelocity.z).normalisedCopy();
        const Vector3 unitHeadPosition = unitPosition + Vector3(0,unitHeadHeight,0);
        if(moveDir.squaredLength()>0.1f && getMeshCollision(Ray(unitHeadPosition,moveDir),front,&frontPlane))
        {
            frontPlane.normal.normalise();
            if(Math::Abs(frontPlane.normal.angleBetween(moveDir).valueDegrees())<90)
            {
                frontPlane.normal = -frontPlane.normal;
                frontPlane.d = -frontPlane.d;
            }
            frontPlane.normalise();
            mMeshCollPlane = frontPlane;
            mHasMeshCollPlane = true;
        }
        else mHasMeshCollPlane = false;

        //Ceiling collision
        if(unitPosition.y+unitHeadHeight > mCeilingHeight)
        {
            killJump = true;
            //Offset position immediately
            unit->setPosition(Vector3(unitPosition.x,mCeilingHeight-unitHeadHeight,unitPosition.z));
            unitPosition = unit->getPosition();
        }

        //Mesh ceiling collision
        Real ceilingHeight = mMeshCollCeiling[3];
        for(int i=0;i<3;i++)
            if(mMeshCollCeiling[i]<ceilingHeight)ceilingHeight = mMeshCollCeiling[i];
        if(unitVelocity.y>0 && (unitPosition.y+unitHeadHeight*1.5f)>=ceilingHeight)
        {
            killJump = true;
            //Offset position immediately
            unit->setPosition(Vector3(unitPosition.x,ceilingHeight-unitHeadHeight*1.5f,unitPosition.z));
        }
        Vector3 ground[4];
        unit->getGroundPoints(ground[0],ground[1],ground[2],ground[3]);
        for(int i=0;i<4;i++)
        {
            mMeshCollCeiling[i] = 10000;
            Vector3 top = Vector3::ZERO;
            Real groundOffset = unitHeadHeight;
            while(getMeshCollision(Ray(ground[i]+Vector3(0,groundOffset,0),Vector3::UNIT_Y),top))
            {
                if(top.y>unit->getGroundHeight(i)+unitHeadHeight)
                {
                    mMeshCollCeiling[i] = top.y;
                    break;
                }
                else
                {
                    if(unitHeadHeight<=0)break;
                    groundOffset += unitHeadHeight;
                }
            }
        }

        //Kill jump
        if(killJump && unitVelocity.y>0)unit->setVelocityY(0);
    }
    void updateGroundHeight(Movable *unit)
    {
        unit->setGroundHeight(getGroundHeight(unit->getPosition(),false));
    }
    void updateGroundHeights(Movable *unit)
    {
        Vector3 n,s,e,w;
        unit->getGroundPoints(n,s,e,w);
        unit->setGroundHeights(getGroundHeight(n,true,mMeshCollCeiling[0]),getGroundHeight(s,true,mMeshCollCeiling[1]),getGroundHeight(e,true,mMeshCollCeiling[2]),getGroundHeight(w,true,mMeshCollCeiling[3]));
    }
    void updateNSGroundHeights(Movable *unit, const bool &includeCollision=false)
    {
        Vector3 n,s,e,w;
        unit->getGroundPoints(n,s,e,w);
        const Real headHeight = unit->getHeadHeight();
        unit->setGroundHeights(getGroundHeight(n,includeCollision,n.y+headHeight,false),getGroundHeight(s,includeCollision,s.y+headHeight,false));
    }
    const Real getGroundHeight(const Vector3 &pos, const bool &includeCollision=true, const Real &ceilingHeight=10000.0f, const bool &useGroundList=true, const Real &fRayOffset=1000.0f)
    {
        const Ray ray(pos + Vector3(0,fRayOffset,0),Vector3::NEGATIVE_UNIT_Y);

		//Get highest ground from collisions
		Real highest = 0;
		if(includeCollision)
		{
		    const Ray ceilingRay(pos + Vector3(0,ceilingHeight-5-pos.y,0),Vector3::NEGATIVE_UNIT_Y);

		    if(useGroundList)
		    {
                for(std::vector<PhysicsCollision*>::iterator i=mGroundList.begin(); i!=mGroundList.end(); i++)
                {
                    PhysicsCollision *coll = *i;
                    const pair<bool,Real> result = coll->collides(ray);
                    if(result.first)
                    {
                        const Real height = ray.getPoint(result.second).y;
                        if(height>highest)highest = height;
                    }
                }
		    }
		    else
		    {
		        Grid *activeGrid = mGridMgr->getGridContaining(pos.x,pos.z);
                if(activeGrid)
                {
                    const Real height = activeGrid->getHighestCollision(ceilingRay);
                    if(height>highest)highest = height;
                }
		    }

            //Get highest point on mesh collision
            Vector3 result;
            if(getMeshCollision(ceilingRay,result))
            {
                if(result.y>highest)highest = result.y;
            }
		}

		//Get highest ground from terrain
		mRayQuery->setRay(ray);
		RaySceneQueryResult& qryResult = mRayQuery->execute();
		RaySceneQueryResult::iterator i = qryResult.begin();
		if(i!=qryResult.end() && i->worldFragment)
		{
		    const Real result = i->worldFragment->singleIntersection.y;
		    if(result>highest)highest = result;
		}

		return highest;
    }
    Collision* createCollision(Collision* copy, const Vector3 &offset=Vector3::ZERO, const Vector3 &scale=Vector3::UNIT_SCALE, const Quaternion &orientation=Quaternion::IDENTITY, const String &mat="Collision/Highlight")
    {
        Collision *coll = 0;
        const Vector3 center = orientation*(copy->getCenter()*scale)+offset;
        const Vector3 collScale = copy->getSize()*scale;
        if(copy->isBox())coll = new CollBox(center,collScale);
        else if(copy->isSphere())coll = new CollSphere(center,collScale.y);
        else if(copy->isGate())coll = new GateCollision(static_cast<GateCollision*>(copy)->getMapInfo(),center,collScale.y,static_cast<GateCollision*>(copy)->getNormal());
        else if(copy->isCircle())coll = new FlatCircle(center,collScale.y,static_cast<FlatCircle*>(copy)->getNormal());

        if(!coll)return 0;

        if(copy->isWater())coll->setIsWater(true);

        if(mShowCollisions || coll->isGate())
        {
            Entity *ent = mSceneMgr->createEntity("CollEnt"+StringConverter::toString(mDebugEntityCounter++),(copy->isBox()?"CollBox": (copy->isSphere()?"CollSphere":"FlatCircle")));
            SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            node->attachObject(ent);
            node->setPosition(coll->getCenter());
            node->setScale(coll->getSize());
            if(coll->isCircle())node->setOrientation(orientation);

            if(coll->isGate())ent->setMaterialName(mat);
            else if(coll->isWater())ent->setMaterialName("Collision/HighlightBlue");
            else if(coll->isCircle())ent->setMaterialName("Collision/HighlightTwoSide");
            else ent->setMaterialName("Collision/Highlight");
        }

        mCollList.push_back(coll);
        return coll;
    }
    void createSphereMesh(const std::string& strName, const float &r, const int &nRings = 16, const int &nSegments = 16)
	{
		MeshPtr pSphere = MeshManager::getSingleton().createManual(strName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		SubMesh *pSphereVertex = pSphere->createSubMesh();

		pSphere->sharedVertexData = new VertexData();
		VertexData* vertexData = pSphere->sharedVertexData;

		// define the vertex format
		VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
		size_t currOffset = 0;
		// positions
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// normals
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// two dimensional texture coordinates
		vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
		currOffset += VertexElement::getTypeSize(VET_FLOAT2);

		// allocate the vertex buffer
		vertexData->vertexCount = (nRings + 1) * (nSegments+1);
		HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(0, vBuf);
		float* pVertex = static_cast<float*>(vBuf->lock(HardwareBuffer::HBL_DISCARD));

		// allocate index buffer
		pSphereVertex->indexData->indexCount = 6 * nRings * (nSegments + 1);
		pSphereVertex->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, pSphereVertex->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		HardwareIndexBufferSharedPtr iBuf = pSphereVertex->indexData->indexBuffer;
		unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(HardwareBuffer::HBL_DISCARD));

		float fDeltaRingAngle = (Math::PI / nRings);
		float fDeltaSegAngle = (2 * Math::PI / nSegments);
		unsigned short wVerticeIndex = 0 ;

		// Generate the group of rings for the sphere
		for( int ring = 0; ring <= nRings; ring++ ) {
			float r0 = r * sinf (ring * fDeltaRingAngle);
			float y0 = r * cosf (ring * fDeltaRingAngle);

			// Generate the group of segments for the current ring
			for(int seg = 0; seg <= nSegments; seg++) {
				float x0 = r0 * sinf(seg * fDeltaSegAngle);
				float z0 = r0 * cosf(seg * fDeltaSegAngle);

				// Add one vertex to the strip which makes up the sphere
				*pVertex++ = x0;
				*pVertex++ = y0;
				*pVertex++ = z0;

				Vector3 vNormal = Vector3(x0, y0, z0).normalisedCopy();
				*pVertex++ = vNormal.x;
				*pVertex++ = vNormal.y;
				*pVertex++ = vNormal.z;

				*pVertex++ = (float) seg / (float) nSegments;
				*pVertex++ = (float) ring / (float) nRings;

				if (ring != nRings) {
								   // each vertex (except the last) has six indices pointing to it
					*pIndices++ = wVerticeIndex + nSegments + 1;
					*pIndices++ = wVerticeIndex;
					*pIndices++ = wVerticeIndex + nSegments;
					*pIndices++ = wVerticeIndex + nSegments + 1;
					*pIndices++ = wVerticeIndex + 1;
					*pIndices++ = wVerticeIndex;
					wVerticeIndex ++;
				}
			}; // end for seg
		} // end for ring

		// Unlock
		vBuf->unlock();
		iBuf->unlock();
		// Generate face list
		pSphereVertex->useSharedVertices = true;

		// the original code was missing this line:
		pSphere->_setBounds( AxisAlignedBox( Vector3(-r, -r, -r), Vector3(r, r, r) ), false );
		pSphere->_setBoundingSphereRadius(r);
		// this line makes clear the mesh is loaded (avoids memory leaks)
        pSphere->load();
    }
    void createBoxMesh(const std::string& strName, const float &r)
	{
		MeshPtr pSphere = MeshManager::getSingleton().createManual(strName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		SubMesh *pSphereVertex = pSphere->createSubMesh();

		pSphere->sharedVertexData = new VertexData();
		VertexData* vertexData = pSphere->sharedVertexData;

		// define the vertex format
		VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
		size_t currOffset = 0;
		// positions
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// normals
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// two dimensional texture coordinates
		vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
		currOffset += VertexElement::getTypeSize(VET_FLOAT2);

		// allocate the vertex buffer
		vertexData->vertexCount = 8;
		HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(0, vBuf);
		float* pVertex = static_cast<float*>(vBuf->lock(HardwareBuffer::HBL_DISCARD));

		// allocate index buffer
		pSphereVertex->indexData->indexCount = 36;
		pSphereVertex->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, pSphereVertex->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		HardwareIndexBufferSharedPtr iBuf = pSphereVertex->indexData->indexBuffer;
		unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(HardwareBuffer::HBL_DISCARD));

		const float d = r*0.5f;

		for(int y=0; y<2; y++)
		{
		    //Position of vertice
            *pVertex++ = -d;
            *pVertex++ = y;
            *pVertex++ = -d;
            //Normal of vertice
            const Vector3 vNormal0 = Vector3(-d, y, -d).normalisedCopy();
            *pVertex++ = vNormal0.x;
            *pVertex++ = vNormal0.y;
            *pVertex++ = vNormal0.z;
            //UV of vertice
            *pVertex++ = 0;
            *pVertex++ = 0;

            *pVertex++ = -d;
            *pVertex++ = y;
            *pVertex++ = d;
            const Vector3 vNormal1 = Vector3(-d, y, d).normalisedCopy();
            *pVertex++ = vNormal1.x;
            *pVertex++ = vNormal1.y;
            *pVertex++ = vNormal1.z;
            *pVertex++ = 0;
            *pVertex++ = 1;

            *pVertex++ = d;
            *pVertex++ = y;
            *pVertex++ = d;
            const Vector3 vNormal2 = Vector3(d, y, d).normalisedCopy();
            *pVertex++ = vNormal2.x;
            *pVertex++ = vNormal2.y;
            *pVertex++ = vNormal2.z;
            *pVertex++ = 1;
            *pVertex++ = 1;

            *pVertex++ = d;
            *pVertex++ = y;
            *pVertex++ = -d;
            const Vector3 vNormal3 = Vector3(d, y, -d).normalisedCopy();
            *pVertex++ = vNormal3.x;
            *pVertex++ = vNormal3.y;
            *pVertex++ = vNormal3.z;
            *pVertex++ = 1;
            *pVertex++ = 0;
		}

        *pIndices++ = 0; *pIndices++ = 3; *pIndices++ = 1;  *pIndices++ = 2; *pIndices++ = 1; *pIndices++ = 3;
        *pIndices++ = 1; *pIndices++ = 5; *pIndices++ = 0;  *pIndices++ = 4; *pIndices++ = 0; *pIndices++ = 5;
        *pIndices++ = 2; *pIndices++ = 6; *pIndices++ = 1;  *pIndices++ = 5; *pIndices++ = 1; *pIndices++ = 6;
        *pIndices++ = 3; *pIndices++ = 7; *pIndices++ = 2;  *pIndices++ = 6; *pIndices++ = 2; *pIndices++ = 7;
        *pIndices++ = 0; *pIndices++ = 4; *pIndices++ = 3;  *pIndices++ = 7; *pIndices++ = 3; *pIndices++ = 4;
        *pIndices++ = 4; *pIndices++ = 5; *pIndices++ = 7;  *pIndices++ = 6; *pIndices++ = 7; *pIndices++ = 5;

		// Unlock
		vBuf->unlock();
		iBuf->unlock();
		// Generate face list
		pSphereVertex->useSharedVertices = true;

		// the original code was missing this line:
		pSphere->_setBounds( AxisAlignedBox( Vector3(-d, 0, -d), Vector3(d, r, d) ), false );
		pSphere->_setBoundingSphereRadius(r);
		// this line makes clear the mesh is loaded (avoids memory leaks)
        pSphere->load();
    }
    void createCircleMesh(const std::string& strName, const float &r)
	{
		MeshPtr pSphere = MeshManager::getSingleton().createManual(strName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		SubMesh *pSphereVertex = pSphere->createSubMesh();

		pSphere->sharedVertexData = new VertexData();
		VertexData* vertexData = pSphere->sharedVertexData;

		// define the vertex format
		VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
		size_t currOffset = 0;
		// positions
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// normals
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// two dimensional texture coordinates
		vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
		currOffset += VertexElement::getTypeSize(VET_FLOAT2);

		// allocate the vertex buffer
		vertexData->vertexCount = 13;
		HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(0, vBuf);
		float* pVertex = static_cast<float*>(vBuf->lock(HardwareBuffer::HBL_DISCARD));

		// allocate index buffer
		pSphereVertex->indexData->indexCount = 36;
		pSphereVertex->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, pSphereVertex->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		HardwareIndexBufferSharedPtr iBuf = pSphereVertex->indexData->indexBuffer;
		unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(HardwareBuffer::HBL_DISCARD));

        //Vertices around the clock
        const Radian angle = Degree(30);
        for(int seg=0;seg<12;seg++)
        {
            const Real x = Math::Sin(angle*seg)*r;
            const Real y = Math::Cos(angle*seg)*r;
            *pVertex++ = x;
            *pVertex++ = y;
            *pVertex++ = 0;
            const Vector3 vNormal = Vector3(x, y, 0).normalisedCopy();
            *pVertex++ = vNormal.x;
            *pVertex++ = vNormal.y;
            *pVertex++ = vNormal.z;
            *pVertex++ = (x+1)*0.5f;
            *pVertex++ = (y-1)*-0.5f;

            *pIndices++ = seg;  //current vertex
            *pIndices++ = (seg==11?0:seg+1);  //next vertex
            *pIndices++ = 12;  //midpoint vertex
        }
        //Midpoint vertex
        *pVertex++ = 0;
        *pVertex++ = 0;
        *pVertex++ = 0;
        const Vector3 vNormal = Vector3(0, 0, 0).normalisedCopy();
        *pVertex++ = vNormal.x;
        *pVertex++ = vNormal.y;
        *pVertex++ = vNormal.z;
        *pVertex++ = 0.5f;
        *pVertex++ = 0.5f;

		// Unlock
		vBuf->unlock();
		iBuf->unlock();
		// Generate face list
		pSphereVertex->useSharedVertices = true;

		// the original code was missing this line:
		pSphere->_setBounds( AxisAlignedBox( Vector3(-r, -r, -r), Vector3(r, r, r) ), false );
		pSphere->_setBoundingSphereRadius(r);
		// this line makes clear the mesh is loaded (avoids memory leaks)
        pSphere->load();
    }
    void setMouseQueryMask(const uint32 &flags)
    {
        mMouseQuery->setQueryMask(flags);
        mMouseQuery->setSortByDistance(true);
    }
    const pair<bool,Vector3> doMousePicking(Camera *camera, const Real &x, const Real &y, SceneNode **node=0, const bool &includeMeshColl=false, const bool &meshCollCopyMouseMask=true)
    {
        if(camera->getRealDirection().directionEquals(Vector3::UNIT_Y,Degree(1)))return pair<bool,Vector3>(false,Vector3::ZERO);
        const Ray mouseRay = camera->getCameraToViewportRay(x,y);

        Entity *meshCollEnt = 0;
        Vector3 meshCollPos = Vector3::ZERO;
        if(includeMeshColl)
        {
            if(meshCollCopyMouseMask)mMeshQuery->setQueryMask(mMouseQuery->getQueryMask());
            getMeshCollision(mouseRay,meshCollPos,0,&meshCollEnt);
            if(meshCollCopyMouseMask)mMeshQuery->setQueryMask(Collision::MESH_COLL_MASK);
        }

        mMouseQuery->setRay(mouseRay);
        RaySceneQueryResult &result = mMouseQuery->execute();
        for(RaySceneQueryResult::iterator it=result.begin(); it!=result.end(); it++)
        {
            //Hit movable
            if(it->movable && ((node&&!includeMeshColl) || meshCollEnt==it->movable))
            {
                if(node && (!includeMeshColl || it->movable->getQueryFlags()==Collision::UNIT_MASK))
                {
                    *node = it->movable->getParentSceneNode();
                    if(it->movable->getQueryFlags()==Collision::UNIT_MASK)*node = (*node)->getParentSceneNode();
                    return pair<bool,Vector3>(true,(*node)->getPosition());
                }
                else return pair<bool,Vector3>(true,meshCollPos);
            }
            //Hit terrain
            else if(it->worldFragment)return pair<bool,Vector3>(true,it->worldFragment->singleIntersection);
        }
        return pair<bool,Vector3>(false,Vector3::ZERO);
    }
    void doMousePicking(Camera *camera, const Real &x, const Real &y, std::vector<SceneNode*> *nodeList)
    {
        if(camera->getRealDirection().directionEquals(Vector3::UNIT_Y,Degree(1)))return;
        const Ray mouseRay = camera->getCameraToViewportRay(x,y);
        mMouseQuery->setRay(mouseRay);
        RaySceneQueryResult &result = mMouseQuery->execute();
        for(RaySceneQueryResult::iterator it=result.begin(); it!=result.end(); it++)
        {
            if(it->movable)
            {
                nodeList->push_back(it->movable->getParentSceneNode());
            }
            else if(it->worldFragment)return;
        }
    }
    void showCollisions(const bool &flag)
    {
        mShowCollisions = flag;
    }
    bool getMeshCollision(const Ray &ray, Vector3 &result, Plane *resultPlane=0, Entity **resultEntity=0)
    {
        mMeshQuery->setRay(ray);

        if(mMeshQuery->execute().size() <= 0)
        {
            //raycast did not hit any objects' bounding box
            return (false);
        }

        // at this point we have raycast to a series of different objects bounding boxes.
        // we need to test these different objects to see which is the first polygon hit.
        // there are some minor optimizations (distance based) that mean we wont have to
        // check all of the objects most of the time, but the worst case scenario is that
        // we need to test every triangle of every object.
        Real closest_distance = -1.0f;
        Vector3 closest_result;
        RaySceneQueryResult &query_result = mMeshQuery->getLastResults();
        for(size_t qr_idx = 0; qr_idx < query_result.size(); qr_idx++)
        {
            // stop checking if we have found a raycast hit that is closer
            // than all remaining entities
            if((closest_distance >= 0.0f) && (closest_distance < query_result[qr_idx].distance))
            {
                 break;
            }
            // only check this result if its a hit against an entity
            if ((query_result[qr_idx].movable != NULL) && (query_result[qr_idx].movable->getMovableType().compare("Entity") == 0))
            {
                // get the entity to check
                Entity *pentity = static_cast<Ogre::Entity*>(query_result[qr_idx].movable);

                // mesh data to retrieve
                size_t vertex_count;
                size_t index_count;
                Vector3 *vertices;
                unsigned long *indices;

                // get the mesh information
                getMeshInformation(pentity->getMesh(), vertex_count, vertices, index_count, indices,
                                    pentity->getParentNode()->_getDerivedPosition(),
                                    pentity->getParentNode()->_getDerivedOrientation(),
                                    pentity->getParentNode()->_getDerivedScale());

                // test for hitting individual triangles on the mesh
                bool new_closest_found = false;
                for(int i = 0; i < static_cast<int>(index_count); i += 3)
                {
                    // check for a hit against this triangle
                    std::pair<bool, Real> hit = Math::intersects(ray, vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]], true, true);

                    // if it was a hit check if its the closest
                    if(hit.first)
                    {
                        if((closest_distance < 0.0f) || (hit.second < closest_distance))
                        {
                            // this is the closest so far, save it off
                            closest_distance = hit.second;
                            new_closest_found = true;

                            if(resultPlane)resultPlane->redefine(vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]]);

                        }
                    }
                }

                // free the verticies and indicies memory
                delete[] vertices;
                delete[] indices;

                // if we found a new closest raycast for this object, update the
                // closest_result before moving on to the next object.
                if(new_closest_found)
                {
                    closest_result = ray.getPoint(closest_distance);
                    if(resultEntity)*resultEntity = pentity;
                }
            }
        }

        // return the result
        if(closest_distance >= 0.0f)
        {
            // raycast success
            result = closest_result;
            return (true);
        }
        else
        {
            // raycast failed
            if(resultEntity)*resultEntity = 0;
            return (false);
        }
    }
    void getMeshInformation(const MeshPtr mesh, size_t &vertex_count, Vector3* &vertices, size_t &index_count, unsigned long* &indices, const Vector3 &position, const Quaternion &orient, const Vector3 &scale)
    {
        bool added_shared = false;
        size_t current_offset = 0;
        size_t shared_offset = 0;
        size_t next_offset = 0;
        size_t index_offset = 0;

        vertex_count = index_count = 0;

        // Calculate how many vertices and indices we're going to need
        for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            SubMesh* submesh = mesh->getSubMesh( i );

            // We only need to add the shared vertices once
            if(submesh->useSharedVertices)
            {
                if(!added_shared)
                {
                    vertex_count += mesh->sharedVertexData->vertexCount;
                    added_shared = true;
                }
            }
            else
            {
                vertex_count += submesh->vertexData->vertexCount;
            }

            // Add the indices
            index_count += submesh->indexData->indexCount;
        }


        // Allocate space for the vertices and indices
        vertices = new Vector3[vertex_count];
        indices = new unsigned long[index_count];

        added_shared = false;

        // Run through the submeshes again, adding the data into the arrays
        for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            SubMesh* submesh = mesh->getSubMesh(i);

            VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

            if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
            {
                if(submesh->useSharedVertices)
                {
                    added_shared = true;
                    shared_offset = current_offset;
                }

                const VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(VES_POSITION);
                HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

                unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));

                // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
                //  as second argument. So make it float, to avoid trouble when Ogre::Real will
                //  be comiled/typedefed as double:
                //      Ogre::Real* pReal;
                float* pReal;

                for(size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
                {
                    posElem->baseVertexPointerToElement(vertex, &pReal);

                    Vector3 pt(pReal[0], pReal[1], pReal[2]);

                    vertices[current_offset + j] = (orient * (pt * scale)) + position;
                }

                vbuf->unlock();
                next_offset += vertex_data->vertexCount;
            }


            IndexData* index_data = submesh->indexData;
            size_t numTris = index_data->indexCount / 3;
            HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

            bool use32bitindexes = (ibuf->getType() == HardwareIndexBuffer::IT_32BIT);

            unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
            unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


            size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;
            size_t index_start = index_data->indexStart;
            size_t last_index = numTris*3 + index_start;

            if(use32bitindexes)
                for(size_t k = index_start; k < last_index; ++k)
                {
                    indices[index_offset++] = pLong[k] + static_cast<unsigned long>( offset );
                }

            else
                for(size_t k = index_start; k < last_index; ++k)
                {
                    indices[ index_offset++ ] = static_cast<unsigned long>( pShort[k] ) +
                        static_cast<unsigned long>( offset );
                }

            ibuf->unlock();
            current_offset = next_offset;
        }
    }
};

template<> CollisionManager* Singleton<CollisionManager>::ms_Singleton = 0;

CollisionManager* CollisionManager::getSingletonPtr()
{
	return ms_Singleton;
}

CollisionManager& CollisionManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
