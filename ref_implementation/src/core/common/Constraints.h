/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_CONSTANTS_H
#define GEO_NETWORK_CLIENT_CONSTANTS_H

#include "Types.h"


static const uint16_t kMaxNeighborNodesCount = std::numeric_limits<uint16_t>::max() - 10;


// Routing tables
static const uint8_t kRoutingTablesMaxLevel = 2; // 0, 1, 2, three in total


#endif //GEO_NETWORK_CLIENT_CONSTANTS_H
