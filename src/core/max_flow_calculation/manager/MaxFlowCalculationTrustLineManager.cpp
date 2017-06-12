#include "MaxFlowCalculationTrustLineManager.h"

MaxFlowCalculationTrustLineManager::MaxFlowCalculationTrustLineManager(
    Logger &logger):
    mLog(logger),
    mPreventDeleting(false)
{}

void MaxFlowCalculationTrustLineManager::addTrustLine(
    MaxFlowCalculationTrustLine::Shared trustLine)
{
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(trustLine->sourceUUID());
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        auto newHashSet = new unordered_set<MaxFlowCalculationTrustLineWithPtr*>();
        auto newTrustLineWithPtr = new MaxFlowCalculationTrustLineWithPtr(
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
            if ((*trLineWithPtr)->maxFlowCalculationtrustLine()->targetUUID() == trustLine->targetUUID()) {
                (*trLineWithPtr)->maxFlowCalculationtrustLine()->setAmount(trustLine->amount());

                // update time creation of trustline
                auto dateTimeAndTrustLine = mtTrustLines.begin();
                while (dateTimeAndTrustLine != mtTrustLines.end()) {
                    if (dateTimeAndTrustLine->second == *trLineWithPtr) {
                        mtTrustLines.erase(
                            dateTimeAndTrustLine);
                        mtTrustLines.insert(
                            make_pair(
                                utc_now(),
                                *trLineWithPtr));
                        break;
                    }
                    dateTimeAndTrustLine++;

                }

                break;
            }
            trLineWithPtr++;
        }
        if (trLineWithPtr == hashSet->end()) {
            auto newTrustLineWithPtr = new MaxFlowCalculationTrustLineWithPtr(
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

vector<MaxFlowCalculationTrustLine::Shared> MaxFlowCalculationTrustLineManager::sortedTrustLines(
    const NodeUUID &nodeUUID)
{
    vector<MaxFlowCalculationTrustLine::Shared> result;
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(nodeUUID);
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        return result;
    }
    for (auto trustLineAndPtr : *nodeUUIDAndSetFlows->second) {
        result.push_back(
            trustLineAndPtr->maxFlowCalculationtrustLine());
    }
    std::sort(result.begin(), result.end(), customLess);
    return result;
}

void MaxFlowCalculationTrustLineManager::resetAllUsedAmounts()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "resetAllUsedAmounts";
#endif
    for (auto &nodeUUIDAndTrustLine : msTrustLines) {
        for (auto &trustLine : *nodeUUIDAndTrustLine.second) {
            trustLine->maxFlowCalculationtrustLine()->setUsedAmount(0);
        }
    }
}

bool MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines()
{
   bool isTrustLineWasDeleted = false;
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "deleteLegacyTrustLines\t" << "delete legacy trustLines set";
    info() << "deleteLegacyTrustLines\t" << "mapTrustLinesCount: " << trustLinesCounts();

    printTrustLines();

    info() << "deleteLegacyTrustLines\t" << "deleteion";
#endif
    for (auto &timeAndTrustLineWithPtr : mtTrustLines) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "deleteLegacyTrustLines\t" << "time created: " << timeAndTrustLineWithPtr.first;
#endif
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
            if (hashSetPtr->size() == 0) {
                NodeUUID keyUUID = trustLineWithPtr->maxFlowCalculationtrustLine()->sourceUUID();
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

size_t MaxFlowCalculationTrustLineManager::trustLinesCounts() const
{
    size_t countTrustLines = 0;
    for (const auto &nodeUUIDAndTrustLines : msTrustLines) {
        countTrustLines += (nodeUUIDAndTrustLines.second)->size();
    }
    return countTrustLines;
}

void MaxFlowCalculationTrustLineManager::printTrustLines() const
{
    info() << "print\t" << "trustLineMap size: " << msTrustLines.size();
    for (const auto &nodeUUIDAndTrustLines : msTrustLines) {
        info() << "print\t" << "key: " << nodeUUIDAndTrustLines.first;
        for (auto &itTrustLine : *nodeUUIDAndTrustLines.second) {
            MaxFlowCalculationTrustLine::Shared trustLine = itTrustLine->maxFlowCalculationtrustLine();
            info() << "print\t" << "value: " << trustLine->targetUUID() << " " << *trustLine->amount().get();
        }
    }
}

DateTime MaxFlowCalculationTrustLineManager::closestTimeEvent() const
{
    DateTime result = utc_now() + kResetTrustLinesDuration();
    // if there are cached trust lines, then take closest trust line removing time as result closest time event
    // else take trust line life time as result closest time event
    if (mtTrustLines.size() > 0) {
        auto timeAndNodeUUID = mtTrustLines.cbegin();
        if (timeAndNodeUUID->first + kResetTrustLinesDuration() < result) {
            result = timeAndNodeUUID->first + kResetTrustLinesDuration();
        }
    }
    return result;
}

void MaxFlowCalculationTrustLineManager::setPreventDeleting(
    bool preventDeleting)
{
    mPreventDeleting = preventDeleting;
}

bool MaxFlowCalculationTrustLineManager::preventDeleting() const
{
    return mPreventDeleting;
}

LoggerStream MaxFlowCalculationTrustLineManager::info() const
{
    return mLog.info(logHeader());
}

const string MaxFlowCalculationTrustLineManager::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTrustLineManager]";
    return s.str();
}