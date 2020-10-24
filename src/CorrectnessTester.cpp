#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include "Bucket.h"
#include <stdlib.h>
#include "Block.h"
#include "RandomForOram.h"
#include "ORangeTreeDeterministicEviction.h"
#include "ORangeTreeInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"
#include "ServerStorage.h"
#include "CorrectnessTester.h"

using namespace std;


void CorrectnessTester::runTester2() {

    int bucketSize = 2;
    int numPoints = pow(2, 7);
    int N = numPoints;
    Bucket::setMaxSize(bucketSize);    

    UntrustedStorageInterface* storage = new ServerStorage();
    RandForOramInterface* random = new RandomForOram();
    ORangeTreeInterface* ORange = new ORangeTreeDeterministicEviction(storage, random, bucketSize, N*(2+log2(N)), N);

    cout << "Test begins..." << endl;
    vector<Point> v;
    vector<double> w,z;
    srand(0);
    for (double i = 0 ; i < N; ++i) {
        w.push_back(i);
        z.push_back(i);
    }
    shuffle(w.begin(), w.end(),default_random_engine(3));
    shuffle(z.begin(), z.end(),default_random_engine(7));
    for (int i = 0 ; i < N; ++i) {
        v.push_back(Point{w[i],z[i]});
    }
    ORange->build(v);
    cout << "Finish building: " << endl;

    for (double _ = 0; _ < 200; ++ _) {
        double x1 = rand() % N;
        double x2 = rand() % N;
        if (x1 > x2) swap(x1, x2);

        double y1 = rand() % N;
        double y2 = rand() % N;
        if (y1 > y2) swap(y1, y2);

        x1 -= 0.5; x2 += 0.5;
        y1 -= 0.5; y2 += 0.5;
        storage->numReadings = 0;
        auto res = ORange->search(x1, x2, y1, y2);

        cout << res.size() << "; ";
        cout << ORange->getStashSize();
        cout << endl;
    }

    
}
