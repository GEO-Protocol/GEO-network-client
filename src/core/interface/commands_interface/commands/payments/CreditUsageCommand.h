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
    CreditUsageCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const TrustLineAmount& amount() const;

    const NodeUUID& contractorUUID() const;

    const SerializedEquivalent equivalent() const;

public:
    // Results handlers
    CommandResult::SharedConst responseNoConsensus() const;
    CommandResult::SharedConst responseOK(
        string &transactionUUID) const;

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_CREDITUSAGECOMMAND_H
