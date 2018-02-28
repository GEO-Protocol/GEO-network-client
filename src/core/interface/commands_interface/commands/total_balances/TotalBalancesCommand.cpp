#include "TotalBalancesCommand.h"

TotalBalancesCommand::TotalBalancesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = 3;
    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "TotalBalancesCommand: can't parse command. "
                    "Received command is to short.");
    }
    size_t tokenSeparatorPos = commandBuffer.find(kTokensSeparator);
    string gatewaysCountStr = commandBuffer.substr(
        0,
        tokenSeparatorPos);
    try {
        mGatewaysCount = std::stoul(gatewaysCountStr);
    } catch (...) {
        throw ValueError(
                "TotalBalancesCommand: can't parse command. "
                    "Error occurred while parsing  'count gateways' token.");
    }

    size_t gatewayStartPoint = tokenSeparatorPos + 1;
    if (mGatewaysCount != 0) {
        mGateways.reserve(mGatewaysCount);
        for (size_t idx = 0; idx < mGatewaysCount; idx++) {
            try {
                string hexUUID = commandBuffer.substr(
                    gatewayStartPoint,
                    NodeUUID::kHexSize);
                mGateways.push_back(
                    boost::lexical_cast<uuids::uuid>(
                        hexUUID));
                gatewayStartPoint += NodeUUID::kHexSize + 1;
            } catch (...) {
                throw ValueError(
                        "TotalBalancesCommand: can't parse command. "
                            "Error occurred while parsing 'Gateway UUID' token.");
            }
        }
    }

    size_t equivalentStartPoint = gatewayStartPoint;
    string equivalentStr = commandBuffer.substr(
        equivalentStartPoint,
        commandBuffer.size() - equivalentStartPoint - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "TotalBalancesCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
    }
}

const string &TotalBalancesCommand::identifier()
{
    static const string identifier = "GET:stats/balance/total";
    return identifier;
}

const vector<NodeUUID>& TotalBalancesCommand::gateways() const
{
    return mGateways;
}

const SerializedEquivalent TotalBalancesCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst TotalBalancesCommand::resultOk(
    string &totalBalancesStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            totalBalancesStr));
}
