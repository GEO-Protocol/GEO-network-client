#include "ResourcesManager.h"

void ResourcesManager::putResource(
    BaseResource::Shared resource) {

    attachResourceSignal(
        resource);
}

template<typename... Params>
void ResourcesManager::requestResource(
    const BaseResource::ResourceType type,
    const TransactionUUID &transactionUUID,
    Params&&... params) {

    switch (type) {

        case BaseResource::ResourceType::Paths: {
            requestPathsResourcesSignal(
                transactionUUID,
                params...);
        }

        default:{
            throw ConflictError("ResourcesManager::requestResource "
                                    "Unexpected resource type");
        }

    }
}