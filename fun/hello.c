#include <efi.h>
#include <efilib.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL* findGraphicsProtocol(
    EFI_BOOT_SERVICES* boot_services
) {
    EFI_HANDLE*                   handle_buffer                           = NULL;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* graphics                                = NULL;
    EFI_GUID                      graphics_guid                           = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    UINTN                         handle_count                            = 0;
    UINTN                         i                                       = 0;

    if(boot_services->LocateHandleBuffer(
        ByProtocol,
        &graphics_guid,
        NULL,
        &handle_count,
        &handle_buffer
    ) != EFI_SUCCESS) {
        Print(L"failed to locate graphics handles buffer\n");
        return NULL;
    }

    for(i = 0; i != handle_count; i++) {
        Print(L"checking graphics protocol id: %llu\n", i);

        if(boot_services->HandleProtocol(
            handle_buffer[i],
            &graphics_guid,
            (void **)&graphics
        ) != EFI_SUCCESS) {
            Print(L"failed to handle protocol\n");
            continue;
        }

        if(graphics->Mode == NULL) {
            Print(L"mode is NULL\n");
            continue;
        }

        if((void *)graphics->Mode->FrameBufferBase == NULL) {
            Print(L"FrameBufferBase is NULL\n");
            continue;
        }

        if(graphics->Mode->FrameBufferSize == 0) {
            Print(L"FrameBufferSize is 0\n");
            continue;
        }

        if(graphics->Mode->Info->VerticalResolution == 0 || graphics->Mode->Info->HorizontalResolution == 0) {
            Print(L"VerticalResolution or HorizontalResolution is 0\n");
            continue;
        }

        if(graphics->Mode->Info->PixelsPerScanLine == 0) {
            Print(L"PixelsPerScanLine is 0\n");
            continue;
        }

        if(
            graphics->Mode->Info->PixelFormat != PixelRedGreenBlueReserved8BitPerColor && 
            graphics->Mode->Info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor
        ) {
            Print(L"PixelFormat is bad\n");
            continue;
        }

        Print(L"selected protocol %llu\n", i);
        return graphics;
    }

    return NULL;
}

EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE*       image_handle,
    EFI_SYSTEM_TABLE* system_table
) {
    EFI_BOOT_SERVICES*             boot_services = system_table->BootServices;
    SIMPLE_TEXT_OUTPUT_INTERFACE*  cout          = system_table->ConOut;
    EFI_GRAPHICS_OUTPUT_PROTOCOL*  graphics      = NULL;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* frame_buffer  = NULL;
    UINT32                         res_x         = 0;
    UINT32                         res_y         = 0;
    UINT32                         x             = 0;
    UINT32                         y             = 0;

    InitializeLib(image_handle, system_table);

    graphics = findGraphicsProtocol(boot_services);
    if(graphics == NULL) {
        Print(L"failed to find suitable graphics protocol\n");
        goto fail;
    }

    res_x = graphics->Mode->Info->HorizontalResolution;
    res_y = graphics->Mode->Info->VerticalResolution;
    Print(L"resolution %u x %u\n", res_x, res_y);

    frame_buffer = AllocatePool((UINTN)res_x * (UINTN)res_y * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    if(frame_buffer == NULL) {
        Print(L"failed to allocate pool\n");
        goto fail;
    }

    for(y = 0; y != res_y; y++) {
        for(x = 0; x != res_x; x++) {
            frame_buffer[x + y * res_x] = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) {
                .Blue = x * 0xFF / res_x,
                .Green = y * 0xFF / res_y,
                .Red = 0x00
            };
        }
    }

    system_table->ConOut->EnableCursor(system_table->ConOut, FALSE);
    system_table->ConOut->ClearScreen(system_table->ConOut);

    if(graphics->Blt(
        graphics,
        frame_buffer,
        EfiBltBufferToVideo,
        0, 0,
        0, 0,
        res_x, res_y,
        0
    ) != EFI_SUCCESS) {
        Print(L"blt failed\n");
        goto fail;
    }

    return EFI_SUCCESS;

    fail: {
        if(frame_buffer != NULL) {
            FreePool(frame_buffer);
        }
        return EFI_NOT_STARTED;
    }
}
