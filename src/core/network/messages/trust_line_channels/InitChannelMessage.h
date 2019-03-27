#ifndef GEO_NETWORK_CLIENT_INITCHANNELMESSAGE_H
#define GEO_NETWORK_CLIENT_INITCHANNELMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class InitChannelMessage : public TransactionMessage {

public:
    typedef shared_ptr<InitChannelMessage> Shared;

public:
    InitChannelMessage(
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        ContractorID contractorID)
        noexcept;

    InitChannelMessage(
        BytesShared buffer)
        noexcept;

    const MessageType typeID() const
    noexcept;

    const ContractorID contractorID() const
    noexcept;

    const bool isAddToConfirmationRequiredMessagesHandler() const override;

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    ContractorID mContractorID;
};


#endif //GEO_NETWORK_CLIENT_INITCHANNELMESSAGE_H
