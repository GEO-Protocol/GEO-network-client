#include "RoutingTableUpdatingTransaction.h"

RoutingTableUpdatingTransaction::RoutingTableUpdatingTransaction(
    const NodeUUID &nodeUUID,
    RoutingTableResponseMessage::Shared message,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    EquivalentsCyclesSubsystemsRouter *equivalentsCyclesSubsystemsRouter,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTableUpdatingType,
        message->transactionUUID(),
        nodeUUID,
        0,
        logger),
    mMessage(message),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mEquivalentsCyclesSubsystemsRouter(equivalentsCyclesSubsystemsRouter)
{}

TransactionResult::SharedConst RoutingTableUpdatingTransaction::run()
{
    const auto kMessage = popNextMessage<RoutingTableResponseMessage>();
    for (const auto &equivalentAndNeighbors : kMessage->neighborsByEquivalents()) {
        try {
            auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(
                    equivalentAndNeighbors.first);
            auto routingTablesManager = mEquivalentsCyclesSubsystemsRouter->routingTableManager(
                    equivalentAndNeighbors.first);
            if(!trustLinesManager->isNeighbor(kMessage->senderUUID)){
                warning() << "Node " << kMessage->senderUUID << " is not a neighbor on equivalent "
                          << equivalentAndNeighbors.first;
                continue;
            }
            routingTablesManager->updateMapAddSeveralNeighbors(
                kMessage->senderUUID,
                equivalentAndNeighbors.second);
        } catch (NotFoundError &e) {
            warning() << "There is no subsystems for equivalent " << equivalentAndNeighbors.first;
            continue;
        }
    }
}

const string RoutingTableUpdatingTransaction::logHeader() const
{
    stringstream s;
    s << "[RoutingTableUpdatingTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}