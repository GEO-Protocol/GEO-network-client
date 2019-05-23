#ifndef GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORADDRESSESCOMMAND_H
#define GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORADDRESSESCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../contractors/addresses/IPv4WithPortAddress.h"
#include "../../../../common/exceptions/ValueError.h"

class SetChannelContractorAddressesCommand : public BaseUserCommand {

public:
    typedef shared_ptr<SetChannelContractorAddressesCommand> Shared;

public:
    SetChannelContractorAddressesCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    vector<BaseAddress::Shared> contractorAddresses() const;

    const ContractorID contractorChannelID() const;

private:
    vector<BaseAddress::Shared> mContractorAddresses;
    ContractorID mContractorChannelID;
};


#endif //GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORADDRESSESCOMMAND_H
