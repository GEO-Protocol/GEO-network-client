#ifndef GEO_NETWORK_CLIENT_BASEADDRESS_H
#define GEO_NETWORK_CLIENT_BASEADDRESS_H

#include "../../common/memory/MemoryUtils.h"

using namespace std;

class BaseAddress {

public:
    typedef shared_ptr<BaseAddress> Shared;
    typedef uint8_t SerializedType;

    enum AddressType {
        /*
         * IPv4
         */
        IPv4_Clear = 11,
        IPv4_IncludingPort = 12,

        /*
         * IPv6
         */

        /*
         * DNS
         */

        /*
         * GNS
         */
    };

    virtual const AddressType typeID() const = 0;

    virtual BytesShared serializeToBytes() const = 0;

    virtual size_t serializedSize() const = 0;
};


#endif //GEO_NETWORK_CLIENT_BASEADDRESS_H
