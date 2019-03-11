#include "src/core/Core.h"

#ifdef INTERNAL_TESTS
#define CATCH_CONFIG_MAIN
#include "src/tests/catch.hpp"
#include "src/tests/TestIncludes.h"
#endif

#ifdef UNIT_TESTS
#define TESTS__DB__UUID_COLUMN
#define TESTS__DB__TRUST_LINE_DIRECTION_COLUMN
#define TESTS__ROUTING_TABLE
#define TESTS__TRUSTLINES
#endif

#ifdef TESTS__DB__UUID_COLUMN
#include "src/tests/db/fields/uuid_column/UUIDColumnTests.cpp"
#endif

#ifdef TESTS__DB__TRUST_LINE_DIRECTION_COLUMN
#include "src/tests/db/fields/tl_direction_column/TrustLineDirectionColumnTests.cpp"
#endif

#ifdef TESTS__ROUTING_TABLE
#include "src/tests/io/routing_tables/OperationsLogTests.cpp"
#include "src/tests/io/routing_tables/operations/SetOperationTests.cpp"
#include "src/tests/io/routing_tables/operations/RemoveOperationTests.cpp"
#include "src/tests/io/routing_tables/operations/DirectionUpdateOperationTests.cpp"
#include "src/tests/io/routing_tables/RoutingTableTests.cpp"
#endif

#ifdef TESTS__TRUSTLINES
#include "src/tests/trust_lines/TrustLineTests.cpp"
#endif

#ifndef INTERNAL_TESTS
int main(int argc, char** argv) {

    return Core(argv[0]).run();

}
#endif