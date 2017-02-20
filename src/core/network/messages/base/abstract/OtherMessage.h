#ifndef GEO_NETWORK_CLIENT_OTHERMESSAGE_H
#define GEO_NETWORK_CLIENT_OTHERMESSAGE_H

#include "../../Message.hpp"

#include "../../../../common/Types.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class OtherMessage: public Message {

protected:
    OtherMessage();

    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;

};
#endif //GEO_NETWORK_CLIENT_OTHERMESSAGE_H
