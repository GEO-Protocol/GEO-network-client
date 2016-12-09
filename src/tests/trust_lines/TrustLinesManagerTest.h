#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGERTEST_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGERTEST_H

#include <iostream>
#include <string>
#include "../../core/trust_lines/TrustLinesManager.h"

using namespace std;

class TrustLinesManagerTest {

public:

    void openSuccessTestCase();

    void acceptSuccessTestCase();

    void closeSuccessTestCase();

    void rejectSuccessTestCase();

    void run();
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGERTEST_H
