#include "TopologyTrustLinesManager.h"

TopologyTrustLinesManager::TopologyTrustLinesManager(
    const SerializedEquivalent equivalent,
    bool iAmGateway,
    Logger &logger):

    mEquivalent(equivalent),
    mLog(logger),
    mPreventDeleting(false),
    mHigherFreeID(1)
{
    // todo : use here kCurrentNodeID
    if (iAmGateway) {
        mGateways.insert(0);
    }
}

void TopologyTrustLinesManager::addTrustLine(
    TopologyTrustLine::Shared trustLine)
{
    auto const &nodeIDAndSetFlows = msTrustLines.find(trustLine->sourceID());
    if (nodeIDAndSetFlows == msTrustLines.end()) {
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
                trustLine->sourceID(),
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
        auto hashSet = nodeIDAndSetFlows->second;
        auto trLineWithPtrIt = hashSet->begin();
        while (trLineWithPtrIt != hashSet->end()) {
            if ((*trLineWithPtrIt)->topologyTrustLine()->targetID() == trustLine->targetID()) {
                (*trLineWithPtrIt)->topologyTrustLine()->setAmount(trustLine->amount());

                // update time creation of trustline
                auto dateTimeAndTrustLine = mtTrustLines.begin();
                while (dateTimeAndTrustLine != mtTrustLines.end()) {
                    if (dateTimeAndTrustLine->second == *trLineWithPtrIt) {
                        if (*(*trLineWithPtrIt)->topologyTrustLine()->amount() != TrustLine::kZeroAmount()) {
                            mtTrustLines.erase(
                                dateTimeAndTrustLine);
                            auto now = utc_now();
                            if (mtTrustLines.count(now) != 0) {
                                now += pt::microseconds(5);
                            }
                            mtTrustLines.insert(
                                make_pair(
                                    now,
                                    *trLineWithPtrIt));
                        } else {
                            auto trLineWithPtr = *trLineWithPtrIt;
                            hashSet->erase(trLineWithPtr);
                            delete trLineWithPtr;
                            mtTrustLines.erase(dateTimeAndTrustLine);
                        }
                        break;
                    }
                    dateTimeAndTrustLine++;
                }

                break;
            }
            trLineWithPtrIt++;
        }
        if (hashSet->empty()) {
            msTrustLines.erase(nodeIDAndSetFlows->first);
            delete hashSet;
        } else if (trLineWithPtrIt == hashSet->end()) {
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
            mtTrustLines.insert(
                make_pair(
                    now,
                    newTrustLineWithPtr));
        }
    }
    mLastTrustLineTimeAdding = utc_now();
}

unordered_set<TopologyTrustLineWithPtr*> TopologyTrustLinesManager::trustLinePtrsSet(
    ContractorID nodeID)
{
    auto const &nodeIDAndSetFlows = msTrustLines.find(nodeID);
    if (nodeIDAndSetFlows == msTrustLines.end()) {
        TrustLineWithPtrHashSet result;
        return result;
    }
    return *nodeIDAndSetFlows->second;
}

void TopologyTrustLinesManager::resetAllUsedAmounts()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "resetAllUsedAmounts";
#endif
    for (auto &nodeIDAndTrustLine : msTrustLines) {
        for (auto &trustLine : *nodeIDAndTrustLine.second) {
            trustLine->topologyTrustLine()->setUsedAmount(0);
        }
    }
}

void TopologyTrustLinesManager::addUsedAmount(
    ContractorID sourceID,
    ContractorID targetID,
    const TrustLineAmount &amount)
{
    auto const &nodeIDAndSetFlows = msTrustLines.find(sourceID);
    if (nodeIDAndSetFlows == msTrustLines.end()) {
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
    ContractorID sourceID,
    ContractorID targetID)
{
    auto const &nodeIDAndSetFlows = msTrustLines.find(sourceID);
    if (nodeIDAndSetFlows == msTrustLines.end()) {
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
            for (auto nodeIDAndSetFlows : msTrustLines) {
                auto hashSetPtr = nodeIDAndSetFlows.second;
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
                   trustLineWithPtr->topologyTrustLine()->sourceID() << " " <<
                   trustLineWithPtr->topologyTrustLine()->targetID() << " " <<
                   trustLineWithPtr->topologyTrustLine()->amount();
#endif
            auto hashSetPtr = trustLineWithPtr->hashSetPtr();
            hashSetPtr->erase(trustLineWithPtr);
            if (hashSetPtr->empty()) {
                ContractorID keyID = trustLineWithPtr->topologyTrustLine()->sourceID();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
                info() << "deleteLegacyTrustLines\t" << "remove all trustLines for node: " << keyID;
#endif
                msTrustLines.erase(keyID);
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
    info() << "deleteLegacyTrustLinesNew\t" << "map size after deleting: " << msTrustLines.size();
#endif
    return isTrustLineWasDeleted;
}

size_t TopologyTrustLinesManager::trustLinesCounts() const
{
    size_t countTrustLines = 0;
    for (const auto &contractoIDAndTrustLines : msTrustLines) {
        countTrustLines += (contractoIDAndTrustLines.second)->size();
    }
    return countTrustLines;
}

void TopologyTrustLinesManager::printTrustLines() const
{
    info() << "participants:";
    for (const auto &participant : mParticipantsAddresses) {
        info() << participant.first->fullAddress() << " " << participant.second;
    }
    size_t trustLinesCnt = 0;
    info() << "print new\t" << "trustLineMap size: " << msTrustLines.size();
    for (const auto &nodeIDAndTrustLines : msTrustLines) {
        info() << "print new\t" << "key: " << nodeIDAndTrustLines.first;
        for (auto &itTrustLine : *nodeIDAndTrustLines.second) {
            TopologyTrustLine::Shared trustLine = itTrustLine->topologyTrustLine();
            info() << "print new\t" << "value: " << trustLine->targetID() << " " << *trustLine->amount().get()
                   << " free amount: " << *trustLine->freeAmount();
        }
        trustLinesCnt += nodeIDAndTrustLines.second->size();
    }
    info() << "print new\t" << "trust lines count: " << trustLinesCnt;

    info() << "now is " << utc_now();
    info() << "print new\t" << "timesMap size: " << mtTrustLines.size();
    for (const auto &timeAndTrustLine : mtTrustLines) {
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
    if (!mtTrustLines.empty()) {
        auto timeAndTrustLine = mtTrustLines.cbegin();
        if (timeAndTrustLine->first + kResetTrustLinesDuration() < result) {
            result = timeAndTrustLine->first + kResetTrustLinesDuration();
        }
    }
    return result;
}

void TopologyTrustLinesManager::addGateway(
    ContractorID gateway)
{
    mGateways.insert(gateway);
}

const set<ContractorID> TopologyTrustLinesManager::gateways() const
{
    return mGateways;
}

void TopologyTrustLinesManager::makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
    ContractorID exceptedNode)
{
    for (const auto &gateway : mGateways) {
        auto const &nodeIDAndSetFlows = msTrustLines.find(gateway);
        if (nodeIDAndSetFlows == msTrustLines.end()) {
            continue;
        }
        for (auto &trustLinePtr : *nodeIDAndSetFlows->second) {
            const auto maxFlowTLTarget = trustLinePtr->topologyTrustLine()->targetID();
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

const TrustLineAmount& TopologyTrustLinesManager::flowAmount(
    ContractorID source,
    ContractorID destination)
{
    auto const &nodeIDAndSetFlows = msTrustLines.find(source);
    if (nodeIDAndSetFlows == msTrustLines.end()) {
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