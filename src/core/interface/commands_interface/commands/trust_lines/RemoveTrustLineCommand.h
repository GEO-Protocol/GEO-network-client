#ifndef GEO_NETWORK_CLIENT_REMOVETRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_REMOVETRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

class RemoveTrustLineCommand : public BaseUserCommand {

public:
    typedef shared_ptr<RemoveTrustLineCommand> Shared;

public:
    RemoveTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    const ContractorID contractorID() const;

    const SerializedEquivalent equivalent() const;

private:
    ContractorID mContractorID;
    SerializedEquivalent mEquivalent;
};


#endif //GEO_NETWORK_CLIENT_REMOVETRUSTLINECOMMAND_H
