#include "ResourcesManager.h"

void ResourcesManager::putResource(
    BaseResource::Shared resource) {

    attachResourceSignal(
        resource);
}

void ResourcesManager::requestPaths(
    const TransactionUUID &transactionUUID,
    BaseAddress::Shared contractorAddress,
    const SerializedEquivalent equivalent) const {

    requestPathsResourcesSignal(
        transactionUUID,
        contractorAddress,
        equivalent);

}
