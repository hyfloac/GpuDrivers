#include "WindowsNtPolyfill.hpp"
#include <Console.hpp>
#include <ConPrinter.hpp>
#include <winternl.h>

#include "GdiThunks.hpp"

int main(int argCount, char* args[])
{
	(void) argCount;
	(void) args;

	// As a DLL create a Console window.
	Console::Create();
	// Initialize the Console window to the correct settings.
	Console::Init();
	(void) InitKernelThunks();

	D3DKMT_OPENADAPTERFROMDEVICENAME openAdapter { };
	openAdapter.pDeviceName = L"\\\\?\\PCI#VEN_10DE&DEV_2482&SUBSYS_37973842&REV_A1#4&2283F625&0&0019#{1CA05180-A699-450A-9A0C-DE4FBE3DDD89}";

	NTSTATUS status = GDIOpenAdapterFromDeviceName(&openAdapter);

	if(!NT_SUCCESS(status))
	{
		ConPrinter::Print(u8"Failed to open adapter {}: 0x{XP0}", openAdapter.pDeviceName, status);
		return 1;
	}

	D3DKMT_CLOSEADAPTER closeAdapter { };
	closeAdapter.hAdapter = openAdapter.hAdapter;

	status = GDICloseAdapter(&closeAdapter);

	D3DKMT_OPENADAPTERFROMLUID openAdapterLuid { };
	openAdapterLuid.AdapterLuid = openAdapter.AdapterLuid;

	status = GDIOpenAdapterFromLuid(&openAdapterLuid);

	if(!NT_SUCCESS(status))
	{
		ConPrinter::Print(u8"Failed to open adapter from LUID: 0x{XP0}", status);
		return 1;
	}

	D3DKMT_VIDMM_ESCAPE vidMMData { };
	vidMMData.Type = D3DKMT_VIDMMESCAPETYPE_GET_VAD_INFO;
	vidMMData.GetVads.Command = D3DKMT_VAD_ESCAPE_GET_SEGMENT_CAPS;
	vidMMData.GetVads.GetSegmentCaps.PhysicalAdapterIndex = 0;

	D3DKMT_ESCAPE escapeQuery { };
	escapeQuery.hAdapter = openAdapterLuid.hAdapter;
	escapeQuery.hDevice = 0;
	escapeQuery.Type = D3DKMT_ESCAPE_VIDMM;
	escapeQuery.Flags.Value = 0;
	escapeQuery.Flags.HardwareAccess = 0;
	escapeQuery.Flags.DeviceStatusQuery = 0;
	escapeQuery.Flags.ChangeFrameLatency = 0;
	escapeQuery.Flags.NoAdapterSynchronization = 0;
	escapeQuery.Flags.VirtualMachineData = 0;
	escapeQuery.Flags.DriverKnownEscape = 1;
	escapeQuery.Flags.DriverCommonEscape = 1;
	escapeQuery.pPrivateDriverData = &vidMMData;
	escapeQuery.PrivateDriverDataSize = sizeof(vidMMData);
	escapeQuery.hContext = 0;

	status = GDIEscape(&escapeQuery);

	ConPrinter::PrintLn(u8"Vad Status: 0x{XP0}", vidMMData.GetVads.Status);

	if(!NT_SUCCESS(status))
	{
		ConPrinter::PrintLn(u8"Failed to get VidMM properties: 0x{XP0}", status);
		return 1;
	}

    return 0;
}