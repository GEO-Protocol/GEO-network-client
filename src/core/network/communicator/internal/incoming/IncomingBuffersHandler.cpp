#include "IncomingBuffersHandler.h"


vector<byte> *IncomingBuffersHandler::buffer (
    const UDPEndpoint &endpoint)
    noexcept
{
    if (0 == mIncomingBuffers.count(key(endpoint))) {
        mIncomingBuffers[key(endpoint)] = make_unique<vector<byte>>();
    }

    return mIncomingBuffers[key(endpoint)].get();
}

void IncomingBuffersHandler::dropBuffer (
    const UDPEndpoint &endpoint)
    noexcept
{
    if (0 != mIncomingBuffers.count(key(endpoint))) {
        mIncomingBuffers.erase(key(endpoint));
    }
}

/**
 * Returns 8 bytes uisnigned interger,
 * where first 4 bytes - are ip address,
 * and the rest 4 (actually only 2) - port.
 */
uint64_t IncomingBuffersHandler::key(
    const UDPEndpoint &endpoint)
    noexcept
{
    return
        endpoint.address().to_v4().to_ulong()
        + endpoint.port();
}
