#ifndef GEO_NETWORK_CLIENT_UPDATECHANNELADDRESSESMESSAGE_H
#define GEO_NETWORK_CLIENT_UPDATECHANNELADDRESSESMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class UpdateChannelAddressesMessage : public TransactionMessage {

public:
    typedef shared_ptr<UpdateChannelAddressesMessage> Shared;

public:
    UpdateChannelAddressesMessage(
        ContractorID idOnSenderSide,
        const TransactionUUID &transactionUUID,
        vector<BaseAddress::Shared> newSenderAddresses,
        Contractor::Shared contractor);

    UpdateChannelAddressesMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    vector<BaseAddress::Shared> newSenderAddresses() const;

    const bool isAddToConfirmationRequiredMessagesHandler() const override;

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    vector<BaseAddress::Shared> mNewSenderAddresses;
};


#endif //GEO_NETWORK_CLIENT_UPDATECHANNELADDRESSESMESSAGE_H
