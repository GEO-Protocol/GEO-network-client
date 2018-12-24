#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONMESSAGE_H

#include "../SenderMessage.h"


class InitiateMaxFlowCalculationMessage :
    public SenderMessage {

public:
    typedef shared_ptr<InitiateMaxFlowCalculationMessage> Shared;

public:
    InitiateMaxFlowCalculationMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> &senderAddresses,
        bool isSenderGateway)
        noexcept;

    InitiateMaxFlowCalculationMessage(
        BytesShared buffer)
        noexcept;

    bool isSenderGateway() const;

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

private:
    bool mIsSenderGateway;
};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONMESSAGE_H
