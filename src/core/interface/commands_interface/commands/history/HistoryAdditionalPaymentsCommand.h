#ifndef GEO_NETWORK_CLIENT_HISTORYADDTIONALPAYMENTSCOMMAND_H
#define GEO_NETWORK_CLIENT_HISTORYADDTIONALPAYMENTSCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../../common/exceptions/ValueError.h"

class HistoryAdditionalPaymentsCommand : public BaseUserCommand {

public:
    typedef shared_ptr<HistoryAdditionalPaymentsCommand> Shared;

public:
    HistoryAdditionalPaymentsCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk(
        string &historyPaymentsStr) const;

    const size_t historyFrom() const;

    const size_t historyCount() const;

    const DateTime timeFrom() const;

    const DateTime timeTo() const;

    const bool isTimeFromPresent() const;

    const bool isTimeToPresent() const;

    const TrustLineAmount& lowBoundaryAmount() const;

    const TrustLineAmount& highBoundaryAmount() const;

    const bool isLowBoundaryAmountPresent() const;

    const bool isHighBoundaryAmountPresent() const;

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
    TrustLineAmount mLowBoundaryAmount;
    TrustLineAmount mHighBoundaryAmount;
    bool mIsLowBoundaryAmountPresent;
    bool mIsHighBoundaryAmountPresent;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_HISTORYADDTIONALPAYMENTSCOMMAND_H
