#include "CycleCloserCommand.h"

CycleCloserCommand::CycleCloserCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()) {

    parse(commandBuffer);
}

const string &CycleCloserCommand::identifier() {

    static const string identifier = "GET:/cycle/close";
    return identifier;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void CycleCloserCommand::parse(
    const string &command) {

}

Path::ConstShared CycleCloserCommand::path() {

    NodeUUID *nodeUUID51Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff51");
    NodeUUID *nodeUUID52Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff52");
    NodeUUID *nodeUUID53Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff53");
    NodeUUID *nodeUUID54Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff54");
    NodeUUID *nodeUUID55Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff55");

    vector<NodeUUID> intermediateNodes;
    intermediateNodes.push_back(*nodeUUID52Ptr);
    intermediateNodes.push_back(*nodeUUID53Ptr);
    intermediateNodes.push_back(*nodeUUID54Ptr);
    intermediateNodes.push_back(*nodeUUID55Ptr);
    auto result = make_shared<const Path>(
        *nodeUUID51Ptr,
        *nodeUUID51Ptr,
        intermediateNodes);

    delete nodeUUID51Ptr;
    delete nodeUUID52Ptr;
    delete nodeUUID53Ptr;
    delete nodeUUID54Ptr;
    delete nodeUUID55Ptr;

    return result;
}

CommandResult::SharedConst CycleCloserCommand::resultOk() const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200));
}