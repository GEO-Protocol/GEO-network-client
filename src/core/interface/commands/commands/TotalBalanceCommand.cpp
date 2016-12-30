#include "TotalBalanceCommand.h"

TotalBalanceCommand::TotalBalanceCommand(
        const CommandUUID &uuid) :

        BaseUserCommand(uuid, identifier()) {}

const string &TotalBalanceCommand::identifier() {
    static const string identifier = "GET:stats/balances/total";
    return identifier;
}

const CommandResult *TotalBalanceCommand::resultOk(
        TrustLineAmount &totalIncomingTrust,
        TrustLineAmount &totalIncomingTrustUsed,
        TrustLineAmount &totalOutgoingTrust,
        TrustLineAmount &totalOutgoingTrustUsed) const {
    string additionalInformation = boost::lexical_cast<string>(totalIncomingTrust) + "\t" +
                                   boost::lexical_cast<string>(totalIncomingTrustUsed) + "\t" +
                                   boost::lexical_cast<string>(totalOutgoingTrust) + "\t" +
                                   boost::lexical_cast<string>(totalOutgoingTrustUsed);
    return new CommandResult(uuid(), 200, additionalInformation);
}



