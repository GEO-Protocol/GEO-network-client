#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"

#include "../../../../common/exceptions/ValueError.h"

using namespace std;

class SetTrustLineCommand: public BaseUserCommand {
public:
    typedef shared_ptr<SetTrustLineCommand> Shared;

public:
    SetTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    const TrustLineAmount &newAmount() const;

    const CommandResult *resultOk() const;

    const CommandResult *trustLineAbsentResult() const;

    const CommandResult *resultConflict() const;

    const CommandResult *resultNoResponse() const;

    const CommandResult *resultTransactionConflict() const;

protected:
    void deserialize(
        const string &command);

private:
    NodeUUID mContractorUUID;
    TrustLineAmount mNewAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINECOMMAND_H
