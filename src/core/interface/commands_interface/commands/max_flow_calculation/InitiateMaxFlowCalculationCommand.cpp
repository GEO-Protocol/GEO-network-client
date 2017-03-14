//
// Created by mc on 14.02.17.
//

#include "InitiateMaxFlowCalculationCommand.h"

InitiateMaxFlowCalculationCommand::InitiateMaxFlowCalculationCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()) {

    parse(commandBuffer);
}

InitiateMaxFlowCalculationCommand::InitiateMaxFlowCalculationCommand(
    BytesShared buffer) :

    BaseUserCommand(identifier()) {

    deserializeFromBytes(buffer);
}

const string &InitiateMaxFlowCalculationCommand::identifier() {

    static const string identifier = "GET:contractors/transactions/max";
    return identifier;
}

const NodeUUID &InitiateMaxFlowCalculationCommand::contractorUUID() const {

    return mContractorUUID;
}

pair<BytesShared, size_t> InitiateMaxFlowCalculationCommand::serializeToBytes(){

    auto parentBytesAndCount = BaseUserCommand::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +
                        NodeUUID::kBytesSize;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mContractorUUID.data,
            NodeUUID::kBytesSize);
    //----------------------------------------------------
    return make_pair(
            dataBytesShared,
            bytesCount);
}

void InitiateMaxFlowCalculationCommand::deserializeFromBytes(
        BytesShared buffer) {

    BaseUserCommand::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = BaseUserCommand::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
            mContractorUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void InitiateMaxFlowCalculationCommand::parse(
        const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset;

    if (command.size() < minCommandLength) {
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
                0,
                NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

CommandResult::SharedConst InitiateMaxFlowCalculationCommand::resultOk(string &maxFlowAmount) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            maxFlowAmount));
}


