#include "TopologyTrustLinesManager.h"

TopologyTrustLinesManager::TopologyTrustLinesManager(
    const SerializedEquivalent equivalent,
    bool iAmGateway,
    NodeUUID &nodeUUID,
    Logger &logger):

    mEquivalent(equivalent),
    mLog(logger),
    mPreventDeleting(false),
    mHigherFreeID(1)
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

void TopologyTrustLinesManager::addTrustLineNew(
    TopologyTrustLineNew::Shared trustLine)
{
    auto const &nodeIDAndSetFlows = msTrustLinesNew.find(trustLine->sourceID());
    if (nodeIDAndSetFlows == msTrustLinesNew.end()) {
        if (*(trustLine->amount()) == TrustLine::kZeroAmount()) {
            return;
        }
        auto newHashSet = new unordered_set<TopologyTrustLineWithPtrNew*>();
        auto newTrustLineWithPtr = new TopologyTrustLineWithPtrNew(
            trustLine,
            newHashSet);
        newHashSet->insert(
            newTrustLineWithPtr);

        msTrustLinesNew.insert(
            make_pair(
                trustLine->sourceID(),
                newHashSet));
        auto now = utc_now();
        if (mtTrustLinesNew.count(now) != 0) {
            now += pt::microseconds(5);
        }
        mtTrustLinesNew.insert(
            make_pair(
                now,
                newTrustLineWithPtr));
    } else {
        auto hashSet = nodeIDAndSetFlows->second;
        auto trLineWithPtr = hashSet->begin();
        while (trLineWithPtr != hashSet->end()) {
            if ((*trLineWithPtr)->topologyTrustLine()->targetID() == trustLine->targetID()) {
                (*trLineWithPtr)->topologyTrustLine()->setAmount(trustLine->amount());

                // update time creation of trustline
                auto dateTimeAndTrustLine = mtTrustLinesNew.begin();
                while (dateTimeAndTrustLine != mtTrustLinesNew.end()) {
                    if (dateTimeAndTrustLine->second == *trLineWithPtr) {
                        if (*(*trLineWithPtr)->topologyTrustLine()->amount() != TrustLine::kZeroAmount()) {
                            mtTrustLinesNew.erase(
                                dateTimeAndTrustLine);
                            auto now = utc_now();
                            if (mtTrustLinesNew.count(now) != 0) {
                                now += pt::microseconds(5);
                            }
                            mtTrustLinesNew.insert(
                                make_pair(
                                    now,
                                    *trLineWithPtr));
                        } else {
                            auto hashSetPtr = (*trLineWithPtr)->hashSetPtr();
                            hashSetPtr->erase(*trLineWithPtr);
                            if (hashSetPtr->empty()) {
                                ContractorID keyID = (*trLineWithPtr)->topologyTrustLine()->sourceID();
                                msTrustLinesNew.erase(keyID);
                                delete hashSetPtr;
                            }
                            delete *trLineWithPtr;
                            mtTrustLinesNew.erase(dateTimeAndTrustLine);
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
            auto newTrustLineWithPtr = new TopologyTrustLineWithPtrNew(
                trustLine,
                hashSet);
            hashSet->insert(
                newTrustLineWithPtr);
            auto now = utc_now();
            if (mtTrustLines.count(now) != 0) {
                now += pt::microseconds(5);
            }
            mtTrustLinesNew.insert(
                make_pair(
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

unordered_set<TopologyTrustLineWithPtrNew*> TopologyTrustLinesManager::trustLinePtrsSetNew(
    ContractorID nodeID)
{
    auto const &nodeIDAndSetFlows = msTrustLinesNew.find(nodeID);
    if (nodeIDAndSetFlows == msTrustLinesNew.end()) {
        TrustLineWithPtrHashSetNew result;
        return result;
    }
    return *nodeIDAndSetFlows->second;
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

    for (auto &nodeIDAndTrustLine : msTrustLinesNew) {
        for (auto &trustLine : *nodeIDAndTrustLine.second) {
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

void TopologyTrustLinesManager::addUsedAmountNew(
    ContractorID sourceID,
    ContractorID targetID,
    const TrustLineAmount &amount)
{
    auto const &nodeIDAndSetFlows = msTrustLinesNew.find(sourceID);
    if (nodeIDAndSetFlows == msTrustLinesNew.end()) {
        return;
    }
    for (auto &trustLinePtr : *nodeIDAndSetFlows->second) {
        if (trustLinePtr->topologyTrustLine()->targetID() == targetID) {
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

void TopologyTrustLinesManager::makeFullyUsedNew(
    ContractorID sourceID,
    ContractorID targetID)
{
    auto const &nodeIDAndSetFlows = msTrustLinesNew.find(sourceID);
    if (nodeIDAndSetFlows == msTrustLinesNew.end()) {
        return;
    }
    for (auto &trustLinePtr : *nodeIDAndSetFlows->second) {
        if (trustLinePtr->topologyTrustLine()->targetID() == targetID) {
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

bool TopologyTrustLinesManager::deleteLegacyTrustLinesNew()
{
    bool isTrustLineWasDeleted = false;
    if (mtTrustLinesNew.empty()) {
        if (utc_now() - mLastTrustLineTimeAdding > kClearTrustLinesDuration()) {
            for (auto nodeIDAndSetFlows : msTrustLinesNew) {
                auto hashSetPtr = nodeIDAndSetFlows.second;
                hashSetPtr->clear();
                delete hashSetPtr;
            }
            msTrustLinesNew.clear();
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "deleteLegacyTrustLinesNew\t" << "map size after deleting: " << msTrustLinesNew.size();
#endif
        return isTrustLineWasDeleted;
    }
    for (auto &timeAndTrustLineWithPtr : mtTrustLinesNew) {
        if (utc_now() - timeAndTrustLineWithPtr.first > kResetTrustLinesDuration()) {
            auto trustLineWithPtr = timeAndTrustLineWithPtr.second;
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "deleteLegacyTrustLinesNew\t" <<
                   trustLineWithPtr->topologyTrustLine()->sourceID() << " " <<
                   trustLineWithPtr->topologyTrustLine()->targetID() << " " <<
                   trustLineWithPtr->topologyTrustLine()->amount();
#endif
            auto hashSetPtr = trustLineWithPtr->hashSetPtr();
            hashSetPtr->erase(trustLineWithPtr);
            if (hashSetPtr->empty()) {
                ContractorID keyID = trustLineWithPtr->topologyTrustLine()->sourceID();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
                info() << "deleteLegacyTrustLinesNew\t" << "remove all trustLines for node: " << keyID;
#endif
                msTrustLinesNew.erase(keyID);
                delete hashSetPtr;
            }
            delete trustLineWithPtr;
            mtTrustLinesNew.erase(timeAndTrustLineWithPtr.first);
            isTrustLineWasDeleted = true;
        } else {
            break;
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "deleteLegacyTrustLinesNew\t" << "map size after deleting: " << msTrustLinesNew.size();
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

void TopologyTrustLinesManager::printTrustLinesNew() const
{
    info() << "participants:";
    for (const auto &participant : mParticipantsAddresses) {
        info() << participant.first->fullAddress() << " " << participant.second;
    }
    size_t trustLinesCnt = 0;
    info() << "print new\t" << "trustLineMap size: " << msTrustLinesNew.size();
    for (const auto &nodeIDAndTrustLines : msTrustLinesNew) {
        info() << "print new\t" << "key: " << nodeIDAndTrustLines.first;
        for (auto &itTrustLine : *nodeIDAndTrustLines.second) {
            TopologyTrustLineNew::Shared trustLine = itTrustLine->topologyTrustLine();
            info() << "print new\t" << "value: " << trustLine->targetID() << " " << *trustLine->amount().get()
                   << " free amount: " << *trustLine->freeAmount();
        }
        trustLinesCnt += nodeIDAndTrustLines.second->size();
    }
    info() << "print new\t" << "trust lines count: " << trustLinesCnt;

    info() << "now is " << utc_now();
    info() << "print new\t" << "timesMap size: " << mtTrustLinesNew.size();
    for (const auto &timeAndTrustLine : mtTrustLinesNew) {
        info() << "print new\t" << "key: " << timeAndTrustLine.first;
        auto trustLine = timeAndTrustLine.second->topologyTrustLine();
        info() << "print new\t" << "value: " << trustLine->targetID() << " " << *trustLine->amount().get()
               << " free amount: " << *trustLine->freeAmount();
    }
}

DateTime TopologyTrustLinesManager::closestTimeEvent() const
{
    DateTime result = utc_now() + kResetTrustLinesDuration();
    // if there are cached trust lines, then take closest trust line removing time as result closest time event
    // else take trust line life time as result closest time event
    if (!mtTrustLinesNew.empty()) {
        auto timeAndTrustLine = mtTrustLines.cbegin();
        if (timeAndTrustLine->first + kResetTrustLinesDuration() < result) {
            result = timeAndTrustLine->first + kResetTrustLinesDuration();
        }
    }
    return result;
}

void TopologyTrustLinesManager::addGateway(
    const NodeUUID &gateway)
{
    mGateways.insert(gateway);
}

void TopologyTrustLinesManager::addGatewayNew(
    ContractorID gateway)
{
    mGatewaysNew.insert(gateway);
}

const set<NodeUUID> TopologyTrustLinesManager::gateways() const
{
    return mGateways;
}

const set<ContractorID> TopologyTrustLinesManager::gatewaysNew() const
{
    return mGatewaysNew;
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

void TopologyTrustLinesManager::makeFullyUsedTLsFromGatewaysToAllNodesExceptOneNew(
    ContractorID exceptedNode)
{
    for (const auto &gateway : mGatewaysNew) {
        auto const &nodeIDAndSetFlows = msTrustLinesNew.find(gateway);
        if (nodeIDAndSetFlows == msTrustLinesNew.end()) {
            continue;
        }
        for (auto &trustLinePtr : *nodeIDAndSetFlows->second) {
            const auto maxFlowTLTarget = trustLinePtr->topologyTrustLine()->targetID();
            if (mGatewaysNew.count(maxFlowTLTarget) != 0) {
                continue;
            }
            if (maxFlowTLTarget != exceptedNode) {
                trustLinePtr->topologyTrustLine()->setUsedAmount(
                    *trustLinePtr->topologyTrustLine()->amount().get());
            }
        }
    }
}

const TrustLineAmount& TopologyTrustLinesManager::flowAmount(
    const NodeUUID &source,
    const NodeUUID &destination)
{
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(source);
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        return TrustLine::kZeroAmount();
    }
    for (auto &trustLinePtr : *nodeUUIDAndSetFlows->second) {
        if (trustLinePtr->topologyTrustLine()->targetUUID() == destination) {
            return *trustLinePtr->topologyTrustLine()->amount();
        }
    }
    return TrustLine::kZeroAmount();
}

const TrustLineAmount& TopologyTrustLinesManager::flowAmountNew(
    ContractorID source,
    ContractorID destination)
{
    auto const &nodeIDAndSetFlows = msTrustLinesNew.find(source);
    if (nodeIDAndSetFlows == msTrustLinesNew.end()) {
        return TrustLine::kZeroAmount();
    }
    for (auto &trustLinePtr : *nodeIDAndSetFlows->second) {
        if (trustLinePtr->topologyTrustLine()->targetID() == destination) {
            return *trustLinePtr->topologyTrustLine()->amount();
        }
    }
    return TrustLine::kZeroAmount();
}

ContractorID TopologyTrustLinesManager::getID(
    BaseAddress::Shared address)
{
    for (const auto &participantAddress : mParticipantsAddresses) {
        if (participantAddress.first == address) {
            return participantAddress.second;
        }
    }
    mParticipantsAddresses.emplace_back(
        address,
        mHigherFreeID);
    auto result = mHigherFreeID;
    mHigherFreeID++;
    return result;
}

// todo : improve this code for preventing loop
BaseAddress::Shared TopologyTrustLinesManager::getAddressByID(
    ContractorID nodeID) const
{
    for (const auto &participant : mParticipantsAddresses) {
        if (participant.second == nodeID) {
            return participant.first;
        }
    }
    return nullptr;
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