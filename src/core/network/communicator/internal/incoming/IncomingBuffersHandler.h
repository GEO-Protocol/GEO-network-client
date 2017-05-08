#ifndef GEO_NETWORK_CLIENT_INCOMMINBUFFERSHANDLER_H
#define GEO_NETWORK_CLIENT_INCOMMINBUFFERSHANDLER_H

#include "../common/Types.h"
#include "../../../../common/Types.h"

//#include <boost/unordered/unordered_map.hpp>
#include <boost/functional/hash/hash.hpp>

#include <vector>
#include <map>
#include <utility>


using namespace std;


/*
 * This class handles buffers per endpoints.
 *
 * Each endpoint uses it's own buffer for incoming bytes sequence.
 * In case if some endpoint will send invalid data - only it's buffer would be broken,
 * and only it's messages would not collect.
 */
class IncomingBuffersHandler {
public:
    vector<byte>* buffer(
        const UDPEndpoint &endpoint)
        noexcept;

    void dropBuffer(
        const UDPEndpoint &endpoint)
        noexcept;

protected:
    static uint64_t key(
        const UDPEndpoint &endpoint)
        noexcept;

protected:
    map<uint64_t, unique_ptr<vector<byte>>> mIncomingBuffers;
};


#endif //GEO_NETWORK_CLIENT_INCOMMINBUFFERSHANDLER_H
