cmake_minimum_required(VERSION 3.6)

find_package(Boost COMPONENTS system filesystem REQUIRED)

set(SOURCE_FILES
        internal/AbstractRoutingTable.cpp
        internal/AbstractRoutingTable.h
        internal/transactions/internal/DirectionUpdateOperation.cpp
        internal/transactions/internal/DirectionUpdateOperation.h
        internal/transactions/internal/Operation.cpp
        internal/transactions/internal/Operation.h
        internal/transactions/internal/RemoveOperation.cpp
        internal/transactions/internal/RemoveOperation.h
        internal/transactions/internal/RollbackSetOperation.cpp
        internal/transactions/internal/RollbackSetOperation.h
        internal/transactions/internal/SetOperation.cpp
        internal/transactions/internal/SetOperation.h
        internal/transactions/OperationsLog.cpp
        internal/transactions/OperationsLog.h
        internal/transactions/Transaction.cpp
        internal/transactions/Transaction.h
        SecondLevelRoutingTable.cpp
        SecondLevelRoutingTable.h
        ThirdLevelRoutingTable.cpp
        ThirdLevelRoutingTable.h internal/transactions/internal/RollbackRemoveOperation.cpp internal/transactions/internal/RollbackRemoveOperation.h internal/transactions/internal/RollbackDirectionUpdateOperation.cpp internal/transactions/internal/RollbackDirectionUpdateOperation.h)

add_library(io_routing_tables ${SOURCE_FILES})
target_link_libraries(io_routing_tables
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}

        common
        db)