#ifndef GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYCOMMAND_H
#define GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYCOMMAND_H

#include "../BaseUserCommand.h"

class SetChannelContractorCryptoKeyCommand : public BaseUserCommand {

public:
    typedef shared_ptr<SetChannelContractorCryptoKeyCommand> Shared;

public:
    SetChannelContractorCryptoKeyCommand(
        const CommandUUID &commandUUID,
        const string &command);

    static const string &identifier();

    const string &cryptoKey() const;

    const ContractorID contractorChannelID() const;

    const ContractorID channelIDOnContractorSide() const;

private:
    string mCryptoKey;
    ContractorID mContractorChannelID;
    ContractorID mChannelIDOnContractorSide;
};


#endif //GEO_NETWORK_CLIENT_SETCHANNELCONTRACTORCRYPTOKEYCOMMAND_H
