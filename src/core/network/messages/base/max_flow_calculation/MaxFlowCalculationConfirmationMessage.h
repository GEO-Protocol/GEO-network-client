#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCONFIRMATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCONFIRMATIONMESSAGE_H

#include "../../SenderMessage.h"

class MaxFlowCalculationConfirmationMessage : public SenderMessage {

public:
    typedef shared_ptr<MaxFlowCalculationConfirmationMessage> Shared;

public:
    MaxFlowCalculationConfirmationMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> senderAddresses,
        const ConfirmationID confirmationID);

    MaxFlowCalculationConfirmationMessage(
        BytesShared buffer);

    void setConfirmationID(
        const ConfirmationID confirmationID);

    const MessageType typeID() const;

    const ConfirmationID confirmationID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    virtual const size_t kOffsetToInheritedBytes() const override;

private:
    ConfirmationID mConfirmationID;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCONFIRMATIONMESSAGE_H
