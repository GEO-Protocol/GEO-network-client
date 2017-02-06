#ifndef GEO_NETWORK_CLIENT_MANAGER_H
#define GEO_NETWORK_CLIENT_MANAGER_H


#include "SecondLevelRoutingTable.h"
#include "ThirdLevelRoutingTable.h"

#include <memory>


using namespace std;


class RoutingTablesHandler {
public:

protected:
    unique_ptr<SecondLevelRoutingTable> mSecondLevelRT;
    unique_ptr<ThirdLevelRoutingTable> mSecondLevelRT;
};


#endif //GEO_NETWORK_CLIENT_MANAGER_H
