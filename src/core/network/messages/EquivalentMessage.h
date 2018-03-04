#ifndef GEO_NETWORK_CLIENT_EQUIVALENTMESSAGE_H
#define GEO_NETWORK_CLIENT_EQUIVALENTMESSAGE_H

#include "Message.hpp"

/*
 * Abstract base class for messages that must contain equivalent.
 */
class EquivalentMessage : public Message {

public:
    EquivalentMessage(
        const SerializedEquivalent equivalent)
        noexcept;

    EquivalentMessage(
        BytesShared buffer)
        noexcept;

    const SerializedEquivalent equivalent() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    virtual const size_t kOffsetToInheritedBytes() const
        noexcept;

private:
    SerializedEquivalent mEquivalent;
};


#endif //GEO_NETWORK_CLIENT_EQUIVALENTMESSAGE_H
