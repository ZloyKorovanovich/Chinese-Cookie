#include "base.h"
#include "render/render.h"
#include "assets/assets.h"

/* THINK ON YOUR LOOPS with i */

void ___chkstk_ms(void) {
    return;
}

VOID* memset(
    VOID* dst, 
    UINT32 val, 
    UINTN count
) {
    UINT8* dst_bytes     = (UINT8*)dst;
    UINT8* dst_bytes_end = (UINT8*)dst + count;
    for(; dst_bytes != dst_bytes_end; dst_bytes++) {
        *dst_bytes = (CHAR8)val;
    }
    return dst;
}

EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE        image_handle,
    EFI_SYSTEM_TABLE* system_table
) {
    EFI_GUID                         file_protocol_guid  = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_GUID                         image_protocol_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_BOOT_SERVICES*               boot_services       = system_table->BootServices;
    EFI_LOADED_IMAGE_PROTOCOL*       image_protocol      = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_protocol       = NULL;

    Sprite                           sprite_1            = (Sprite){0};
    Sprite                           sprite_2            = (Sprite){0};
    Sprite                           sprite_anim         = (Sprite){0};
    const AssetView*                 asset_view_1        = NULL;
    const AssetView*                 asset_view_2        = NULL;
    const AssetView*                 asset_view_anim[3]  = {0};
    UINTN                            anim_id             = 0;

    if(boot_services->HandleProtocol(
        image_handle,
        &image_protocol_guid,
        (VOID**)&image_protocol
    ) != EFI_SUCCESS) {
        system_table->ConOut->OutputString(system_table->ConOut, L"failed to handle image protocol\r\n");
        goto fail;
    }

    if(boot_services->HandleProtocol(
        image_protocol->DeviceHandle, 
        &file_protocol_guid, 
        (VOID**)&file_protocol
    ) != EFI_SUCCESS) {
        system_table->ConOut->OutputString(system_table->ConOut, L"failed to locate file protocol\r\n");
        goto fail;
    }

    
    if(loadAssets(
        file_protocol, 
        boot_services,
        system_table->ConOut
    ) != EFI_SUCCESS) {
        system_table->ConOut->OutputString(system_table->ConOut, L"failed to locate load assets\r\n");
        goto fail;
    }

    asset_view_1       = assetById(0);
    asset_view_2       = assetById(1);
    asset_view_anim[0] = assetById(2);
    asset_view_anim[1] = assetById(3);
    asset_view_anim[2] = assetById(4);

    sprite_1 = (Sprite) {
        .buffer = asset_view_1->buffer,
        .color  = EFI_BLUE,
        .size_x = asset_view_1->size_x,
        .size_y = asset_view_1->size_y,
        .x = 13,
        .y = 14
    };
    sprite_2 = (Sprite) {
        .buffer = asset_view_2->buffer,
        .color  = EFI_RED,
        .size_x = asset_view_2->size_x,
        .size_y = asset_view_2->size_y,
        .x = 0,
        .y = 0
    };
    sprite_anim = (Sprite) {
        .buffer = NULL,
        .color  = EFI_GREEN,
        .size_x = asset_view_anim[0]->size_x,
        .size_y = asset_view_anim[0]->size_y,
        .x = 12,
        .y = 12
    };

    while(1) {
        clearFrameBuffer(L' ', EFI_BLACK);
        drawSprite(&sprite_1, BM_OVERWRITE);
        drawSprite(&sprite_2, BM_OR);
        drawSprite(&sprite_anim, BM_OR);
        sprite_1.x = (sprite_1.x < -10) ? RESOLUTION_X : sprite_1.x - 1;
        sprite_1.y = (sprite_1.y < -10) ? RESOLUTION_Y : sprite_1.y - 1;
        sprite_2.x = (sprite_2.x > RESOLUTION_X + 10) ? 0 : sprite_2.x + 2;
        sprite_2.y = (sprite_2.y > RESOLUTION_Y + 10) ? 0 : sprite_2.y + 1;
        sprite_anim.buffer = asset_view_anim[(anim_id+=1) % 3]->buffer;
        showFrameBuffer(system_table->ConOut);
        boot_services->Stall(10000);
    }
    unloadAssets(boot_services);

    return EFI_SUCCESS;

    fail: {
        boot_services->Stall(10000000);
        return EFI_NOT_STARTED;
    }
}
