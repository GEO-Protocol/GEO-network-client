#ifndef GEO_NETWORK_CLIENT_SHAREKEYSCOMMAND_H
#define GEO_NETWORK_CLIENT_SHAREKEYSCOMMAND_H

#include "../BaseUserCommand.h"

class ShareKeysCommand : public BaseUserCommand {

public:
    typedef shared_ptr<ShareKeysCommand> Shared;

public:
    ShareKeysCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    const ContractorID contractorID() const;

    const SerializedEquivalent equivalent() const;

private:
    ContractorID mContractorID;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_SHAREKEYSCOMMAND_H
