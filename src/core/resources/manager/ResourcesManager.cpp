#include "ResourcesManager.h"

void ResourcesManager::putResource(
    BaseResource::Shared resource) {

    attachResourceSignal(
        resource);
}

void ResourcesManager::requestPaths(
    const TransactionUUID &transactionUUID,
    const NodeUUID &contractorUUID) const {

    requestPathsResourcesSignal(
        transactionUUID,
        contractorUUID);

}
