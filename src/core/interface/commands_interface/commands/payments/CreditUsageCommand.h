#ifndef GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
#define GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../common/exceptions/ValueError.h"
#include "../../../../common/exceptions/MemoryError.h"


class CreditUsageCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<CreditUsageCommand> Shared;

public:
    static const string &identifier();

// TODO: deprecated
//    static const size_t kMinRequestedBufferSize();

public:
    CreditUsageCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    CreditUsageCommand(
        BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes();
    const TrustLineAmount& amount() const;
    const NodeUUID& contractorUUID() const;

public:
    // Results handlers
    CommandResult::SharedConst responseNoConsensus() const;
    CommandResult::SharedConst responseOK(string &transactionUUID) const;

protected:
    void deserializeFromBytes(
        BytesShared buffer);

    void parse(
        const string &command);

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
