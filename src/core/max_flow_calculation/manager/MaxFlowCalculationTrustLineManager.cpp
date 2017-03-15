#include "MaxFlowCalculationTrustLineManager.h"

MaxFlowCalculationTrustLineManager::MaxFlowCalculationTrustLineManager(
        Logger *logger):
        mLog(logger) {}

void MaxFlowCalculationTrustLineManager::addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine) {
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
        const NodeUUID &nodeUUID) {

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

void MaxFlowCalculationTrustLineManager::resetAllUsedAmounts() {

    for (auto &nodeUUIDAndTrustLine : msTrustLines) {
        for (auto &trustLine : *nodeUUIDAndTrustLine.second) {
            trustLine->maxFlowCalculationtrustLine()->setUsedAmount(0);
        }
    }
}

void MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines() {

    mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "delete legacy trustLines set");

//#ifdef TESTS
    uint32_t countTrustLines = 0;
    for (const auto &nodeUUIDAndTrustLines : msTrustLines) {
        countTrustLines += (nodeUUIDAndTrustLines.second)->size();
    }
    mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "mapTrustLinesCount: " + to_string(countTrustLines));
//#endif

    mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "cached trustLines:");
    for (auto &nodeUUIDAndTrustLine : msTrustLines) {
        mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "key: " + nodeUUIDAndTrustLine.first.stringUUID());
        for (auto trustLine = nodeUUIDAndTrustLine.second->begin();
             trustLine != nodeUUIDAndTrustLine.second->end(); ++trustLine) {
            mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "\tvalue: " +
                    (*trustLine)->maxFlowCalculationtrustLine()->sourceUUID().stringUUID() +
                    "->" + (*trustLine)->maxFlowCalculationtrustLine()->targetUUID().stringUUID());/* + " " +
                    to_string((uint32_t)((*trustLine)->maxFlowCalculationtrustLine()->amount())));*/
        }
    }

    mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "deleteion");
    for (auto &timeAndTrustLineWithPtr : mtTrustLines) {
        //mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "time created: " + to_string(timeAndTrustLineWithPtr.first));
        if (utc_now() - timeAndTrustLineWithPtr.first > kResetTrustLinesDuration()) {
            auto trustLineWithPtr = timeAndTrustLineWithPtr.second;
            mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines",
                          ((NodeUUID) trustLineWithPtr->maxFlowCalculationtrustLine()->sourceUUID()).stringUUID() + " " +
                 ((NodeUUID) trustLineWithPtr->maxFlowCalculationtrustLine()->targetUUID()).stringUUID() + " " +
                 to_string((uint32_t)trustLineWithPtr->maxFlowCalculationtrustLine()->amount()));
            auto hashSetPtr = trustLineWithPtr->hashSetPtr();
            hashSetPtr->erase(trustLineWithPtr);
            if (hashSetPtr->size() == 0) {
                NodeUUID keyUUID = trustLineWithPtr->maxFlowCalculationtrustLine()->sourceUUID();
                mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "remove all trustLines for node: " + keyUUID.stringUUID());
                msTrustLines.erase(keyUUID);
                delete hashSetPtr;
            }
            delete trustLineWithPtr;
            mtTrustLines.erase(timeAndTrustLineWithPtr.first);
        } else {
            break;
        }
    }
    mLog->logInfo("MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines", "map size after deleting: " + to_string(msTrustLines.size()));
}