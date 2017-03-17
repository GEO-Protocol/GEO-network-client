#ifndef GEO_NETWORK_CLIENT_HISTORYPAYMENTSCOMMAND_H
#define GEO_NETWORK_CLIENT_HISTORYPAYMENTSCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class HistoryPaymentsCommand : public BaseUserCommand {

public:
    typedef shared_ptr<HistoryPaymentsCommand> Shared;

public:
    HistoryPaymentsCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk(string &historyPaymentsStr) const;

    const size_t historyFrom() const;

    const size_t historyCount() const;

protected:

    void parse(
        const string &command);

private:
    size_t mHistoryFrom;
    size_t mHistoryCount;

};


#endif //GEO_NETWORK_CLIENT_HISTORYPAYMENTSCOMMAND_H
