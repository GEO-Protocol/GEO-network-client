#ifndef GEO_NETWORK_CLIENT_SERVICEMESSAGE_H
#define GEO_NETWORK_CLIENT_SERVICEMESSAGE_H

#include "../../common/Types.h"

#include <vector>
#include <memory>
#include <malloc.h>

using namespace std;

class ServiceMessage {

public:
    typedef shared_ptr<ServiceMessage> Shared;

public:
    enum ServiceMessageType {
        RemoveDeprecatedChannelType = 1001
    };

    ServiceMessage(ServiceMessageType messageType,
                   uint16_t value) {

        mMessageType = messageType;
        mValue = value;
    };

    ServiceMessage(byte *buffer) {

        deserialize(buffer);
    };

    const ServiceMessageType messageType() const {

        return mMessageType;
    }

    const uint16_t value() const {
        return mValue;
    };

    static const size_t kServieMessageBytesCount() {

        static size_t bytesCount = 4;
        return bytesCount;
    }

    vector<byte> serialize() {

        size_t dataSize = sizeof(uint16_t) * 2;
        byte * data = (byte *) calloc(
            dataSize,
            sizeof(byte)
        );
        memcpy(
            data,
            &mMessageType,
            sizeof(uint16_t)
        );
        memcpy(
            data + sizeof(uint16_t),
            &mValue,
            sizeof(uint16_t)
        );

        vector<byte> bytes;
        bytes.reserve(dataSize);
        for (size_t i = 0; i < dataSize; ++i) {
            byte item = data[i];
            bytes.push_back(item);
        }
        return bytes;
    };

private:

    void deserialize(
        byte *buffer) {

        uint16_t *messageType = new (buffer) uint16_t;
        mMessageType = (ServiceMessageType) *messageType;
        uint16_t *value = new (buffer + sizeof(uint16_t)) uint16_t;
        mValue = *value;
    };

private:
    ServiceMessageType mMessageType;
    uint16_t mValue;
};

#endif //GEO_NETWORK_CLIENT_SERVICEMESSAGE_H
