#pragma once

#include "Common.h"

NTSTATUS GsCloseAllocation(
    IN_CONST_HANDLE hDevice,
    IN_CONST_PDXGKARG_CLOSEALLOCATION pCloseAllocation
);
