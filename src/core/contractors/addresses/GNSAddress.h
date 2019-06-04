#ifndef GEO_NETWORK_CLIENT_GNSADDRESS_H
#define GEO_NETWORK_CLIENT_GNSADDRESS_H

#include "BaseAddress.h"
#include "../../common/Constraints.h"
#include "../../network/communicator/internal/common/Types.h"
#include "../../common/exceptions/ValueError.h"

#include <string>

class GNSAddress : public BaseAddress {

public:
    typedef shared_ptr<GNSAddress> Shared;

public:
    GNSAddress(
        const string &fullAddress);

    GNSAddress(
        byte* buffer);

    const string host() const override;

    const Port port() const override;

    const string fullAddress() const override;

    const AddressType typeID() const override;

    BytesShared serializeToBytes() const override;

    size_t serializedSize() const override;

    void setIPAndPort(
        const string& providerData);

    const string name() const;

    const string provider() const;

private:
    string mProvider;
    string mName;
    string mHost;
    Port mPort;
};


#endif //GEO_NETWORK_CLIENT_GNSADDRESS_H
