#ifndef GEO_NETWORK_CLIENT_RESETTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_RESETTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class ResetTrustLineCommand : public BaseUserCommand {

public:
    typedef shared_ptr<ResetTrustLineCommand> Shared;

public:
    ResetTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    const ContractorID contractorID() const;

    const AuditNumber auditNumber() const;

    const TrustLineAmount &incomingTrustAmount() const;

    const TrustLineAmount &outgoingTrustAmount() const;

    const TrustLineBalance &balance() const;

    const SerializedEquivalent equivalent() const;

private:
    ContractorID mContractorID;
    AuditNumber mAuditNumber;
    TrustLineAmount mIncomingTrustAmount;
    TrustLineAmount mOutgoingTrustAmount;
    TrustLineBalance mBalance;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_RESETTRUSTLINECOMMAND_H
