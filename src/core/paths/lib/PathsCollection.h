#ifndef GEO_NETWORK_CLIENT_PATHSCOLLECTION_H
#define GEO_NETWORK_CLIENT_PATHSCOLLECTION_H

#include "Path.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/IndexError.h"

#include <vector>

class PathsCollection {

public:
    typedef shared_ptr<PathsCollection> Shared;

public:
    PathsCollection(
        BaseAddress::Shared destinationAddress);

    void add(
        Path::Shared &path);

    void resetCurrentPath();

    Path::Shared nextPathNew();

    bool hasNextPathNew();

    size_t count() const;

private:
    BaseAddress::Shared mDestinationNodeNew;
    vector<Path::Shared> mPaths;
    size_t mCurrentPath;
};


#endif //GEO_NETWORK_CLIENT_PATHSCOLLECTION_H
