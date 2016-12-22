#ifndef GEO_NETWORK_CLIENT_TESTMESSAGE_H
#define GEO_NETWORK_CLIENT_TESTMESSAGE_H

#include "Message.h"

class TestMessage: public Message {
public:
    std::shared_ptr<SerialisedMessage> serialize() const {
        const char *bytes = "abcdabcd"; // note: "SerialisedMessage" object will own it.
        const uint64_t bytesLen = 8;

        return std::shared_ptr<SerialisedMessage>(new SerialisedMessage((uint8_t*)(bytes), bytesLen));
    }

    const MessageTypeID typeID() const {
        return Message::MessageTypeID ::ProcessingReportMessage;
    }
};

#endif //GEO_NETWORK_CLIENT_TESTMESSAGE_H
