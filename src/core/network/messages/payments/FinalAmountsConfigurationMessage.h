#ifndef GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>

class FinalAmountsConfigurationMessage : public TransactionMessage {
public:
    typedef shared_ptr<FinalAmountsConfigurationMessage> Shared;

public:
    FinalAmountsConfigurationMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathUUID, ConstSharedTrustLineAmount>> &finalAmountsConfig);

    FinalAmountsConfigurationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    vector<pair<PathUUID, ConstSharedTrustLineAmount>> finalAmountsConfiguration() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

    const size_t kOffsetToInheritedBytes() const
        noexcept;

private:
    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

private:
    vector<pair<PathUUID, ConstSharedTrustLineAmount>> mFinalAmountsConfiguration;
};


#endif //GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
