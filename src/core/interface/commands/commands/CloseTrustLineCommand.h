#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H

#include "BaseUserCommand.h"
#include "../../../common/exceptions/ValueError.h"


class CloseTrustLineCommand:
    public BaseUserCommand {

public:
    typedef shared_ptr<CloseTrustLineCommand> Shared;

protected:
    NodeUUID mContractorUUID;

public:
    CloseTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    const CommandResult *resultOk() const;

    const CommandResult *trustLineIsAbsentResult() const;

    const CommandResult *trustLineIsAbsentOrInvalidResult() const;

private:
    void deserialize(
            const string &command);
};

#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINECOMMAND_H
