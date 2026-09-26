#ifndef _GRIDMANAGER_H_
#define _GRIDMANAGER_H_

#define GRID_WIDTH 2500

#include <CameraManager.h>
#include <Grid.h>

class GridManager : public Singleton<GridManager>
{
private:
    std::vector<Grid*> mGridList;
    Grid *mActiveGrid;
    Real mViewDistanceRatio;
public:
    GridManager()
    {
        mViewDistanceRatio = 0.667f;
        clear();
    }
    ~GridManager()
    {
        clear();
    }
    void clear()
    {
        while(!mGridList.empty())
        {
            Grid *grid = mGridList.back();
            mGridList.pop_back();
            delete grid;
        }
        mActiveGrid = 0;
    }
    void init(SceneManager *sceneMgr, const Vector2 &mapSize)
    {
        const unsigned short width = Math::ICeil(mapSize.x/GRID_WIDTH);
        const unsigned short height = Math::ICeil(mapSize.y/GRID_WIDTH);
        SceneNode *rootNode = sceneMgr->getRootSceneNode();

        for(int y=0;y<height;y++)
            for(int x=0;x<width;x++)
            {
                Grid *grid = new Grid(x,y);
                grid->setStaticGeometry(sceneMgr->createStaticGeometry("SG_"+StringConverter::toString(x)+"_"+StringConverter::toString(y)));
                grid->setSceneNode(rootNode->createChildSceneNode());
                grid->setRootNode(rootNode);
                //Starting from North, clockwise, 0-7
                //North
                if(y>0)
                {
                    grid->setNeighbour(0,getGrid(x,y-1));
                    if(grid->getNeighbour(0))grid->getNeighbour(0)->setNeighbour(4,grid);
                }
                //West
                if(x>0)
                {
                    grid->setNeighbour(6,getGrid(x-1,y));
                    if(grid->getNeighbour(6))grid->getNeighbour(6)->setNeighbour(2,grid);
                }
                //NorthEast
                if(y>0 && x+1<width)
                {
                    grid->setNeighbour(1,getGrid(x+1,y-1));
                    if(grid->getNeighbour(1))grid->getNeighbour(1)->setNeighbour(5,grid);
                }
                //NorthWest
                if(x>0 && y>0)
                {
                    grid->setNeighbour(7,getGrid(x-1,y-1));
                    if(grid->getNeighbour(7))grid->getNeighbour(7)->setNeighbour(3,grid);
                }

                mGridList.push_back(grid);
            }
    }
    void update(const Vector3 &camPos)
    {
        if(!mActiveGrid)
        {
            mActiveGrid = getCurrentGrid(camPos.x,camPos.z);
            if(mActiveGrid)
            {
                mActiveGrid->activate();
                mActiveGrid->activateNeighbours();
            }
            return;
        }

        if(!mActiveGrid->isInGrid(camPos.x,camPos.z))
        {
            Grid* nextGrid = getCurrentGrid(camPos.x,camPos.z);
            if(nextGrid)
            {
                const unsigned char dir = mActiveGrid->compareDirection(nextGrid);
                mActiveGrid->deactivateNeighbours(dir);
                if(dir==0)mActiveGrid->deactivate();
                mActiveGrid = nextGrid;
                if(dir==0)mActiveGrid->activate();
                mActiveGrid->activateNeighbours(dir);
            }
        }
    }
    Grid* getGrid(const unsigned short &x, const unsigned short &y)
    {
        for(std::vector<Grid*>::iterator i=mGridList.begin(); i!=mGridList.end(); i++)
        {
            Grid *grid = *i;
            if(grid->getX()==x && grid->getY()==y)return grid;
        }
        return 0;
    }
    Grid* getGridContaining(const Real &x, const Real &y)
    {
        return getGrid((int)x/GRID_WIDTH,(int)y/GRID_WIDTH);
    }
    Grid* getCurrentGrid(const Real &camX, const Real &camY)
    {
        if(camX<0 || camY<0)return 0;

        const unsigned short x = (int)camX/GRID_WIDTH;
        const unsigned short y = (int)camY/GRID_WIDTH;
        Grid *grid = 0;
        if(mActiveGrid)grid = mActiveGrid->getContainingNeighbour(x,y);
        if(!grid)grid = getGrid(x,y);
        return grid;
    }
    Grid *getActiveGrid()
    {
        return mActiveGrid;
    }
    void buildAllGeometries()
    {
        const Real distance = mViewDistanceRatio*9000.0f+1000.0f;
        for(std::vector<Grid*>::iterator i=mGridList.begin(); i!=mGridList.end(); i++)
        {
            Grid *grid = *i;
            grid->setRenderingDistance(distance);
            grid->buildGeometry();
        }
    }
    static GridManager* getSingletonPtr();
    static GridManager& getSingleton();
    const Real getViewDistanceRatio()
    {
        return mViewDistanceRatio;
    }
    void setViewDistanceRatio(const Real &ratio)
    {
        mViewDistanceRatio = ratio;
        const Real distance = ratio*9000.0f+1000.0f;
        for(std::vector<Grid*>::iterator i=mGridList.begin(); i!=mGridList.end(); i++)
        {
            Grid *grid = *i;
            grid->setRenderingDistance(distance);
        }
    }
};

template<> GridManager* Singleton<GridManager>::ms_Singleton = 0;

GridManager* GridManager::getSingletonPtr()
{
	return ms_Singleton;
}

GridManager& GridManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
