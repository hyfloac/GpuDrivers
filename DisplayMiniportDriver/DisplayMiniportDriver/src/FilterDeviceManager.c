// #include <ntddk.h>
#include <fltKernel.h>

#include "FilterDeviceManager.h"
#include "Logging.h"

#pragma warning(push)
#pragma warning(disable:4200) /* nonstandard extension used : zero-sized array in struct/union */
#pragma warning(disable:4201) // anonymous unions warning

typedef struct FilterPortStorage
{
    union
    {
        PFLT_PORT ClientPort;
        struct FilterPortStorage* Next;
    };
} FilterPortStorage;

#pragma warning(pop)

static FilterPortStorage FilterPorts[HY_FILTER_PORT_COUNT] = { 0 };
static FilterPortStorage* Head = &FilterPorts[0];

NTSTATUS HyInitFilterDevices(void)
{
    PAGED_CODE();

    LOG_DEBUG("HyInitFilterDevices\n");

    for(int i = 0; i < HY_FILTER_PORT_COUNT - 1; ++i)
    {
        FilterPorts[i].Next = &FilterPorts[i + 1];
    }

    return STATUS_SUCCESS;
}

NTSTATUS HyRegisterFilterClient(PFLT_PORT ClientPort, PVOID* ConnectionPortCookie)
{
    PAGED_CODE();

    LOG_DEBUG("HyRegisterFilterClient\n");

    // Ensure that we can return the handle.
    if(!ConnectionPortCookie)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    // Ensure we have nodes available for storage.
    if(!Head)
    {
        return STATUS_NO_MEMORY;
    }

    // Get the current node.
    FilterPortStorage* const node = Head;
    // Point the head to the next available node.
    Head = Head->Next;

    // Store the port.
    node->ClientPort = ClientPort;
    // Return the handle.
    *ConnectionPortCookie = node;

    return STATUS_SUCCESS;
}

NTSTATUS HyGetFilterClient(PVOID ConnectionPortCookie, PFLT_PORT* ClientPort)
{
    PAGED_CODE();

    LOG_DEBUG("HyGetFilterClient\n");

    // Ensure we have a valid pointer to a port.
    if(!ConnectionPortCookie)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // Ensure that we can return the port.
    if(!ClientPort)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    // Get the node.
    const FilterPortStorage* const node = ConnectionPortCookie;
    // Return the port
    *ClientPort = node->ClientPort;

    return STATUS_SUCCESS;
}

NTSTATUS HyUnregisterFilterClient(PVOID ConnectionPortCookie)
{
    PAGED_CODE();

    LOG_DEBUG("HyUnregisterFilterClient\n");

    // Ensure we have a valid pointer to a port.
    if(!ConnectionPortCookie)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get the node.
    FilterPortStorage* const node = ConnectionPortCookie;
    // Push this node onto the head of the node linked list.
    node->Next = Head;
    Head = node;

    return STATUS_SUCCESS;
}
