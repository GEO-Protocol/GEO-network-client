#include "TotalBalanceResult.h"

TotalBalanceResult::TotalBalanceResult(Command *command,
                                       const uint16_t &resultCode,
                                       const string &timestampCompleted,
                                       const trust_amount &totalIncomingTrust,
                                       const trust_amount &totalIncomingTrustUsed,
                                       const trust_amount &totalOutgoingTrust,
                                       const trust_amount &totalOutgoingTrustUsed) :
        Result(command, resultCode, timestampCompleted), mTotalIncomingTrust(totalIncomingTrust),
        mTotalIncomingTrustUsed(mTotalIncomingTrustUsed),
        mTotalOutgoingTrust(totalOutgoingTrust), mTotalOutgoingTrustUsed(totalOutgoingTrustUsed) {}

const uuids::uuid &TotalBalanceResult::commandUUID() const {
    return commandsUUID();
}

const string &TotalBalanceResult::id() const {
    return identifier();
}

const uint16_t &TotalBalanceResult::resultCode() const {
    return resCode();
}

const string &TotalBalanceResult::timestampExcepted() const {
    return exceptedTimestamp();
}

const string &TotalBalanceResult::timestampCompleted() const {
    return completedTimestamp();
}

const trust_amount &TotalBalanceResult::totalIncomingTrust() const {
    return mTotalIncomingTrust;
}

const trust_amount &TotalBalanceResult::totalIncomingTrustUsed() const {
    return mTotalIncomingTrustUsed;
}

const trust_amount &TotalBalanceResult::totalOutgoingTrust() const {
    return mTotalOutgoingTrust;
}

const trust_amount &TotalBalanceResult::totalOutgoingTrustUsed() const {
    return mTotalOutgoingTrustUsed;
}

string TotalBalanceResult::serialize() {
    if (resultCode() == 200) {
        return boost::lexical_cast<string>(commandUUID()) + " " +
               boost::lexical_cast<string>(resultCode()) + " " +
               boost::lexical_cast<string>(mTotalIncomingTrust) + " " +
               boost::lexical_cast<string>(mTotalIncomingTrustUsed) + " " +
               boost::lexical_cast<string>(mTotalOutgoingTrust) + " " +
               boost::lexical_cast<string>(mTotalOutgoingTrustUsed) +
               "\n";
    }
    return boost::lexical_cast<string>(commandUUID()) + " " +
           boost::lexical_cast<string>(resultCode()) +
           "\n";
}

