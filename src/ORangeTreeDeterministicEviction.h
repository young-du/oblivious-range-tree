//
//
//

#ifndef _ORangeTREEDETEVICTION_H
#define _ORangeTREEDETEVICTION_H
#include "ORangeTreeInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"
#include "IdGenerator.h"
#include "Exceptions.h"
#include <cmath>
#include <deque>


class ORangeTreeDeterministicEviction : public ORangeTreeInterface {
    public:
    ORangeTreeDeterministicEviction(UntrustedStorageInterface* storage,
            RandForOramInterface* rand_gen, int bucket_size, int num_blocks, int num_points);
            
    int P(int leaf, int level);

    vector<Block> getStash();
    int getStashSize();
    int getNumLeaves();
    int getNumLevels();
    int getNumBlocks();
    int getNumBuckets();
    void dumpStash();

    void build(vector<Point> v);
    
    
    vector<Point> search(double x1, double x2, double y1, double y2);
    pair<int, int> buildX (vector<Point> v);
    pair<int, int> buildY (vector<Point> v);

    private:
    UntrustedStorageInterface* storage;
    RandForOramInterface* rand_gen;
    int G;
    int bucket_size;
    int num_levels;
    int num_leaves;
    int num_blocks;
    int num_buckets;
    int num_points;

    vector<Block> stash;
    deque<Block> local;
    deque<Block> left;
    deque<Block> right;
    
    
    void evictAfterSearch();
    void evictTwoPaths();
    void evictByLeaf(int leaf);
    IdGenerator* id_gen;
    Root root;
    void moveToLocal(int leaf, int blockid);
    void moveToLeft(int leaf, int blockid);
    void moveToRight(int leaf, int blockid);
    void padding();
    int ReverseBits(int G, int bits_length);
    void performDummyFinds(int num);
    void performDummyEvictions(int num);
    pair<int, int> reportY(int y_id, int y_pos, double y1, double y2, vector<Point> &res);
    // id, pos
    
};


#endif 
