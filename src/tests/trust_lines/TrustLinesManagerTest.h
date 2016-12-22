#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGERTEST_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGERTEST_H

#include <iostream>
#include <string>
#include "../../core/trust_lines/manager/TrustLinesManager.h"

using namespace std;

class TrustLinesManagerTest {

public:
    NodeUUID contractor1;
    NodeUUID contractor2;
    NodeUUID contractor3;
    NodeUUID contractor4;

    TrustLinesManager *mTrustLinesManager;

public:

    TrustLinesManagerTest();

    void openSuccessTestCase();

    void acceptSuccessTestCase();

    void closeSuccessTestCase();

    void rejectSuccessTestCase();

    void deletePointer();

    void showAllTrustLines();

    void run();
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGERTEST_H
