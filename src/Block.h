//
//
//

#ifndef _BLOCK_H
#define _BLOCK_H

#include <algorithm>
using namespace std;

enum Nodetype {X,Y};

struct Point {
    double x;
    double y;
};


class Block {
    public:
    static const int BLOCK_SIZE = 4;
    int leaf_id = -1;
    int id = -1;
    Nodetype type = X;
    double data[BLOCK_SIZE];
    double key;
    int leftChildPos = -1;
    int leftChildId = -1;
    int rightChildPos = -1;
    int rightChildId = -1;
    int YrootPos = -1;
    int YrootId = -1;
    bool isLeaf = false;
    Block();
    Block(const Block& b);
    Block(int leaf_id, int id, double data[]);
    virtual ~Block();
};

#endif //_BLOCK_H
