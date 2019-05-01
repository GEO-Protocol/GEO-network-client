#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"

/**
 * This command is used to open/close/update trust line to the remote contractor node.
 *
 * To create trust line — this command must be launched against new contractor.
 * To update trust line — this command must be issued against already present contractor.
 * To remove trust line — this command must be issued with 0 amount.
 */
class SetOutgoingTrustLineCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<SetOutgoingTrustLineCommand> Shared;

public:
    SetOutgoingTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    const ContractorID contractorID() const;

    const TrustLineAmount &amount() const;

    const SerializedEquivalent equivalent() const;

private:
    ContractorID mContractorID;
    TrustLineAmount mAmount;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H
