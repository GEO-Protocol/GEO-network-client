#ifndef GEO_NETWORK_CLIENT_TOTALBALANCEREMOUTENODECOMMAND_H
#define GEO_NETWORK_CLIENT_TOTALBALANCEREMOUTENODECOMMAND_H

#include "../BaseUserCommand.h"

class TotalBalanceRemouteNodeCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TotalBalanceRemouteNodeCommand> Shared;

public:
    TotalBalanceRemouteNodeCommand(
            const CommandUUID &uuid,
            const string &commandBuffer);

    TotalBalanceRemouteNodeCommand(
            BytesShared buffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    pair<BytesShared, size_t> serializeToBytes();

    CommandResult::SharedConst resultOk(string &totalBalancesStr) const;

protected:
    void deserializeFromBytes(
            BytesShared buffer);

    void parse(
            const string &command);

private:
    NodeUUID mContractorUUID;

};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCEREMOUTENODECOMMAND_H
