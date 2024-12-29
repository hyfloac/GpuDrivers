#pragma once

#include "Common.h"

NTSTATUS GsCreateAllocation(
    IN_CONST_HANDLE hAdapter, 
    INOUT_PDXGKARG_CREATEALLOCATION pCreateAllocation
);
