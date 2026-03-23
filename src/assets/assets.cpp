#include "assets.h"

typedef struct {
    VOID*      pool_allocation;
    AssetView* view_table;
    CHAR16*    key_table;
    CHAR16*    char_buffer;
    UINTN      char_buffer_size;
    UINTN      char_buffer_used_size;
    UINTN      asset_count;
} AssetsCtx;

const  CHAR16*   c_file_name  = (CHAR16*)L"assets.txt";
static AssetsCtx s_assets_ctx = {0};

static EFI_STATUS assetToBuffer(
    const CHAR8* key_begin,
    const CHAR8* key_end,
    const CHAR8* text_begin,
    const CHAR8* text_end
) {
    /* text buffer is guranateed to have space for all letters with right formatting */
    CHAR16* key_buffer      = s_assets_ctx.key_table + s_assets_ctx.asset_count * MAX_KEY_LEN;
    CHAR16* key_buffer_end  = s_assets_ctx.key_table + s_assets_ctx.asset_count * MAX_KEY_LEN + MAX_KEY_LEN;
    CHAR16* dst_text_buffer = s_assets_ctx.char_buffer + s_assets_ctx.char_buffer_used_size;
    UINT32  line_len        = 0;
    UINT32  size_x          = 0;
    UINT32  size_y          = 0;
    CHAR16  read_char       = 0;

    if(s_assets_ctx.asset_count == MAX_ASSET_COUNT) {
        return EFI_OUT_OF_RESOURCES;
    }

    for(; key_buffer != key_buffer_end; key_buffer++, key_begin++) {
        *key_buffer = (key_begin < key_end) ? (CHAR16)*(key_begin) : 0;
    }
    for(; text_begin != text_end; text_begin++) {
        read_char = (CHAR16)*text_begin;
        if(read_char == L'\n' || read_char == L'\r') {
            if(line_len != 0) {
                if(size_x == 0) {
                    size_x = line_len;
                }
                if(size_x != line_len) {
                    return EFI_NOT_STARTED;    
                }
                line_len = 0;
                size_y++;
            }
            continue;
        }

        *dst_text_buffer++ = read_char;
        line_len++;
    }

    *dst_text_buffer = 0;
    s_assets_ctx.view_table[s_assets_ctx.asset_count] = (AssetView) {
        .buffer = s_assets_ctx.char_buffer + s_assets_ctx.char_buffer_used_size,
        .size_x = size_x,
        .size_y = size_y
    };
    s_assets_ctx.char_buffer_used_size += size_x * size_y + 1;
    s_assets_ctx.asset_count++;

    return EFI_SUCCESS;
}

static EFI_STATUS parseAssets(
    CHAR8*                        buffer_begin,
    CHAR8*                        buffer_end,
    SIMPLE_TEXT_OUTPUT_INTERFACE* console_out
) {
    CHAR8* key_begin  = NULL;
    CHAR8* key_end    = NULL;
    CHAR8* text_begin = NULL;
    CHAR8* text_end   = NULL;
    CHAR8  input      = 0;
 
    for (;buffer_begin != buffer_end; buffer_begin++) {
        input = *buffer_begin;

        if(key_begin == NULL) {
            if(input == '%') {
                key_begin = buffer_begin + 1;
            }
            continue;
        }
        if(key_end == NULL) {
            if(input == '%') {
                /* end symbol %% */
                if(buffer_begin == buffer_end) {
                    return EFI_SUCCESS;
                }
                key_end = buffer_begin;
            }
            else if(
                (input < '0' || input > '9') &&
                (input < '@' || input > 'Z') &&
                (input < 'a' || input > 'z') &&
                (input != '_')
            ) {
                console_out->OutputString(console_out, (CHAR16*)L"unexpected symbol in asset name\r\n");
                return EFI_NOT_STARTED;
            }
            if((UINTN)(buffer_begin - key_begin) == MAX_KEY_LEN - 1) {
                console_out->OutputString(console_out, (CHAR16*)L"too long asset name\r\n");
            }
            continue;
        }
        if(text_begin == NULL) {
            if(input != '\r' && input != '\n') {
                text_begin = buffer_begin;
            }
            continue;
        }
        if(text_end == NULL) {
            if(input == '%') {
                text_end = buffer_begin;
                if(assetToBuffer(
                    key_begin,
                    key_end,
                    text_begin,
                    text_end
                ) != EFI_SUCCESS) {
                    console_out->OutputString(console_out, (CHAR16*)L"failed to add asset from file\r\n");
                    return EFI_NOT_STARTED;
                }

                key_begin  = buffer_begin + 1;
                key_end    = NULL;
                text_begin = NULL;
                text_end   = NULL;
            }
            continue;
        }
    }
    
    return EFI_SUCCESS;
}

EFI_STATUS loadAssets(
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_protocol,
    EFI_BOOT_SERVICES*               boot_services,
    SIMPLE_TEXT_OUTPUT_INTERFACE*    console_out
) {
    UINT8              file_info_buffer[256 + sizeof(EFI_FILE_INFO)] = {0};
    EFI_GUID           file_guid                                     = EFI_FILE_INFO_ID;
    EFI_FILE_PROTOCOL* root                                          = NULL;
    EFI_FILE_PROTOCOL* file                                          = NULL;
    CHAR8*             char8_buffer                                  = NULL;
    UINTN              char8_buffer_size                             = 0; 
    UINTN              file_info_size                                = sizeof(file_info_buffer);
    EFI_STATUS         efi_status                                    = EFI_SUCCESS;

    if(file_protocol->OpenVolume(
        file_protocol, 
        &root
    ) != EFI_SUCCESS) {
        console_out->OutputString(console_out, (CHAR16*)L"failed to open root volume\r\n");
        goto fail;
    }

    efi_status = root->Open(
        root,
        &file,
        (CHAR16*)c_file_name,
        EFI_FILE_READ_ONLY,
        0
    );
    if(efi_status != EFI_SUCCESS) {
        if(efi_status == EFI_NOT_FOUND) {
            console_out->OutputString(console_out, (CHAR16*)L"failed to find assets file\r\n");
        }
        if(efi_status == EFI_ACCESS_DENIED) {
            console_out->OutputString(console_out, (CHAR16*)L"failed no access to assets file\r\n");
        }
        console_out->OutputString(console_out, (CHAR16*)L"failed to open assets file\r\n");
        goto fail;
    }

    if(file->GetInfo(
        file,
        &file_guid,
        &file_info_size,
        file_info_buffer
    ) != EFI_SUCCESS) {
        console_out->OutputString(console_out, (CHAR16*)L"failed to get assets file info\r\n");
        goto fail;
    }

    char8_buffer_size = ((EFI_FILE_INFO*)file_info_buffer)->FileSize;
    if(char8_buffer_size == 0) {
        console_out->OutputString(console_out, (CHAR16*)L"assets file size is 0\r\n");
        goto success;
    }

    if(boot_services->AllocatePool(
        EfiLoaderData,
        char8_buffer_size,
        (VOID**)&char8_buffer
    ) != EFI_SUCCESS) {
        console_out->OutputString(console_out, (CHAR16*)L"failed to allocate assets pool\r\n");
        goto fail;
    }

    if(file->Read(
        file, 
        &char8_buffer_size,
        char8_buffer
    ) != EFI_SUCCESS) {
        console_out->OutputString(console_out, (CHAR16*)L"failed to read assets file\r\n");
        goto fail;
    }

    file->Close(file);
    root->Close(root);
    file = NULL;
    root = NULL;

    /* allocate persistent buffer */
    if(boot_services->AllocatePool(
        EfiLoaderData,
        char8_buffer_size * 2 + (sizeof(AssetView) + sizeof(CHAR16)) * MAX_KEY_LEN * MAX_ASSET_COUNT,
        (VOID**)&s_assets_ctx.pool_allocation
    ) != EFI_SUCCESS) {
        console_out->OutputString(console_out, (CHAR16*)L"failed to allocate assets buffers");
        goto fail;
    }
    s_assets_ctx.key_table             = (CHAR16*   )((BYTE*)s_assets_ctx.pool_allocation                                                                                       );
    s_assets_ctx.view_table            = (AssetView*)((BYTE*)s_assets_ctx.pool_allocation + sizeof(CHAR16) * MAX_KEY_LEN * MAX_ASSET_COUNT                                      );
    s_assets_ctx.char_buffer           = (CHAR16*   )((BYTE*)s_assets_ctx.pool_allocation + sizeof(CHAR16) * MAX_KEY_LEN * MAX_ASSET_COUNT + sizeof(AssetView) * MAX_ASSET_COUNT);
    s_assets_ctx.char_buffer_size      = char8_buffer_size * 2;
    s_assets_ctx.char_buffer_used_size = 0;
    s_assets_ctx.asset_count             = 0;

    if(parseAssets(
        char8_buffer,
        char8_buffer + char8_buffer_size,
        console_out
    ) != EFI_SUCCESS) {
        console_out->OutputString(console_out, (CHAR16*)L"failed to parse assets\r\n");
        goto fail;
    }

    boot_services->FreePool(char8_buffer);

    success: {
        return EFI_SUCCESS;
    }
    fail: {
        if(s_assets_ctx.pool_allocation) {
            boot_services->FreePool(s_assets_ctx.pool_allocation);
        }
        if(char8_buffer != NULL) {
            boot_services->FreePool(char8_buffer);
        }
        if(file != NULL) {
            file->Close(file);
        }
        if(root != NULL) {
            root->Close(root);
        }

        s_assets_ctx = (AssetsCtx){0};
        return EFI_NOT_STARTED;
    }
}

VOID unloadAssets(
    EFI_BOOT_SERVICES* boot_services
) {
    if(s_assets_ctx.pool_allocation != NULL) {
        boot_services->FreePool(s_assets_ctx.pool_allocation);
    }
    s_assets_ctx = (AssetsCtx){0};
}

VOID printAssets(
    SIMPLE_TEXT_OUTPUT_INTERFACE* console_out
) {
    AssetView* views = s_assets_ctx.view_table;
    CHAR16*    names = s_assets_ctx.key_table;
    UINT32     count = s_assets_ctx.asset_count;
    UINT32     i     = 0;

    for(i = count - 1; i != UINT32_MAX; i--) {
        console_out->OutputString(console_out, (CHAR16*)L"key: ");
        console_out->OutputString(console_out, &names[i * MAX_KEY_LEN]);
        console_out->OutputString(console_out, (CHAR16*)L"\r\n");
        console_out->OutputString(console_out, (CHAR16*)L"text: \r\n");
        console_out->OutputString(console_out, views[i].buffer);
        console_out->OutputString(console_out, (CHAR16*)L"\r\n");
    }
}

const AssetView* assetById(UINTN id) {
    return (id < s_assets_ctx.asset_count) ? s_assets_ctx.view_table + id : NULL;
}
