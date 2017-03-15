#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H

#include "../BaseUserCommand.h"

class TotalBalancesCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TotalBalancesCommand> Shared;

public:
    TotalBalancesCommand(
            const CommandUUID &uuid,
            const string &commandBuffer);

    TotalBalancesCommand(
            BytesShared buffer);

    static const string &identifier();

    pair<BytesShared, size_t> serializeToBytes();

    CommandResult::SharedConst resultOk(string &totalBalancesStr) const;

protected:
    void deserializeFromBytes(
            BytesShared buffer);

    void parse(
            const string &command);

};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H
