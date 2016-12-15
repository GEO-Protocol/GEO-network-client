#include "TotalBalanceCommand.h"

TotalBalanceCommand::TotalBalanceCommand(const uuids::uuid &commandUUID, const string &identifier,
                                         const string &timestampExcepted) :
        Command(commandUUID, identifier, timestampExcepted) {
}

const uuids::uuid &TotalBalanceCommand::commandUUID() const {
    return commandsUUID();
}

const string &TotalBalanceCommand::id() const {
    return identifier();
}

const string &TotalBalanceCommand::exceptedTimestamp() const {
    return timeStampExcepted();
}

void TotalBalanceCommand::deserialize() {}

