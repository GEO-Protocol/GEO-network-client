#ifndef GEO_NETWORK_CLIENT_GETTRUSTLINEBYIDCOMMAND_H
#define GEO_NETWORK_CLIENT_GETTRUSTLINEBYIDCOMMAND_H

#include "../BaseUserCommand.h"

class GetTrustLineByIDCommand : public BaseUserCommand {

public:
    typedef shared_ptr<GetTrustLineByIDCommand> Shared;

public:
    GetTrustLineByIDCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier()
    noexcept;

    const ContractorID contractorID() const
    noexcept;

    const SerializedEquivalent equivalent() const
    noexcept;

    CommandResult::SharedConst resultOk(
        string &neighbor) const;

private:
    ContractorID mContractorID;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_GETTRUSTLINEBYIDCOMMAND_H
