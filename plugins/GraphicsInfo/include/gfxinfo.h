#include "resource.h"
#include "nvapi.h"

#include <phdk.h>
#include <graph.h>

#define GFXINFO_MENUITEM 1

#define WM_ET_ETWSYS_ACTIVATE (WM_APP + 150)
#define WM_ET_ETWSYS_UPDATE (WM_APP + 151)
#define WM_ET_ETWSYS_PANEL_UPDATE (WM_APP + 160)

VOID ShowDialog(VOID);
VOID NvInit(VOID);
NvPhysicalGpuHandle EnumNvidiaGpuHandles(VOID);
VOID GetNvidiaGpuUsages(VOID);
VOID LogEvent(__in PWSTR str, __in INT status);


PPH_PLUGIN PluginInstance;
PH_CALLBACK_REGISTRATION PluginLoadCallbackRegistration;
PH_CALLBACK_REGISTRATION PluginUnloadCallbackRegistration;
PH_CALLBACK_REGISTRATION PluginMenuItemCallbackRegistration;
PH_CALLBACK_REGISTRATION MainWindowShowingCallbackRegistration;
PH_CALLBACK_REGISTRATION PluginShowOptionsCallbackRegistration;

static PH_LAYOUT_MANAGER WindowLayoutManager;
static PH_CALLBACK_REGISTRATION ProcessesUpdatedRegistration;

VOID NTAPI LoadCallback(
    __in_opt PVOID Parameter,
    __in_opt PVOID Context
    );

VOID NTAPI UnloadCallback(
    __in_opt PVOID Parameter,
    __in_opt PVOID Context
    );

VOID NTAPI MenuItemCallback(
	__in_opt PVOID Parameter,
	__in_opt PVOID Context
	);

VOID NTAPI MainWindowShowingCallback(
	__in_opt PVOID Parameter,
	__in_opt PVOID Context
	);

VOID NTAPI ShowOptionsCallback(
	__in_opt PVOID Parameter,
	__in_opt PVOID Context
	);

INT_PTR CALLBACK MainWndProc(      
	__in HWND hwndDlg,
	__in UINT uMsg,
	__in WPARAM wParam,
	__in LPARAM lParam
	);

INT_PTR CALLBACK EtpEtwSysPanelDlgProc(      
    __in HWND hwndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

#pragma pack(push,8) // Make sure we have consistent structure packings

// macro by dmex
#define NV_SUCCESS(Status) (((NvStatus)(Status)) == NVAPI_OK)

// dmex - magic numbers, do not change them
#define NVAPI_MAX_PHYSICAL_GPUS   64
#define NVAPI_MAX_USAGES_PER_GPU  33 //34
#define MAX_MEMORY_VALUES_PER_GPU 5

#define NV_MEMORY_INFO_VER MAKE_NVAPI_VERSION(NV_MEMORY_INFO_V2, 2)
#define NV_USAGES_INFO_VER MAKE_NVAPI_VERSION(NV_USAGES_INFO_V1, 1)

// rev
typedef struct _NV_MEMORY_INFO
{
    NvU32 Version;
    NvU32 Values[MAX_MEMORY_VALUES_PER_GPU];
} NV_MEMORY_INFO_V2, *PNV_MEMORY_INFO_V2;

// rev
typedef struct _NV_USAGES_INFO
{
    NvU32 Version;
    NvU32 Values[NVAPI_MAX_USAGES_PER_GPU];
} NV_USAGES_INFO_V1, *PNV_USAGES_INFO_V1;

// rev
typedef void* (__cdecl *P_NvAPI_QueryInterface)(UINT);
P_NvAPI_QueryInterface NvAPI_QueryInterface;

// rev
typedef NvStatus (__cdecl *P_NvAPI_GetMemoryInfo)(NvDisplayHandle, PNV_MEMORY_INFO_V2);
P_NvAPI_GetMemoryInfo NvAPI_GetMemoryInfo;

// rev
typedef NvStatus (__cdecl *P_NvAPI_GPU_GetUsages)(NvPhysicalGpuHandle, PNV_USAGES_INFO_V1); //UINT *usages
P_NvAPI_GPU_GetUsages NvAPI_GetUsages;