#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class TotalBalancesRemouteNodeCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TotalBalancesRemouteNodeCommand> Shared;

public:
    TotalBalancesRemouteNodeCommand(
            const CommandUUID &uuid,
            const string &commandBuffer);

    TotalBalancesRemouteNodeCommand(
            BytesShared buffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    pair<BytesShared, size_t> serializeToBytes();

    CommandResult::SharedConst resultOk(string &totalBalancesStr) const;

    CommandResult::SharedConst resultNoResponse() const;

protected:
    void deserializeFromBytes(
            BytesShared buffer);

    void parse(
            const string &command);

private:
    NodeUUID mContractorUUID;

};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H
