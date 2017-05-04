#ifndef GEO_NETWORK_CLIENT_HISTORYTRUSTLINESCOMMAND_H
#define GEO_NETWORK_CLIENT_HISTORYTRUSTLINESCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class HistoryTrustLinesCommand : public BaseUserCommand {

public:
    typedef shared_ptr<HistoryTrustLinesCommand> Shared;

public:
    HistoryTrustLinesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk(string &historyTrustLinesStr) const;

    const size_t historyFrom() const;

    const size_t historyCount() const;

protected:
    void parse(
            const string &command);

private:
    size_t mHistoryFrom;
    size_t mHistoryCount;
};


#endif //GEO_NETWORK_CLIENT_HISTORYTRUSTLINESCOMMAND_H
