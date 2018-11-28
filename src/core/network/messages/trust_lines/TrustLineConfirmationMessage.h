#ifndef GEO_NETWORK_CLIENT_TRUSTLINECONFIRMATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINECONFIRMATIONMESSAGE_H

#include "../base/transaction/ConfirmationMessage.h"

class TrustLineConfirmationMessage : public ConfirmationMessage {

public:
    typedef shared_ptr<TrustLineConfirmationMessage> Shared;

public:
    TrustLineConfirmationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        ContractorID idOnSenderSide,
        const TransactionUUID &transactionUUID,
        ContractorID contractorID,
        bool isContractorGateway,
        const OperationState state);

    TrustLineConfirmationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const ContractorID contractorID() const;

    const bool isContractorGateway() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    ContractorID mContractorID;
    bool mIsContractorGateway;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINECONFIRMATIONMESSAGE_H
