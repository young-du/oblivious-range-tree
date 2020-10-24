//
//
//

#ifndef _ORangeTREEINTERFACE_H
#define _ORangeTREEINTERFACE_H
#include "Block.h"
#include <vector>

class ORangeTreeInterface {
public:
    struct Root
    {
        int id = -1;
        int leaf_id = -1;
    };
    
    virtual void build(vector<Point>) = 0;
    virtual vector<Point> search(double x1, double x2, double y1, double y2) = 0;

    virtual int P(int leaf, int level) = 0;
    
    virtual vector<Block> getStash() = 0;
    virtual int getStashSize() = 0;
    virtual int getNumLeaves() = 0;
    virtual int getNumLevels() = 0;
    virtual int getNumBlocks() = 0;
    virtual int getNumBuckets() = 0;
    virtual void dumpStash() = 0;
};


#endif //_ORangeTREEINTERFACE_H
