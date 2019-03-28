#ifndef GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

class InitTrustLineCommand : public BaseUserCommand {

public:
    typedef shared_ptr<InitTrustLineCommand> Shared;

public:
    InitTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier()
    noexcept;

    const ContractorID contractorID() const
    noexcept;

    const SerializedEquivalent equivalent() const
    noexcept;

private:
    SerializedEquivalent mEquivalent;
    ContractorID mContractorID;
};

#endif //GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H
