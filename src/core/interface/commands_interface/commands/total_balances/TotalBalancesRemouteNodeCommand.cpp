#include "TotalBalancesRemouteNodeCommand.h"

TotalBalancesRemouteNodeCommand::TotalBalancesRemouteNodeCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = NodeUUID::kHexSize + 2;
    if (commandBuffer.size() < minCommandLength) {
        throw ValueError("TotalBalancesRemoteNodeCommand: "
                             "Can't parse command. Received command is to short.");
    }
    try {
        string hexUUID = commandBuffer.substr(
            0,
            NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        throw ValueError("TotalBalancesRemoteNodeCommand: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }


    size_t tokenSeparatorPos = commandBuffer.find(kTokensSeparator, NodeUUID::kHexSize + 1);
    string gatewaysCountStr = commandBuffer.substr(
        NodeUUID::kHexSize + 1,
        tokenSeparatorPos);
    if (gatewaysCountStr.at(0) == '-') {
        throw ValueError("TotalBalancesRemoteNodeCommand: "
                             "Can't parse command. 'count gateways' token can't be negative.");
    }
    try {
        mGatewaysCount = std::stoul(gatewaysCountStr);
    } catch (...) {
        throw ValueError("TotalBalancesRemoteNodeCommand: "
                             "Can't parse command. Error occurred while parsing  'count gateways' token.");
    }
    if (mGatewaysCount == 0) {
        return;
    }
    mGateways.reserve(mGatewaysCount);
    size_t gatewayStartPoint = tokenSeparatorPos + 1;
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
            throw ValueError("TotalBalancesRemoteNodeCommand: "
                                 "Can't parse command. Error occurred while parsing 'Gateway UUID' token.");
        }
    }
    if (gatewayStartPoint + 1 < commandBuffer.length()) {
        throw ValueError("TotalBalancesRemoteNodeCommand: "
                             "Can't parse command. Disparity between command count gateways and real count gateways.");
    }
}

const string &TotalBalancesRemouteNodeCommand::identifier()
{
    static const string identifier = "GET:nodes/stats/balances/total";
    return identifier;
}

const NodeUUID &TotalBalancesRemouteNodeCommand::contractorUUID() const
{
    return mContractorUUID;
}

CommandResult::SharedConst TotalBalancesRemouteNodeCommand::responseOk(
    string &totalBalancesStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            totalBalancesStr));
}

