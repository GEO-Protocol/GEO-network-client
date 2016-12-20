#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H

#include "BaseUserCommand.h"
#include "../../../common/exceptions/ValueError.h"


class CloseTrustLineCommand:
    public BaseUserCommand {

public:
    static const string &identifier();

public:
    CloseTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    const NodeUUID &contractorUUID() const;

protected:
    NodeUUID mContractorUUID;

private:
    void deserialize(const string &command);
};

#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
