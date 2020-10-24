//
//
//
#include "Block.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

Block::Block(const Block& b) {
    for (int i = 0; i<BLOCK_SIZE; i++) {
		this->data[i] = b.data[i];
	}
    this->leaf_id = b.leaf_id;
    this->id = b.id;
    this->leftChildPos = b.leftChildPos;
    this->leftChildId = b.leftChildId;
    this->rightChildPos = b.rightChildPos;
    this->rightChildId = b.rightChildId;
    this->type = b.type;
    this->YrootPos = b.YrootPos;
    this->YrootId = b.YrootId;
    this->isLeaf = b.isLeaf;
    this->key = b.key;
}

Block::Block() { }
Block::~Block() { }

Block::Block(int leaf_id, int id, double data[]) : leaf_id(leaf_id), id(id)
{
   for (int i = 0; i < BLOCK_SIZE; i++){
       this->data[i] = data[i];
   }
}

