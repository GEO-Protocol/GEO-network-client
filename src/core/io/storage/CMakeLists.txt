cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        StorageHandler.h
        StorageHandler.cpp
        RoutingTableHandler.h
        RoutingTableHandler.cpp
        RoutingTablesHandler.h
        RoutingTablesHandler.cpp
        TrustLineHandler.h
        TrustLineHandler.cpp
        PaymentOperationStateHandler.h
        PaymentOperationStateHandler.cpp
        TransactionsHandler.h
        TransactionsHandler.cpp
        HistoryStorage.h
        HistoryStorage.cpp
        record/base/Record.h
        record/base/Record.cpp
        record/trust_line/TrustLineRecord.h
        record/trust_line/TrustLineRecord.cpp
        record/payment/PaymentRecord.h
        record/payment/PaymentRecord.cpp

        IOTransaction.cpp
        IOTransaction.h

        #
        # Migrations
        #
        MigrationsHandler.h
        MigrationsHandler.cpp

        migrations/AbstractMigration.h
        migrations/AbstractMigration.cpp

        migrations/AmountAndBalanceSerialization.h
        migrations/AmountAndBalanceSerialization.cpp

        migrations/SolomonHistoryMigration.h
        migrations/SolomonHistoryMigration.cpp

        migrations/SolomonHistoryMigrationTwo.h
        migrations/SolomonHistoryMigrationTwo.cpp
        migrations/SolomonHistoryMigration.cpp

        migrations/UniqueIndexHistoryMigration.h
        migrations/UniqueIndexHistoryMigration.cpp

        migrations/MaxDemianMigration.h
        migrations/MaxDemianMigration.cpp)

add_library(io__storage ${SOURCE_FILES})

target_link_libraries(io__storage
        libsqlite3)
