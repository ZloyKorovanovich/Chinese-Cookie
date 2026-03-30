#ifndef _ASSETS_INCLUDED
#define _ASSETS_INCLUDED

#include "../base.h"

/* maximum length of asset name */
#define MAX_KEY_LEN     (32)
/* maximum nuber of assets loaded */
#define MAX_ASSET_COUNT (128)

/* buffer = pointer to buffer where ascii symbols stored in 16bits values aka words 
   size_x = x resolution of ascii sprite 
   size_y = y resolution of ascii sprite                                            */
typedef struct {
    CHAR16* buffer;
    UINT32  size_x;
    UINT32  size_y;
} AssetView;

/* loads assets from file 
   file_protocol = valid pointer to EFI_SIMPLE_FILE_SYSTEM_PROTOCOL that file will be searched through
   boot_services = valid pointer to EFI_BOOT_SERVICES that are used for stuff like memory allocation 
   console_out   = valid pointer to SIMPLE_TEXT_OUTPUT_INTERFACE used for printing debug info          
   return        = EFI_SUCCESS (success) / other (fail)                                                */
EFI_STATUS loadAssets(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_protocol, EFI_BOOT_SERVICES* boot_services, SIMPLE_TEXT_OUTPUT_INTERFACE* console_out);
/* destroys all assets stored in buffer, frees memory 
   boot_services = valid pointer to EFI_BOOT_SERVICES that are used for stuff like memory allocation */
VOID unloadAssets(EFI_BOOT_SERVICES* boot_services);
/* prints all keys and assets for debug 
   console_out   = valid pointer to SIMPLE_TEXT_OUTPUT_INTERFACE used for printing debug info */
VOID printAssets(SIMPLE_TEXT_OUTPUT_INTERFACE* console_out);

/* returns pointer to asset view struct stored at given id */
const AssetView* assetById(UINTN id);
/* returns pointer to asset view struct on index coresssponding to index of key that is searched (YOU IMPLEMENT THIS) 
   name = valid pointer to CHAR16 zero terminated string of size that is less or equals to MAX_KEY_LEN (including 0 terminator) */
const AssetView* assetByName(const CHAR16* name);


#endif
