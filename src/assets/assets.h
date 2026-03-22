#ifndef _ASSETS_INCLUDED
#define _ASSETS_INCLUDED

#include "../base.h"

#define MAX_KEY_LEN     (32)
#define MAX_ASSET_COUNT (128)

typedef struct {
    CHAR16* buffer;
    UINT32  size_x;
    UINT32  size_y;
} AssetView;

EFI_STATUS loadAssets(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_protocol, EFI_BOOT_SERVICES* boot_services, SIMPLE_TEXT_OUTPUT_INTERFACE* console_out);
VOID unloadAssets(EFI_BOOT_SERVICES* boot_services);
VOID printAssets(SIMPLE_TEXT_OUTPUT_INTERFACE* console_out);

const AssetView* assetById(UINTN id);
const AssetView* assetByName(const CHAR16* name);


#endif
