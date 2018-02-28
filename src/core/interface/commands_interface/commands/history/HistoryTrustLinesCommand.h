#ifndef GEO_NETWORK_CLIENT_HISTORYTRUSTLINESCOMMAND_H
#define GEO_NETWORK_CLIENT_HISTORYTRUSTLINESCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"
#include "../../../../common/time/TimeUtils.h"

class HistoryTrustLinesCommand : public BaseUserCommand {

public:
    typedef shared_ptr<HistoryTrustLinesCommand> Shared;

public:
    HistoryTrustLinesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk(
        string &historyTrustLinesStr) const;

    const size_t historyFrom() const;

    const size_t historyCount() const;

    const DateTime timeFrom() const;

    const DateTime timeTo() const;

    const bool isTimeFromPresent() const;

    const bool isTimeToPresent() const;

    const SerializedEquivalent equivalent() const;

private:
    string kNullParameter = "null";

private:
    size_t mHistoryFrom;
    size_t mHistoryCount;
    DateTime mTimeFrom;
    DateTime mTimeTo;
    bool mIsTimeFromPresent;
    bool mIsTimeToPresent;
    SerializedEquivalent mEquivalent;
};


#endif //GEO_NETWORK_CLIENT_HISTORYTRUSTLINESCOMMAND_H
