#ifndef GEO_NETWORK_CLIENT_GETTRUSTLINEBYADDRESSCOMMAND_H
#define GEO_NETWORK_CLIENT_GETTRUSTLINEBYADDRESSCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../contractors/addresses/IPv4WithPortAddress.h"
#include "../../../../common/exceptions/ValueError.h"

class GetTrustLineByAddressCommand : public BaseUserCommand {

public:
    typedef shared_ptr<GetTrustLineByAddressCommand> Shared;

public:
    GetTrustLineByAddressCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    vector<BaseAddress::Shared> contractorAddresses() const;

    const SerializedEquivalent equivalent() const;

    CommandResult::SharedConst resultOk(
        string &neighbor) const;

protected:
    vector<BaseAddress::Shared> mContractorAddresses;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_GETTRUSTLINEBYADDRESSCOMMAND_H
