#include "MaxFlowCalculationTrustLineManager.h"

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
            if ((*trLineWithPtr)->maxFlowCalculationtrustLine()->sourceUUID() == trustLine->sourceUUID()
                && (*trLineWithPtr)->maxFlowCalculationtrustLine()->targetUUID() == trustLine->targetUUID()) {
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

    cout << "delete legacy trustLines set:\n";

    cout << "cached trustLines:\n";
    for (auto &nodeUUIDAndTrustLine : msTrustLines) {
        cout << "key: " << nodeUUIDAndTrustLine.first.stringUUID() << "\n";
        for (auto trustLine = nodeUUIDAndTrustLine.second->begin();
             trustLine != nodeUUIDAndTrustLine.second->end(); ++trustLine) {
            cout << "\tvalue: " << (*trustLine)->maxFlowCalculationtrustLine()->sourceUUID() << "->" << (*trustLine)->maxFlowCalculationtrustLine()->targetUUID() <<
                 " " << (*trustLine)->maxFlowCalculationtrustLine()->amount() <<"\n";
        }
    }

    cout << "deleteion\n";
    for (auto &timeAndTrustLineWithPtr : mtTrustLines) {
        cout << "time created: " << timeAndTrustLineWithPtr.first << "\n";
        if (utc_now() - timeAndTrustLineWithPtr.first > kResetTrustLinesDuration()) {
            auto trustLineWithPtr = timeAndTrustLineWithPtr.second;
            cout << ((NodeUUID) trustLineWithPtr->maxFlowCalculationtrustLine()->sourceUUID()).stringUUID() << " " <<
                 ((NodeUUID) trustLineWithPtr->maxFlowCalculationtrustLine()->targetUUID()).stringUUID() << " " <<
                 trustLineWithPtr->maxFlowCalculationtrustLine()->amount() <<"\n";
            auto hashSetPtr = trustLineWithPtr->hashSetPtr();
            hashSetPtr->erase(trustLineWithPtr);
            if (hashSetPtr->size() == 0) {
                NodeUUID keyUUID = trustLineWithPtr->maxFlowCalculationtrustLine()->sourceUUID();
                cout << "remove all trustLines for node: " << keyUUID.stringUUID() << "\n";
                msTrustLines.erase(keyUUID);
                delete hashSetPtr;
            }
            delete trustLineWithPtr;
            mtTrustLines.erase(timeAndTrustLineWithPtr.first);
        } else {
            break;
        }
    }
    cout << "map size after deleting: " << msTrustLines.size() << "\n";
}