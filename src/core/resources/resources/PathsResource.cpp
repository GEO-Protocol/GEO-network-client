#include "PathsResource.h"

PathsResource::PathsResource(
    const TransactionUUID &transactionUUID,
    PathsCollection::Shared pathsCollection):

    BaseResource(
        BaseResource::ResourceType::Paths,
        transactionUUID),

    mPathsCollection(pathsCollection){}

PathsCollection::Shared PathsResource::pathCollection() const {

    return mPathsCollection;
}
