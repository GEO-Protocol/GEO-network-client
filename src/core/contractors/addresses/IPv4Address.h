#ifndef GEO_NETWORK_CLIENT_IPV4ADDRESS_H
#define GEO_NETWORK_CLIENT_IPV4ADDRESS_H

#include "../../common/Types.h"
#include "../../common/exceptions/ValueError.h"

#include <string>

using namespace std;

class IPv4Address {
public:
    typedef shared_ptr<IPv4Address> Shared;

public:
    IPv4Address(
        const string &fullAddress);

    string host() const;

    uint16_t port() const;

    string fullAddress() const;

private:
    string mAddress;
    uint16_t mPort;
};


#endif //GEO_NETWORK_CLIENT_IPV4ADDRESS_H
