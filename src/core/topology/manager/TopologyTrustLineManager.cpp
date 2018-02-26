#include "TopologyTrustLineManager.h"

TopologyTrustLineManager::TopologyTrustLineManager(
    bool iAmGateway,
    NodeUUID &nodeUUID,
    Logger &logger):
    mLog(logger),
    mPreventDeleting(false)
{
    if (iAmGateway) {
        mGateways.insert(nodeUUID);
    }
}

void TopologyTrustLineManager::addTrustLine(
    TopologyTrustLine::Shared trustLine)
{
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(trustLine->sourceUUID());
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        if (*(trustLine->amount()) == TrustLine::kZeroAmount()) {
            return;
        }
        auto newHashSet = new unordered_set<TopologyTrustLineWithPtr*>();
        auto newTrustLineWithPtr = new TopologyTrustLineWithPtr(
            trustLine,
            newHashSet);
        newHashSet->insert(
            newTrustLineWithPtr);

        msTrustLines.insert(
            make_pair(
                trustLine->sourceUUID(),
                newHashSet));
        mtTrustLines.insert(
            make_pair(
                utc_now(),
                newTrustLineWithPtr));
    } else {
        auto hashSet = nodeUUIDAndSetFlows->second;
        auto trLineWithPtr = hashSet->begin();
        while (trLineWithPtr != hashSet->end()) {
            if ((*trLineWithPtr)->topologyTrustLine()->targetUUID() == trustLine->targetUUID()) {
                (*trLineWithPtr)->topologyTrustLine()->setAmount(trustLine->amount());

                // update time creation of trustline
                auto dateTimeAndTrustLine = mtTrustLines.begin();
                while (dateTimeAndTrustLine != mtTrustLines.end()) {
                    if (dateTimeAndTrustLine->second == *trLineWithPtr) {
                        if (*(*trLineWithPtr)->topologyTrustLine()->amount() != TrustLine::kZeroAmount()) {
                            mtTrustLines.erase(
                                dateTimeAndTrustLine);
                            mtTrustLines.insert(
                                make_pair(
                                    utc_now(),
                                    *trLineWithPtr));
                        } else {
                            auto hashSetPtr = (*trLineWithPtr)->hashSetPtr();
                            hashSetPtr->erase(*trLineWithPtr);
                            if (hashSetPtr->empty()) {
                                NodeUUID keyUUID = (*trLineWithPtr)->topologyTrustLine()->sourceUUID();
                                msTrustLines.erase(keyUUID);
                                delete hashSetPtr;
                            }
                            delete *trLineWithPtr;
                            mtTrustLines.erase(dateTimeAndTrustLine);
                        }
                        break;
                    }
                    dateTimeAndTrustLine++;
                }

                break;
            }
            trLineWithPtr++;
        }
        if (trLineWithPtr == hashSet->end()) {
            if (*trustLine->amount() == TrustLine::kZeroAmount()) {
                return;
            }
            auto newTrustLineWithPtr = new TopologyTrustLineWithPtr(
                trustLine,
                hashSet);
            hashSet->insert(
                newTrustLineWithPtr);
            mtTrustLines.insert
                (make_pair(
                    utc_now(),
                    newTrustLineWithPtr));
        }
    }
}

unordered_set<TopologyTrustLineWithPtr*> TopologyTrustLineManager::trustLinePtrsSet(
    const NodeUUID &nodeUUID)
{
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(nodeUUID);
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        TrustLineWithPtrHashSet result;
        return result;
    }
    return *nodeUUIDAndSetFlows->second;
}

void TopologyTrustLineManager::resetAllUsedAmounts()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "resetAllUsedAmounts";
#endif
    for (auto &nodeUUIDAndTrustLine : msTrustLines) {
        for (auto &trustLine : *nodeUUIDAndTrustLine.second) {
            trustLine->topologyTrustLine()->setUsedAmount(0);
        }
    }
}

void TopologyTrustLineManager::addUsedAmount(
    const NodeUUID &sourceUUID,
    const NodeUUID &targetUUID,
    const TrustLineAmount &amount)
{
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(sourceUUID);
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        return;
    }
    for (auto &trustLinePtr : *nodeUUIDAndSetFlows->second) {
        if (trustLinePtr->topologyTrustLine()->targetUUID() == targetUUID) {
            trustLinePtr->topologyTrustLine()->addUsedAmount(amount);
            return;
        }
    }
}

void TopologyTrustLineManager::makeFullyUsed(
    const NodeUUID &sourceUUID,
    const NodeUUID &targetUUID)
{
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(sourceUUID);
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        return;
    }
    for (auto &trustLinePtr : *nodeUUIDAndSetFlows->second) {
        if (trustLinePtr->topologyTrustLine()->targetUUID() == targetUUID) {
            trustLinePtr->topologyTrustLine()->setUsedAmount(
                *trustLinePtr->topologyTrustLine()->amount().get());
            return;
        }
    }
}

bool TopologyTrustLineManager::deleteLegacyTrustLines()
{
    bool isTrustLineWasDeleted = false;
    for (auto &timeAndTrustLineWithPtr : mtTrustLines) {
        if (utc_now() - timeAndTrustLineWithPtr.first > kResetTrustLinesDuration()) {
            auto trustLineWithPtr = timeAndTrustLineWithPtr.second;
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "deleteLegacyTrustLines\t" <<
                          trustLineWithPtr->maxFlowCalculationtrustLine()->sourceUUID() << " " <<
                 trustLineWithPtr->maxFlowCalculationtrustLine()->targetUUID() << " " <<
                 trustLineWithPtr->maxFlowCalculationtrustLine()->amount();
#endif
            auto hashSetPtr = trustLineWithPtr->hashSetPtr();
            hashSetPtr->erase(trustLineWithPtr);
            if (hashSetPtr->empty()) {
                NodeUUID keyUUID = trustLineWithPtr->topologyTrustLine()->sourceUUID();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
                info() << "deleteLegacyTrustLines\t" << "remove all trustLines for node: " << keyUUID;
#endif
                msTrustLines.erase(keyUUID);
                delete hashSetPtr;
            }
            delete trustLineWithPtr;
            mtTrustLines.erase(timeAndTrustLineWithPtr.first);
            isTrustLineWasDeleted = true;
        } else {
            break;
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "deleteLegacyTrustLines\t" << "map size after deleting: " << msTrustLines.size();
#endif
    return isTrustLineWasDeleted;
}

size_t TopologyTrustLineManager::trustLinesCounts() const
{
    size_t countTrustLines = 0;
    for (const auto &nodeUUIDAndTrustLines : msTrustLines) {
        countTrustLines += (nodeUUIDAndTrustLines.second)->size();
    }
    return countTrustLines;
}

void TopologyTrustLineManager::printTrustLines() const
{
    size_t trustLinesCnt = 0;
    info() << "print\t" << "trustLineMap size: " << msTrustLines.size();
    for (const auto &nodeUUIDAndTrustLines : msTrustLines) {
        info() << "print\t" << "key: " << nodeUUIDAndTrustLines.first;
        for (auto &itTrustLine : *nodeUUIDAndTrustLines.second) {
            TopologyTrustLine::Shared trustLine = itTrustLine->topologyTrustLine();
            info() << "print\t" << "value: " << trustLine->targetUUID() << " " << *trustLine->amount().get()
                    << " free amount: " << *trustLine->freeAmount();
        }
        trustLinesCnt += nodeUUIDAndTrustLines.second->size();
    }
    info() << "print\t" << "trust lines count: " << trustLinesCnt;
}

DateTime TopologyTrustLineManager::closestTimeEvent() const
{
    DateTime result = utc_now() + kResetTrustLinesDuration();
    // if there are cached trust lines, then take closest trust line removing time as result closest time event
    // else take trust line life time as result closest time event
    if (!mtTrustLines.empty()) {
        auto timeAndNodeUUID = mtTrustLines.cbegin();
        if (timeAndNodeUUID->first + kResetTrustLinesDuration() < result) {
            result = timeAndNodeUUID->first + kResetTrustLinesDuration();
        }
    }
    return result;
}

set<NodeUUID> TopologyTrustLineManager::neighborsOf(
    const NodeUUID &sourceUUID)
{
    set<NodeUUID> result;
    info() << "neighborsOf map size" << msTrustLines.size();
    auto const &nodeUUIDAndSetTrustLines = msTrustLines.find(sourceUUID);
    if (nodeUUIDAndSetTrustLines == msTrustLines.end()) {
        return result;
    }
    for (auto &trustLinePtr : *nodeUUIDAndSetTrustLines->second) {
        result.insert(trustLinePtr->topologyTrustLine()->targetUUID());
    }
    return result;
}

void TopologyTrustLineManager::addGateway(
    const NodeUUID &gateway)
{
    mGateways.insert(gateway);
}

const set<NodeUUID> TopologyTrustLineManager::gateways() const
{
    return mGateways;
}

void TopologyTrustLineManager::makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
    const NodeUUID &exceptedNode)
{
    for (const auto &gateway : mGateways) {
        auto const &nodeUUIDAndSetFlows = msTrustLines.find(gateway);
        if (nodeUUIDAndSetFlows == msTrustLines.end()) {
            continue;
        }
        for (auto &trustLinePtr : *nodeUUIDAndSetFlows->second) {
            const auto maxFlowTLTarget = trustLinePtr->topologyTrustLine()->targetUUID();
            if (mGateways.count(maxFlowTLTarget) != 0) {
                continue;
            }
            if (maxFlowTLTarget != exceptedNode) {
                trustLinePtr->topologyTrustLine()->setUsedAmount(
                        *trustLinePtr->topologyTrustLine()->amount().get());
            }
        }
    }
}

void TopologyTrustLineManager::setPreventDeleting(
    bool preventDeleting)
{
    mPreventDeleting = preventDeleting;
}

bool TopologyTrustLineManager::preventDeleting() const
{
    return mPreventDeleting;
}

LoggerStream TopologyTrustLineManager::info() const
{
    return mLog.info(logHeader());
}

LoggerStream TopologyTrustLineManager::debug() const
{
    return mLog.debug(logHeader());
}

const string TopologyTrustLineManager::logHeader() const
{
    stringstream s;
    s << "[TopologyTrustLineManager]";
    return s.str();
}