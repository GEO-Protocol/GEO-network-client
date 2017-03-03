#include "MaxFlowCalculationTrustLineManager.h"

void MaxFlowCalculationTrustLineManager::addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine) {
    auto const &it = mvTrustLines.find(trustLine->sourceUUID());
    if (it == mvTrustLines.end()) {
        vector<MaxFlowCalculationTrustLine::Shared> newVect;
        newVect.push_back(trustLine);
        mvTrustLines.insert(make_pair(trustLine->sourceUUID(), newVect));
    } else {
        for (auto trLine = it->second.begin(); trLine != it->second.end(); trLine++) {
            if ((*trLine)->sourceUUID() == trustLine->sourceUUID()
                && (*trLine)->targetUUID() == trustLine->targetUUID()) {
                it->second.erase(trLine);
                break;
            }
        }
        it->second.push_back(trustLine);
    }
}

vector<MaxFlowCalculationTrustLine::Shared> MaxFlowCalculationTrustLineManager::sortedTrustLines(
    const NodeUUID &nodeUUID) {

    vector<MaxFlowCalculationTrustLine::Shared> result;
    auto const &nodeUUIDAndVector = mvTrustLines.find(nodeUUID);
    if (nodeUUIDAndVector == mvTrustLines.end()) {
        return result;
    }

    for (auto const &trustLine : nodeUUIDAndVector->second) {
        result.push_back(trustLine);
    }

    std::sort(result.begin(), result.end(), customLess);
    return result;
}

void MaxFlowCalculationTrustLineManager::resetAllUsedAmounts() {

    for (auto &nodeUUIDAndTrustLine : mvTrustLines) {
        for (auto &trustLine : nodeUUIDAndTrustLine.second) {
            trustLine->setUsedAmount(0);
        }
    }
}

void MaxFlowCalculationTrustLineManager::deleteLegacyTrustLines() {

    cout << "delete legacy trustLines:\n";

    cout << "cached trustLines:\n";
    for (auto &nodeUUIDAndTrustLine : mvTrustLines) {
        cout << "key: " << nodeUUIDAndTrustLine.first.stringUUID() << "\n";
        for (auto trustLine = nodeUUIDAndTrustLine.second.begin();
             trustLine != nodeUUIDAndTrustLine.second.end(); ++trustLine) {
            cout << "\tvalue: " << (*trustLine)->sourceUUID() << "->" << (*trustLine)->targetUUID() <<
                                " " << (*trustLine)->amount() << " " << (*trustLine)->timeInserted() <<"\n";
        }
    }

    for (auto &nodeUUIDAndTrustLine : mvTrustLines) {
        auto trustLine = nodeUUIDAndTrustLine.second.begin();
        while (trustLine != nodeUUIDAndTrustLine.second.end()) {
            if (utc_now() - (*trustLine)->timeInserted() > kResetTrustLinesDuration()) {
                cout << "remove trustLine: " << (*trustLine)->sourceUUID() << "->" << (*trustLine)->targetUUID() <<
                     " " << (*trustLine)->amount() << "\n";
                nodeUUIDAndTrustLine.second.erase(trustLine);
            } else {
                trustLine++;
            }
        }
        if (nodeUUIDAndTrustLine.second.size() == 0) {
            cout << "remove all trustLines for node: " << nodeUUIDAndTrustLine.first.stringUUID() << "\n";
            mvTrustLines.erase(nodeUUIDAndTrustLine.first);
        }
    }
    cout << "map size after deleting: " << mvTrustLines.size() << "\n";
}