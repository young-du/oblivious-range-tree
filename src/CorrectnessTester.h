#include <iostream>
#include <cmath>
#include "Bucket.h"
#include "Block.h"
#include "RandomForOram.h"
#include "ORangeTreeDeterministicEviction.h"
#include "ORangeTreeInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"
#include "ServerStorage.h"

using namespace std;

class CorrectnessTester {
    public:
    CorrectnessTester() { };
    void runTester1();
    void runTester2();
};
