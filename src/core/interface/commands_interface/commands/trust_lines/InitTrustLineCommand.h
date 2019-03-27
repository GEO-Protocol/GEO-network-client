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

    const SerializedEquivalent equivalent() const
    noexcept;

    vector<BaseAddress::Shared> contractorAddresses() const
    noexcept;

private:
    SerializedEquivalent mEquivalent;
    size_t mContractorAddressesCount;
    vector<BaseAddress::Shared> mContractorAddresses;
};

#endif //GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H
