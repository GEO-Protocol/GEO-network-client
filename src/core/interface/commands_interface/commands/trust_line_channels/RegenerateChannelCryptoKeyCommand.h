#ifndef GEO_NETWORK_CLIENT_REGENERATECHANNELCRYPTOKEYCOMMAND_H
#define GEO_NETWORK_CLIENT_REGENERATECHANNELCRYPTOKEYCOMMAND_H

#include "../BaseUserCommand.h"

class RegenerateChannelCryptoKeyCommand : public BaseUserCommand {

public:
    typedef shared_ptr<RegenerateChannelCryptoKeyCommand> Shared;

public:
    RegenerateChannelCryptoKeyCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    const ContractorID contractorChannelID() const;

    CommandResult::SharedConst responseOk(
        string &channelInfo) const;

private:
    ContractorID mContractorChannelID;
};


#endif //GEO_NETWORK_CLIENT_REGENERATECHANNELCRYPTOKEYCOMMAND_H
