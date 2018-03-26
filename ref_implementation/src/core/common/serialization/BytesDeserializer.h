/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_BYTESDESERIALIZER_H
#define GEO_NETWORK_CLIENT_BYTESDESERIALIZER_H

#include "../Types.h"
#include "../NodeUUID.h"
#include "../memory/MemoryUtils.h"


using namespace std;


class BytesDeserializer {
public:
    const BytesShared buffer;

public:
    BytesDeserializer(
        BytesShared buffer,
        size_t initialOffset=0)
        noexcept;

    void copyInto (
        byte *b)
        noexcept;

    void copyIntoDespiteConst (
        const byte *b)
        noexcept;

    void copyInto (
        uint16_t *v)
        noexcept;

    void copyIntoDespiteConst (
        const uint16_t *v)
        noexcept;

    void copyInto (
        uint32_t *v)
        noexcept;

    void copyIntoDespiteConst (
        const uint32_t *v)
        noexcept;

    void copyInto (
        NodeUUID *nodeUUID)
        noexcept;

    void copyIntoDespiteConst (
        const NodeUUID *nodeUUID)
        noexcept;

    void copyInto (
        void *destination,
        const size_t bytesCount)
        noexcept;

protected:
    size_t mCurrentOffset;
};


#endif //GEO_NETWORK_CLIENT_BYTESDESERIALIZER_H
