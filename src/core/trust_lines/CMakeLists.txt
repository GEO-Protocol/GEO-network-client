cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES

        manager/TrustLinesManager.cpp
        manager/TrustLinesManager.h

        audit_rules/BaseAuditRule.h
        audit_rules/BaseAuditRule.cpp
        audit_rules/AuditRuleCountPayments.h
        audit_rules/AuditRuleCountPayments.cpp
        audit_rules/AuditRuleBoundaryOverflowed.h
        audit_rules/AuditRuleBoundaryOverflowed.cpp

        TrustLine.cpp
        TrustLine.h)

add_library(trust_lines ${SOURCE_FILES})
target_link_libraries(trust_lines
        common
        reservations)