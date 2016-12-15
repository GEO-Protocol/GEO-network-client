#include "ContractorsListCommand.h"

ContractorsListCommand::ContractorsListCommand(const uuids::uuid &commandUUID, const string &identifier,
                                         const string &timestampExcepted) :
        Command(commandUUID, identifier, timestampExcepted) {
}

const uuids::uuid &ContractorsListCommand::commandUUID() const {
    return commandsUUID();
}

const string &ContractorsListCommand::id() const {
    return identifier();
}

const string &ContractorsListCommand::exceptedTimestamp() const {
    return timeStampExcepted();
}

void ContractorsListCommand::deserialize() {}

