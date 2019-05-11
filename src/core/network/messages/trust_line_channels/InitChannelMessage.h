#ifndef GEO_NETWORK_CLIENT_INITCHANNELMESSAGE_H
#define GEO_NETWORK_CLIENT_INITCHANNELMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../contractors/Contractor.h"

class InitChannelMessage : public TransactionMessage {

public:
    typedef shared_ptr<InitChannelMessage> Shared;

public:
    InitChannelMessage(
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        Contractor::Shared contractor);

    InitChannelMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    const ContractorID contractorID() const;

    const MsgEncryptor::PublicKey::Shared publicKey() const;

    const bool isAddToConfirmationRequiredMessagesHandler() const override;

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    ContractorID mContractorID;
    MsgEncryptor::PublicKey::Shared mPublicKey;
};


#endif //GEO_NETWORK_CLIENT_INITCHANNELMESSAGE_H
