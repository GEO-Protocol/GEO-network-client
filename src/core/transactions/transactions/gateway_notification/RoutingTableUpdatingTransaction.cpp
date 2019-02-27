#include "RoutingTableUpdatingTransaction.h"

RoutingTableUpdatingTransaction::RoutingTableUpdatingTransaction(
    RoutingTableResponseMessage::Shared message,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    EquivalentsCyclesSubsystemsRouter *equivalentsCyclesSubsystemsRouter,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTableUpdatingType,
        message->transactionUUID(),
        0,
        logger),
    mMessage(message),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mEquivalentsCyclesSubsystemsRouter(equivalentsCyclesSubsystemsRouter)
{}

TransactionResult::SharedConst RoutingTableUpdatingTransaction::run()
{
    for (const auto &equivalentAndNeighbors : mMessage->neighborsByEquivalents()) {
        try {
            auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(
                equivalentAndNeighbors.first);
            auto routingTablesManager = mEquivalentsCyclesSubsystemsRouter->routingTableManager(
                equivalentAndNeighbors.first);
            if(!trustLinesManager->trustLineIsPresent(mMessage->idOnReceiverSide)){
                warning() << "Node " << mMessage->idOnReceiverSide << " is not a neighbor on equivalent "
                          << equivalentAndNeighbors.first;
                continue;
            }
            routingTablesManager->updateMapAddSeveralNeighbors(
                mMessage->idOnReceiverSide,
                equivalentAndNeighbors.second);
        } catch (NotFoundError &e) {
            warning() << "There is no subsystems for equivalent " << equivalentAndNeighbors.first;
            continue;
        }
    }
    return resultDone();
}

const string RoutingTableUpdatingTransaction::logHeader() const
{
    stringstream s;
    s << "[RoutingTableUpdatingTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}