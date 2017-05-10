#include "MaxFlowCalculationTrustLineManager.h"

MaxFlowCalculationTrustLineManager::MaxFlowCalculationTrustLineManager(
    Logger *logger):
    mLog(logger)
{}

void MaxFlowCalculationTrustLineManager::addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine)
{
    auto const &nodeUUIDAndSetFlows = msTrustLines.find(trustLine->sourceUUID());
    if (nodeUUIDAndSetFlows == msTrustLines.end()) {
        auto newHashSet = new unordered_set<MaxFlowCalculationTrustLineWithPtr*>();
        auto newTrustLineWithPtr = new MaxFlowCalculationTrustLineWithPtr(trustLine, newHashSet);
        newHashSet->insert(newTrustLineWithPtr);
        msTrustLines.insert(make_pair(trustLine->sourceUUID(), newHashSet));
        mtTrustLines.insert(make_pair(utc_now(), newTrustLineWithPtr));
    } else {
        auto hashSet = nodeUUIDAndSetFlows->second;
        auto trLineWithPtr = hashSet->begin();
        while (trLineWithPtr != hashSet->end()) {
            if ((*trLineWithPtr)->maxFlowCalculationtrustLine()->targetUUID() == trustLine->targetUUID()) {
                (*trLineWithPtr)->maxFlowCalculationtrustLine()->setAmount(trustLine->amount());
                break;
            }
            trLineWithPtr++;
        }
        if (trLineWithPtr == hashSet->end()) {
            auto newTrustLineWithPtr = new MaxFlowCalculationTrustLineWithPtr(trustLine, hashSet);
            hashSet->insert(newTrustLineWithPtr);
            mtTrustLines.insert(make_pair(utc_now(), newTrustLineWithPtr));
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
        result.push_back(trustLineAndPtr->maxFlowCalculationtrustLine());
    }
    std::sort(result.begin(), result.end(), customLess);
    return result;
}

void MaxFlowCalculationTrustLineManager::resetAllUsedAmounts()
{
    for (auto &nodeUUIDAndTrustLine : msTrustLines) {
        for (auto &trustLine : *nodeUUIDAndTrustLine.second) {
            trustLine->maxFlowCalculationtrustLine()->setUsedAmount(0);
        }
    }
}

void MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines()
{
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    info() << "deleteLegacyTrustLines\t" << "delete legacy trustLines set";
    info() << "deleteLegacyTrustLines\t" << "mapTrustLinesCount: " << trustLinesCounts();

    printTrustLines();

    info() << "deleteLegacyTrustLines\t" << "deleteion";
#endif
    for (auto &timeAndTrustLineWithPtr : mtTrustLines) {
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
        info() << "deleteLegacyTrustLines\t" << "time created: " << timeAndTrustLineWithPtr.first;
#endif
        if (utc_now() - timeAndTrustLineWithPtr.first > kResetTrustLinesDuration()) {
            auto trustLineWithPtr = timeAndTrustLineWithPtr.second;
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
            info() << "deleteLegacyTrustLines\t" <<
                          trustLineWithPtr->maxFlowCalculationtrustLine()->sourceUUID() << " " <<
                 trustLineWithPtr->maxFlowCalculationtrustLine()->targetUUID() << " " <<
                 trustLineWithPtr->maxFlowCalculationtrustLine()->amount();
#endif
            auto hashSetPtr = trustLineWithPtr->hashSetPtr();
            hashSetPtr->erase(trustLineWithPtr);
            if (hashSetPtr->size() == 0) {
                NodeUUID keyUUID = trustLineWithPtr->maxFlowCalculationtrustLine()->sourceUUID();
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
                info() << "deleteLegacyTrustLines\t" << "remove all trustLines for node: " << keyUUID;
#endif
                msTrustLines.erase(keyUUID);
                delete hashSetPtr;
            }
            delete trustLineWithPtr;
            mtTrustLines.erase(timeAndTrustLineWithPtr.first);
        } else {
            break;
        }
    }
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    info() << "deleteLegacyTrustLines\t" << "map size after deleting: " << msTrustLines.size();
#endif
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
            info() << "print\t" << "value: " << trustLine->targetUUID() << " " << trustLine->amount();
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

LoggerStream MaxFlowCalculationTrustLineManager::info() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");

    return mLog->info(logHeader());
}

const string MaxFlowCalculationTrustLineManager::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTrustLineManager]";
    return s.str();
}