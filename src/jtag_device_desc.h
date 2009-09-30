#ifndef JTAG_DEVICE_DESC_H__
#define JTAG_DEVICE_DESC_H__

/* The data in this structure will be sent directorly to the jtagice box. */

typedef struct {
    unsigned char cmd;                 // The jtag command to prefix the desc.

    /* The following arrays are bitmaps where each bit is a flag denoting
       wether the register can be read or written. Bit 0 of byte 0 represents
       the IO register at sram addres 0x20, while bit 7 of byte 7 represents
       the register at 0x5f. */

    unsigned char rd[8];               // IO read access.
    unsigned char wr[8];               // IO write access.
    unsigned char sh_rd[8];            // IO shadow read access.
    unsigned char sh_wr[8];            // IO shadow write access.

    /* Same as above, except that first bit is register at sram address 0x60
       and last bit is register at 0xff. */

    unsigned char ext_rd[20];          // Extended IO read access.
    unsigned char ext_wr[20];          // Extended IO write access.
    unsigned char ext_sh_rd[20];       // Extended IO shadow read access.
    unsigned char ext_sh_wr[20];       // Extended IO shadow write access.

    /* Register locations. */

    unsigned char idr_addr;            // IDR address in IO space.
    unsigned char spmcr_addr;          // SPMCR address in SRAM space.
    unsigned char rampz_addr;          // RAMPZ address in IO space.

    /* Memory programming page sizes (in bytes). */

    unsigned char flash_pg_sz[2];      // [0]->little end; [1]->big end
    unsigned char eeprom_pg_sz;

    unsigned char boot_addr[4];        // Boot loader start address.
                                       // This is a WORD address.
                                       // [0]->little end; [3]->big end

    unsigned char last_ext_io_addr;    // Last extended IO location, 0 if no
                                       // extended IO.

    unsigned char eom[2];              // JTAG command terminator.
} jtag1_device_desc_type;

// In appnote AVR067, struct device_descriptor is written with
// int/long field types.  We cannot use them directly, as they were
// neither properly aligned for portability, nor did they care for
// endianess issues.  We thus use arrays of unsigned chars, plus
// conversion macros.

typedef struct {
    unsigned char cmd;                 // The jtag command to prefix the desc.

    unsigned char ucReadIO[8]; //LSB = IOloc 0, MSB = IOloc63
    unsigned char ucReadIOShadow[8]; //LSB = IOloc 0, MSB = IOloc63
    unsigned char ucWriteIO[8]; //LSB = IOloc 0, MSB = IOloc63
    unsigned char ucWriteIOShadow[8]; //LSB = IOloc 0, MSB = IOloc63
    unsigned char ucReadExtIO[52]; //LSB = IOloc 96, MSB = IOloc511
    unsigned char ucReadIOExtShadow[52]; //LSB = IOloc 96, MSB = IOloc511
    unsigned char ucWriteExtIO[52]; //LSB = IOloc 96, MSB = IOloc511
    unsigned char ucWriteIOExtShadow[52];//LSB = IOloc 96, MSB = IOloc511
    unsigned char ucIDRAddress; //IDR address
    unsigned char ucSPMCRAddress; //SPMCR Register address and dW BasePC
    unsigned char ucRAMPZAddress; //RAMPZ Register address in SRAM I/O
				  //space
    unsigned char uiFlashPageSize[2]; //Device Flash Page Size
    unsigned char ucEepromPageSize; //Device Eeprom Page Size in bytes
    unsigned char ulBootAddress[4]; //Device Boot Loader Start Address
    unsigned char uiUpperExtIOLoc[2]; //Topmost (last) extended I/O
				      //location, 0 if no external I/O
    unsigned char ulFlashSize[4]; //Device Flash Size
    unsigned char ucEepromInst[20]; //Instructions for W/R EEPROM
    unsigned char ucFlashInst[3]; //Instructions for W/R FLASH
    unsigned char ucSPHaddr;	// stack pointer high
    unsigned char ucSPLaddr;	// stack pointer low
    // new as of 16-02-2004
    unsigned char uiFlashpages[2]; // number of pages in flash
    unsigned char ucDWDRAddress; // DWDR register address
    unsigned char ucDWBasePC;	// base/mask value of the PC
    // new as of 30-04-2004
    unsigned char ucAllowFullPageBitstream; // FALSE on ALL new
					    //parts
    unsigned char uiStartSmallestBootLoaderSection[2]; //
    // new as of 18-10-2004
    unsigned char EnablePageProgramming; // For JTAG parts only,
					 // default TRUE
    unsigned char ucCacheType;	// CacheType_Normal 0x00,
				// CacheType_CAN 0x01,
				// CacheType_HEIMDALL 0x02
				// new as of 27-10-2004
    unsigned char uiSramStartAddr[2]; // Start of SRAM
    unsigned char ucResetType;	// Selects reset type. ResetNormal = 0x00
				// ResetAT76CXXX = 0x01
    unsigned char ucPCMaskExtended; // For parts with extended PC
    unsigned char ucPCMaskHigh; // PC high mask
    unsigned char ucEindAddress; // Selects reset type. [EIND address...]
    // new as of early 2005, firmware 4.x
    unsigned char EECRAddress[2]; // EECR IO address
} jtag2_device_desc_type;

enum dev_flags {
	DEVFL_NONE         = 0x000000,
	DEVFL_NO_SOFTBP    = 0x000001, // Device cannot use software BPs (no BREAK insn)
	DEVFL_MKII_ONLY    = 0x000002, // Device is only supported in JTAG ICE mkII
};

typedef struct {
    const char* name;
    const unsigned int device_id;      // Part Number from JTAG Device 
                                       // Identification Register
    unsigned int flash_page_size;      // Flash memory page size in bytes
    unsigned int flash_page_count;     // Flash memory page count
    unsigned char eeprom_page_size;    // EEPROM page size in bytes
    unsigned int eeprom_page_count;    // EEPROM page count
    unsigned int vectors_end;	       // End of interrupt vector table
    enum dev_flags device_flags;       // See above.

    gdb_io_reg_def_type *io_reg_defs;

    bool is_xmega;                     // Device is an ATxmega part

    jtag1_device_desc_type dev_desc1;  // Device descriptor to download to
                                       // mkI device
    jtag2_device_desc_type dev_desc2;  // Device descriptor to download to
                                       // mkII device
} jtag_device_def_type;

extern jtag_device_def_type *global_p_device_def, deviceDefinitions[];

#endif /* JTAG_DEVICE_DESC_H__ */
