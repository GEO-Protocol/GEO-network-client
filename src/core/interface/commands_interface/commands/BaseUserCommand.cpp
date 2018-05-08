#include "BaseUserCommand.h"

BaseUserCommand::BaseUserCommand(
    const string& identifier) :

    mCommandIdentifier(identifier)
{}

BaseUserCommand::BaseUserCommand(
    const CommandUUID &commandUUID,
    const string &identifier) :

    mCommandUUID(commandUUID),
    mCommandIdentifier(identifier)
{}


const CommandUUID &BaseUserCommand::UUID() const
{
    return mCommandUUID;
}

const string &BaseUserCommand::identifier() const
{
    return mCommandIdentifier;
}

/**
 * Shortcut for creating results in derived commands classes.
 *
 * @param code - result code that would be transferred out of the engine.
 */
CommandResult::SharedConst BaseUserCommand::makeResult(
    const uint16_t code) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        code);
}

CommandResult::SharedConst BaseUserCommand::responseOK() const
    noexcept
{
    return makeResult(200);
}

CommandResult::SharedConst BaseUserCommand::responseCreated() const
    noexcept
{
    return makeResult(201);
}

CommandResult::SharedConst BaseUserCommand::responsePostponedByReservations() const
    noexcept
{
    return makeResult(203);
}

CommandResult::SharedConst BaseUserCommand::responseProtocolError() const
    noexcept
{
    return makeResult(401);
}

CommandResult::SharedConst BaseUserCommand::responseTrustLineIsAbsent() const
    noexcept
{
    return makeResult(405);
}

CommandResult::SharedConst BaseUserCommand::responseCurrentIncomingDebtIsGreaterThanNewAmount() const
    noexcept
{
    return makeResult(406);
}

CommandResult::SharedConst BaseUserCommand::responseTrustLineIsAlreadyPresent() const
    noexcept
{
    return makeResult(409);
}

CommandResult::SharedConst BaseUserCommand::responseTrustLineRejected() const
{
    return makeResult(402);
}

CommandResult::SharedConst BaseUserCommand::responseInsufficientFunds() const
    noexcept
{
    return makeResult(412);
}

CommandResult::SharedConst BaseUserCommand::responseConflictWithOtherOperation() const
    noexcept
{
    return makeResult(429);
}

CommandResult::SharedConst BaseUserCommand::responseRemoteNodeIsInaccessible() const
    noexcept
{
    return makeResult(444);
}

CommandResult::SharedConst BaseUserCommand::responseNoRoutes() const
    noexcept
{
    return makeResult(462);
}

CommandResult::SharedConst BaseUserCommand::responseUnexpectedError() const
    noexcept
{
    return makeResult(501);
}

CommandResult::SharedConst BaseUserCommand::responseForbiddenRunTransaction() const
    noexcept
{
    return makeResult(603);
}

CommandResult::SharedConst BaseUserCommand::responseEquivalentIsAbsent() const
    noexcept
{
    return makeResult(604);
}

