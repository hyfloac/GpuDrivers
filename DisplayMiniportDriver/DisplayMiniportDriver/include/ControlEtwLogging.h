#pragma once

#include <ntddk.h>
#include <dispmprt.h>

void HyControlEtwLogging(IN_BOOLEAN Enable, IN_ULONG Flags, IN_UCHAR Level);
