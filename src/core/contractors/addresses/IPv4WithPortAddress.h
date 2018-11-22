#ifndef GEO_NETWORK_CLIENT_IPV4WITHPORTADDRESS_H
#define GEO_NETWORK_CLIENT_IPV4WITHPORTADDRESS_H

#include "BaseAddress.h"
#include "../../common/Types.h"
#include "../../network/communicator/internal/common/Types.h"
#include "../../common/exceptions/ValueError.h"

#include <string>

class IPv4WithPortAddress : public BaseAddress {
public:
    typedef shared_ptr<IPv4WithPortAddress> Shared;

public:
    IPv4WithPortAddress(
        const string &fullAddress);

    IPv4WithPortAddress(
        const Host &host,
        const Port port);

    IPv4WithPortAddress(
        byte* buffer);

    string host() const;

    Port port() const;

    string fullAddress() const;

    const AddressType typeID() const override;

    BytesShared serializeToBytes() const override;

    size_t serializedSize() const override;

private:
    byte mAddress[4];
    Port mPort;
};


#endif //GEO_NETWORK_CLIENT_IPV4WITHPORTADDRESS_H
