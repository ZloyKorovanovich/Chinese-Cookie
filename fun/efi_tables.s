; efi system table
EFI_SYS_HEADER                   equ 0x00
EFI_SYS_FVENDOR                  equ 0x28
EFI_SYS_FVERSION                 equ 0x30
EFI_SYS_CIN_HANDLE               equ 0x38
EFI_SYS_CIN_INTERFACE            equ 0x40
EFI_SYS_COUT_HANDLE              equ 0x48
EFI_SYS_COUT_INTERFACE           equ 0x50
EFI_SYS_ERR_HANDLE               equ 0x58
EFI_SYS_ERR_INTERFACE            equ 0x60
EFI_SYS_RT_SERVICES              equ 0x68
EFI_SYS_BOOT_SERVICES            equ 0x70
EFI_SYS_TABLE_ENTRIES_COUNT      equ 0x78
EFI_SYS_TABLE_CONGIG_TABLE       equ 0x80

; efi simple text output protocol
EFI_TEXT_OUT_RESET               equ 0x00
EFI_TEXT_OUT_OUTPUT_STRING       equ 0x08
EFI_TEXT_OUT_TEST_STRING         equ 0x10
EFI_TEXT_OUT_QUERY_MODE          equ 0x18
EFI_TEXT_OUT_SET_MODE            equ 0x20
EFI_TEXT_OUT_SET_ATTRIBUTE       equ 0x28
EFI_TEXT_OUT_CLEAR_SCREEN        equ 0x30
EFI_TEXT_OUT_SET_CURSOR_POSITION equ 0x38
EFI_TEXT_OUT_ENABLE_CURSOR       equ 0x40
EFI_TEXT_OUT_MODE                equ 0x48

; efi simple text input protocol
EFI_TEXT_IN_RESET                equ 0x00
EFI_TEXT_IN_READ_KEY_STROKE      equ 0x08
EFI_TEXT_IN_WAIT_FOR_KEY         equ 0x10

EFI_GRAPHICS_QUERY_MODE          equ 0x00
EFI_GRAPHICS_SET_MODE            equ 0x08
EFI_GRAPHICS_BLT                 equ 0x10
EFI_GRAPHICS_MODE                equ 0x18
