//
//
//

#include "ORangeTreeDeterministicEviction.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>


ORangeTreeDeterministicEviction::ORangeTreeDeterministicEviction(UntrustedStorageInterface* storage, RandForOramInterface* rand_gen, int bucket_size, int num_blocks, int num_points) {
    this->G = 0;
    this->storage = storage;
    this->rand_gen = rand_gen;
    this->bucket_size = bucket_size;
    this->num_blocks = num_blocks;
    this->num_points = num_points;
    this->num_levels = ceil(log10(num_blocks) / log10(2)) + 1;
    this->num_buckets = pow(2, num_levels)-1;
    if (this->num_buckets*this->bucket_size < this->num_blocks) //deal with precision loss
    {
        throw new runtime_error("Not enough space for the acutal number of blocks.");
    }
    this->num_leaves = pow(2, num_levels-1);
    Bucket::resetState();
    Bucket::setMaxSize(bucket_size);
    this->rand_gen->setBound(num_leaves);
    this->storage->setCapacity(num_buckets);
    this->id_gen = new IdGenerator;
    this->stash = vector<Block>();
    

    for(int i = 0; i < num_buckets; i++){

        Bucket init_bkt = Bucket();
        for(int j = 0; j < bucket_size; j++){
            init_bkt.addBlock(Block());
        }
        storage->writeBucket(i, Bucket(init_bkt));
    }

}

void ORangeTreeDeterministicEviction::moveToLocal(int leaf, int blockid) {
    bool inOram = false;
    for (int i = 0; i < num_levels; i++) {
        vector<Block> blocks = storage->readBucket(P(leaf, i)).getBlocks();
        Bucket writeBack;
        for (Block &b: blocks) {
            if (b.id == blockid) {
                local.push_back(Block(b));
                inOram = true;
                writeBack.addBlock(Block());
            } else writeBack.addBlock(b);
        }
        storage->writeBucket(P(leaf, i), writeBack);
    }
    if (inOram == false) {
        for (auto it = stash.begin(); it != stash.end(); it++) {
            if (it->id == blockid) {
                local.push_back(*it);
                stash.erase(it);
                break;
            } 
        }
    }
}

void ORangeTreeDeterministicEviction::moveToLeft(int leaf, int blockid) {
    bool inOram = false;
    for (int i = 0; i < num_levels; i++) {
        vector<Block> blocks = storage->readBucket(P(leaf, i)).getBlocks();
        Bucket writeBack;
        for (Block &b: blocks) {
            if (b.id == blockid) {
                left.push_back(Block(b));
                inOram = true;
                writeBack.addBlock(Block());
            } else writeBack.addBlock(b);
        }
        storage->writeBucket(P(leaf, i), writeBack);
    }
    if (inOram == false) {
        for (auto it = stash.begin(); it != stash.end(); it++) {
            if (it->id == blockid) {
                left.push_back(*it);
                stash.erase(it);
                break;
            }
        }
    }
}

void ORangeTreeDeterministicEviction::moveToRight(int leaf, int blockid) {
    bool inOram = false;
    for (int i = 0; i < num_levels; i++) {
        vector<Block> blocks = storage->readBucket(P(leaf, i)).getBlocks();
        Bucket writeBack;
        for (Block &b: blocks) {
            if (b.id == blockid) {
                right.push_back(Block(b));
                inOram = true;
                writeBack.addBlock(Block());
            } else writeBack.addBlock(b);
        }
        storage->writeBucket(P(leaf, i), writeBack);
    }
    if (inOram == false) {
        for (auto it = stash.begin(); it != stash.end(); it++) {
            if (it->id == blockid) {
                right.push_back(*it);
                stash.erase(it);
                break;
            }
        }
    }
}

void ORangeTreeDeterministicEviction::padding() {
    int leaf = rand_gen->getRandomLeaf();
    for (int i = 0; i < num_levels; i++) {
        vector<Block> blocks = storage->readBucket(P(leaf, i)).getBlocks();
        Bucket writeBack;
        for (Block &b: blocks) {
            writeBack.addBlock(b);
        }
        storage->writeBucket(P(leaf, i), writeBack);
    }
    evictTwoPaths();
}

// ================  build  =============================

bool compy(const Point& a, const Point& b) {
    return a.y < b.y;
}
bool compx(const Point& a, const Point& b) {
    return a.x < b.x;
}

void ORangeTreeDeterministicEviction::build(vector<Point> v) {
    sort(v.begin(), v.end(), compx);
    auto r = buildX(v);
    root.id = r.first;
    root.leaf_id = r.second;
    int s = (int) stash.size();
    for (int i = 0; i <= s * s; ++ i) {
        evictTwoPaths();
    }
}


pair<int, int> ORangeTreeDeterministicEviction::buildX (vector<Point> v) {
    Block b;
    b.id = id_gen->getNextId();
    b.leaf_id = rand_gen->getRandomLeaf();
    b.type = X;
    b.leftChildPos = -1;
    b.leftChildId = -1;
    b.rightChildPos = -1;
    b.rightChildId = -1;
    b.YrootPos = -1;
    b.YrootId = -1;
    b.isLeaf = false;
    if (v.size() == 1) {
        b.data[0] = v[0].x;
        b.data[1] = v[0].y;
        b.isLeaf = true;
        stash.push_back(Block(b));
        return pair<int, int>(b.id, b.leaf_id);
    }
    size_t split = v.size() / 2;
    b.data[0] = v[split].x;
    b.data[1] = v[split].y;
    b.key = (v[split].x + v[split - 1].x) / 2;
    auto left = buildX(vector<Point>(v.begin(), v.begin()+split));
    b.leftChildId = left.first;
    b.leftChildPos = left.second;
    auto right = buildX(vector<Point>(v.begin()+split, v.end()));
    b.rightChildId = right.first;
    b.rightChildPos = right.second;
    vector<Point> w(v);
    sort(w.begin(), w.end(), compy);
    auto y = buildY(w);
    b.YrootId = y.first;
    b.YrootPos = y.second;
    stash.push_back(Block(b));
    return pair<int, int>(b.id, b.leaf_id);
}
pair<int, int> ORangeTreeDeterministicEviction::buildY (vector<Point> v) {
    if (v.size() == 0) return pair<int, int>(-1, -1);
    Block b;
    b.id = id_gen->getNextId();
    b.leaf_id = rand_gen->getRandomLeaf();
    b.type = Y;
    b.leftChildPos = -1;
    b.leftChildId = -1;
    b.rightChildPos = -1;
    b.rightChildId = -1;
    b.YrootPos = -1;
    b.YrootId = -1;
    b.isLeaf = false;
    if (v.size() == 1) {
        b.data[0] = v[0].x;
        b.data[1] = v[0].y;
        b.isLeaf = true;
        stash.push_back(Block(b));
        return pair<int, int>(b.id, b.leaf_id);
    }
    size_t split = v.size() / 2;
    b.data[0] = v[split].x;
    b.data[1] = v[split].y;
    auto left = buildY(vector<Point>(v.begin(), v.begin()+split));
    b.leftChildId = left.first;
    b.leftChildPos = left.second;
    auto right = buildY(vector<Point>(v.begin()+split+1, v.end()));
    b.rightChildId = right.first;
    b.rightChildPos = right.second;
    if (b.leftChildId == -1 && b.rightChildId == -1) b.isLeaf = true;
    stash.push_back(Block(b));
    return pair<int, int>(b.id, b.leaf_id);
}





// ================  search  =============================
vector<Point> ORangeTreeDeterministicEviction::search(double x1, double x2, double y1, double y2) {
    storage->numReadings = 0;
    vector<Point> res;
    moveToLocal(root.leaf_id, root.id);
    local[0].leaf_id = rand_gen->getRandomLeaf();
    local[0].id = id_gen->getNextId();
    root.leaf_id = local[0].leaf_id;
    root.id = local[0].id;
    while (true) {
        if (local[0].isLeaf) {
            if (local[0].data[0] >= x1 && local[0].data[0] <= x2
                && local[0].data[1] >= y1 && local[0].data[1] <= y2 ) {
                res.push_back(Point{local[0].data[0], local[0].data[1]});
            }
            stash.push_back(local[0]);
            local.pop_front();
            evictTwoPaths();
            return res;
        }
        if (local[0].key <= x1) {
            // go right
            moveToLocal(local[0].rightChildPos, local[0].rightChildId);
            local[1].leaf_id = rand_gen->getRandomLeaf();
            local[1].id = id_gen->getNextId();
            local[0].rightChildPos = local[1].leaf_id;
            local[0].rightChildId = local[1].id;
            stash.push_back(local[0]);
            local.pop_front();
            evictTwoPaths();
        } else if (local[0].key >= x2) {
            // go left
            moveToLocal(local[0].leftChildPos, local[0].leftChildId);
            local[1].leaf_id = rand_gen->getRandomLeaf();
            local[1].id = id_gen->getNextId();
            local[0].leftChildPos = local[1].leaf_id;
            local[0].leftChildId = local[1].id;
            stash.push_back(local[0]);
            local.pop_front();
            evictTwoPaths();
        } else break;
    }
    // at this point, local.size = 1, local[0] is the split node
    moveToLeft(local[0].leftChildPos, local[0].leftChildId);
    moveToRight(local[0].rightChildPos, local[0].rightChildId);
    left[0].leaf_id = rand_gen->getRandomLeaf();
    left[0].id = id_gen->getNextId();
    right[0].leaf_id = rand_gen->getRandomLeaf();
    right[0].id = id_gen->getNextId();
    local[0].leftChildPos = left[0].leaf_id;
    local[0].leftChildId = left[0].id;
    local[0].rightChildPos = right[0].leaf_id;
    local[0].rightChildId = right[0].id;
    stash.push_back(local[0]);
    local.pop_front();
    evictTwoPaths();
    
    // at this point local is empty;
    
    //do the left subtree
    while (true) {
        if (left[0].isLeaf) {
            if (left[0].data[0] >= x1 && left[0].data[0] <= x2
                && left[0].data[1] >= y1 && left[0].data[1] <= y2 ) {
                res.push_back(Point{left[0].data[0], left[0].data[1]});
            }
            stash.push_back(left[0]);
            left.pop_front();
            evictTwoPaths();
            break;
        }
        if (left[0].key >= x1) {
            // go left
            moveToLeft(left[0].leftChildPos, left[0].leftChildId);
            left[1].leaf_id = rand_gen->getRandomLeaf();
            left[1].id = id_gen->getNextId();
            left[0].leftChildPos = left[1].leaf_id;
            left[0].leftChildId = left[1].id;
            //report right subtree
            moveToLeft(left[0].rightChildPos, left[0].rightChildId);
            left[2].leaf_id = rand_gen->getRandomLeaf();
            left[2].id = id_gen->getNextId();
            left[0].rightChildPos = left[2].leaf_id;
            left[0].rightChildId = left[2].id;
            if (left[2].isLeaf) {
                if (left[2].data[0] >= x1 && left[2].data[0] <= x2
                    && left[2].data[1] >= y1 && left[2].data[1] <= y2 ) {
                    res.push_back(Point{left[2].data[0], left[2].data[1]});
                }
                stash.push_back(left[2]);
                left.pop_back();
                evictTwoPaths();
            } else {
                auto t = reportY(left[2].YrootId, left[2].YrootPos, y1, y2, res);
                left[2].YrootId = t.first;
                left[2].YrootPos = t.second;
                stash.push_back(left[2]);
                left.pop_back();
                evictTwoPaths();
            }
            stash.push_back(left[0]);
            left.pop_front();
            evictTwoPaths();
        } else {
            // go right
            moveToLeft(left[0].rightChildPos, left[0].rightChildId);
            left[1].leaf_id = rand_gen->getRandomLeaf();
            left[1].id = id_gen->getNextId();
            left[0].rightChildPos = left[1].leaf_id;
            left[0].rightChildId = left[1].id;
            stash.push_back(left[0]);
            left.pop_front();
            evictTwoPaths();
        }
    }
    //do the right subtree
    while (true) {
        if (right[0].isLeaf) {
            if (right[0].data[0] >= x1 && right[0].data[0] <= x2
                && right[0].data[1] >= y1 && right[0].data[1] <= y2 ) {
                res.push_back(Point{right[0].data[0], right[0].data[1]});
            }
            stash.push_back(right[0]);
            right.pop_front();
            evictTwoPaths();
            break;
        }
        if (right[0].key <= x2) {
            // go right
            moveToRight(right[0].rightChildPos, right[0].rightChildId);
            right[1].leaf_id = rand_gen->getRandomLeaf();
            right[1].id = id_gen->getNextId();
            right[0].rightChildPos = right[1].leaf_id;
            right[0].rightChildId = right[1].id;
            //report left subtree
            moveToRight(right[0].leftChildPos, right[0].leftChildId);
            right[2].leaf_id = rand_gen->getRandomLeaf();
            right[2].id = id_gen->getNextId();
            right[0].leftChildPos = right[2].leaf_id;
            right[0].leftChildId = right[2].id;
            if (right[2].isLeaf) {
                if (right[2].data[0] >= x1 && right[2].data[0] <= x2
                    && right[2].data[1] >= y1 && right[2].data[1] <= y2 ) {
                    res.push_back(Point{right[2].data[0], right[2].data[1]});
                }
                stash.push_back(right[2]);
                right.pop_back();
                evictTwoPaths();
            } else {
                auto t = reportY(right[2].YrootId, right[2].YrootPos, y1, y2, res);
                right[2].YrootId = t.first;
                right[2].YrootPos = t.second;
                stash.push_back(right[2]);
                right.pop_back();
                evictTwoPaths();
            }
            stash.push_back(right[0]);
            right.pop_front();
            evictTwoPaths();
        } else {
            // go left
            moveToRight(right[0].leftChildPos, right[0].leftChildId);
            right[1].leaf_id = rand_gen->getRandomLeaf();
            right[1].id = id_gen->getNextId();
            right[0].leftChildPos = right[1].leaf_id;
            right[0].leftChildId = right[1].id;
            stash.push_back(right[0]);
            right.pop_front();
            evictTwoPaths();
        }
    }
    // padding
    int logn = ceil(log10(num_points) / log10(2));
    int r = 4*logn+2*logn*logn+(int)res.size();
    while (storage->numReadings < 3*this->num_levels*r) {
        padding();
    }
    return res;
}

pair<int, int> ORangeTreeDeterministicEviction::reportY(int y_id, int y_pos, double y1, double y2, vector<Point> &res) {
    if (y_id == -1 && y_pos == -1) return pair<int,int>(-1, -1);
    Block local;
    bool inOram = false;
    for (int i = 0; i < num_levels; i++) {
        vector<Block> blocks = storage->readBucket(P(y_pos, i)).getBlocks();
        Bucket writeBack;
        for (Block &b: blocks) {
            if (b.id == y_id) {
                local = Block(b);
                inOram = true;
                writeBack.addBlock(Block());
            } else writeBack.addBlock(b);
        }
        storage->writeBucket(P(y_pos, i), writeBack);
    }
    if (inOram == false) {
        for (auto it = stash.begin(); it != stash.end(); it++) {
            if (it->id == y_id) {
                local = Block(*it);
                stash.erase(it);
                break;
            }
        }
    }
    local.leaf_id = rand_gen->getRandomLeaf();
    local.id = id_gen->getNextId();
    if (local.isLeaf) {
        if (local.data[1] >= y1 && local.data[1] <= y2) {
            res.push_back(Point{local.data[0], local.data[1]});
        }
    } else {
        if (local.data[1] >= y1 && local.data[1] <= y2) {
            res.push_back(Point{local.data[0], local.data[1]});
            auto le = reportY(local.leftChildId, local.leftChildPos, y1, y2, res);
            local.leftChildId = le.first;
            local.leftChildPos = le.second;
            auto ri = reportY(local.rightChildId, local.rightChildPos, y1, y2, res);
            local.rightChildId = ri.first;
            local.rightChildPos = ri.second;
        } else if (local.data[1] < y1) {
            auto ri = reportY(local.rightChildId, local.rightChildPos, y1, y2, res);
            local.rightChildId = ri.first;
            local.rightChildPos = ri.second;
        } else {
            auto le = reportY(local.leftChildId, local.leftChildPos, y1, y2, res);
            local.leftChildId = le.first;
            local.leftChildPos = le.second;
        }
    }
    stash.push_back(local);
    evictTwoPaths();
    return pair<int,int>(local.id, local.leaf_id);
}


void ORangeTreeDeterministicEviction::evictTwoPaths() {
    int g_1 = ReverseBits(this->G, num_levels - 1);
    this->G += 1;
    this->evictByLeaf(g_1);
    int g_2 = ReverseBits(this->G, num_levels - 1);
    this->G += 1;
    this->evictByLeaf(g_2);
}

void ORangeTreeDeterministicEviction::evictByLeaf(int leaf) {
    for (int l = 0; l < num_levels; l++) {
      std::vector<Block> blocks = this->storage->readBucket(this->P(leaf, l)).getBlocks();
      for (const Block& b : blocks) {
        if (b.id != -1) {
          this->stash.push_back(b);
        }
      }
    }
    for (int l = num_levels-1; l >= 0; l--) {
      auto evictedId = std::vector<int>();
      Bucket bucket;
      int Pxl = P(leaf, l);
      int counter = 0;

      for (const Block& be_evicted : this->stash) {
        if (counter >= this->bucket_size) {
          break;
        }
        if (Pxl == this->P(be_evicted.leaf_id, l)) {
          bucket.addBlock(be_evicted);
          evictedId.push_back(be_evicted.id);
          counter++;
        }
      }

      // removing from the stash; careful not to modify the stash while iterating over it
      for (int i = 0; i < evictedId.size(); i++) {
        for (int j = 0; j < this->stash.size(); j++) {
          if (this->stash.at(j).id == evictedId.at(i)) {
            this->stash.erase(this->stash.begin() + j);
            break;
          }
        }
      }

      // pad out the bucket with dummy blocks
      while (counter < bucket_size) {
        bucket.addBlock(Block());
        counter++;
      }

      this->storage->writeBucket(Pxl, bucket);
    }
}

int ORangeTreeDeterministicEviction::P(int leaf, int level) {
    /*
    * This function should be deterministic. 
    * INPUT: leaf in range 0 to num_leaves - 1, level in range 0 to num_levels - 1. 
    * OUTPUT: Returns the location in the storage of the bucket which is at the input level and leaf.
    */
    return (1<<level) - 1 + (leaf >> (this->num_levels - level - 1));
}


/*
The below functions are to access various parameters, as described by their names.
INPUT: No input
OUTPUT: Value of internal variables given in the name.
*/


vector<Block> ORangeTreeDeterministicEviction::getStash() {
    return this->stash;
}
    
int ORangeTreeDeterministicEviction::getStashSize() {
    return (int)(this->stash).size();
}
    
int ORangeTreeDeterministicEviction::getNumLeaves() {
    return this->num_leaves;

}

int ORangeTreeDeterministicEviction::getNumLevels() {
    return this->num_levels;

}

int ORangeTreeDeterministicEviction::getNumBlocks() {
    return this->num_blocks;

}

int ORangeTreeDeterministicEviction::getNumBuckets() {
    return this->num_buckets;

}

void ORangeTreeDeterministicEviction::dumpStash() {
    for (auto bl: getStash()) {
        if (bl.id != -1) {
            cout << bl.id << " " << bl.data[0] << " "<< bl.data[1] << " "<< bl.data[2] << " "<< bl.data[3]  << " "<< bl.leftChildId  << " "<< bl.rightChildId << endl;
        }
    }
}

int ORangeTreeDeterministicEviction::ReverseBits(int g, int bits_length) {
    /*
    INPUT: Integers g and bits_length.
    OUTPUT: Integer reverse of length bits_length consisting of reversed bits of g.
    To be used to traverse the leaves and run eviction in reverse lexicographical order.
    */
    int g_mod = g % num_leaves;
    int reverse = 0;
    while(g_mod) {
        reverse <<= 1;
        reverse |= g_mod & 1;
        g_mod >>= 1;
        bits_length--;
    }
    
    reverse <<= bits_length;
    return reverse;
}

void ORangeTreeDeterministicEviction::performDummyFinds(int num) {
    for (int i = 0; i < num; ++i) {
        auto leaf = rand_gen->getRandomLeaf();
        for (int i = 0; i < num_levels; i++) {
            vector<Block> blocks = storage->readBucket(P(leaf, i)).getBlocks();
            Bucket writeBack;
            for (Block &b: blocks) {
                writeBack.addBlock(b);
            }
            storage->writeBucket(P(leaf, i), writeBack);
        }
    }
}
void ORangeTreeDeterministicEviction::performDummyEvictions(int num) {
    for (int i = 0; i < num; ++i) evictTwoPaths();
}
