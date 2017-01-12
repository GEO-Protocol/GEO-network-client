#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/Types.h"

#include "../channels/packet/PacketHeader.h"
#include "../channels/packet/Packet.h"

#include "../../common/exceptions/Exception.h"
#include "../../common/exceptions/MemoryError.h"

#include <boost/crc.hpp>

#include <stdint.h>
#include <malloc.h>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>


class SerialisationError: public Exception {
    using Exception::Exception;
};


using namespace std;

class Message {

public:
    typedef shared_ptr<Message> Shared;

public:
    enum MessageTypeID {
        ProcessingReportMessage = 1,
        OpenTrustLineMessageType = 2
    };

public:
    virtual pair<ConstBytesShared, size_t> serialize() const = 0;

    virtual const MessageTypeID typeID() const = 0;
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
