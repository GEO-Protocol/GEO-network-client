#include "TopologyTrustLinesManager.h"

TopologyTrustLinesManager::TopologyTrustLinesManager(
    const SerializedEquivalent equivalent,
    bool iAmGateway,
    NodeUUID &nodeUUID,
    Logger &logger):

    mEquivalent(equivalent),
    mLog(logger),
    mPreventDeleting(false)
{
    if (iAmGateway) {
        mGateways.insert(nodeUUID);
    }
}

void TopologyTrustLinesManager::addTrustLine(
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
        auto now = utc_now();
        if (mtTrustLines.count(now) != 0) {
            now += pt::microseconds(5);
        }
        mtTrustLines.insert(
            make_pair(
                now,
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
                            auto now = utc_now();
                            if (mtTrustLines.count(now) != 0) {
                                now += pt::microseconds(5);
                            }
                            mtTrustLines.insert(
                                make_pair(
                                    now,
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
            auto now = utc_now();
            if (mtTrustLines.count(now) != 0) {
                now += pt::microseconds(5);
            }
            mtTrustLines.insert
                (make_pair(
                    now,
                    newTrustLineWithPtr));
        }
    }
    mLastTrustLineTimeAdding = utc_now();
}

unordered_set<TopologyTrustLineWithPtr*> TopologyTrustLinesManager::trustLinePtrsSet(
    const NodeUUID &nodeUUID)
{
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(nodeUUID);
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        TrustLineWithPtrHashSet result;
        return result;
    }
    return *nodeUUIDAndSetFlows->second;
}

void TopologyTrustLinesManager::resetAllUsedAmounts()
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

void TopologyTrustLinesManager::addUsedAmount(
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

void TopologyTrustLinesManager::makeFullyUsed(
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

bool TopologyTrustLinesManager::deleteLegacyTrustLines()
{
    bool isTrustLineWasDeleted = false;
    if (mtTrustLines.empty()) {
        if (utc_now() - mLastTrustLineTimeAdding > kClearTrustLinesDuration()) {
            for (auto nodeUUIDAndSetFlows : msTrustLines) {
                auto hashSetPtr = nodeUUIDAndSetFlows.second;
                hashSetPtr->clear();
                delete hashSetPtr;
            }
            msTrustLines.clear();
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "deleteLegacyTrustLines\t" << "map size after deleting: " << msTrustLines.size();
#endif
        return isTrustLineWasDeleted;
    }
    for (auto &timeAndTrustLineWithPtr : mtTrustLines) {
        if (utc_now() - timeAndTrustLineWithPtr.first > kResetTrustLinesDuration()) {
            auto trustLineWithPtr = timeAndTrustLineWithPtr.second;
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "deleteLegacyTrustLines\t" <<
                          trustLineWithPtr->topologyTrustLine()->sourceUUID() << " " <<
                 trustLineWithPtr->topologyTrustLine()->targetUUID() << " " <<
                 trustLineWithPtr->topologyTrustLine()->amount();
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

size_t TopologyTrustLinesManager::trustLinesCounts() const
{
    size_t countTrustLines = 0;
    for (const auto &nodeUUIDAndTrustLines : msTrustLines) {
        countTrustLines += (nodeUUIDAndTrustLines.second)->size();
    }
    return countTrustLines;
}

void TopologyTrustLinesManager::printTrustLines() const
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

    info() << "now is " << utc_now();
    info() << "print\t" << "timesMap size: " << mtTrustLines.size();
    for (const auto &timeAndTrustLine : mtTrustLines) {
        info() << "print\t" << "key: " << timeAndTrustLine.first;
        auto trustLine = timeAndTrustLine.second->topologyTrustLine();
        info() << "print\t" << "value: " << trustLine->targetUUID() << " " << *trustLine->amount().get()
               << " free amount: " << *trustLine->freeAmount();
    }
}

DateTime TopologyTrustLinesManager::closestTimeEvent() const
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

set<NodeUUID> TopologyTrustLinesManager::neighborsOf(
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

void TopologyTrustLinesManager::addGateway(
    const NodeUUID &gateway)
{
    mGateways.insert(gateway);
}

const set<NodeUUID> TopologyTrustLinesManager::gateways() const
{
    return mGateways;
}

void TopologyTrustLinesManager::makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
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

void TopologyTrustLinesManager::setPreventDeleting(
    bool preventDeleting)
{
    mPreventDeleting = preventDeleting;
}

bool TopologyTrustLinesManager::preventDeleting() const
{
    return mPreventDeleting;
}

LoggerStream TopologyTrustLinesManager::info() const
{
    return mLog.info(logHeader());
}

LoggerStream TopologyTrustLinesManager::debug() const
{
    return mLog.debug(logHeader());
}

const string TopologyTrustLinesManager::logHeader() const
{
    stringstream s;
    s << "[TopologyTrustLinesManager: " << mEquivalent << "] ";
    return s.str();
}