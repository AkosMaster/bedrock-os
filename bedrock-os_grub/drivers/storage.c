#include "storage.h"
#include "screen.h"
#include "ATA.h"
#include "../cpu/timer.h"
#include "../libc/mem.h"

int vdID = 0; // virtual device id counter

char ata_buff[512];

void IDE_init() {
    kprint("\\ IDE driver init\n");

    ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);

    int drive_num;
    for (drive_num = 0; drive_num < 4; drive_num++) {
	if (ide_devices[drive_num].Reserved != 0) {
	    int sector_size = ide_devices[drive_num].Type == 0 ? 512 : 2048;
	    vdevices[vdID] = (struct device_info){.vdID=vdID, .type=ide_devices[drive_num].Type, .ata_id=drive_num, .sector_size=sector_size};
	    vdID++;
        }
    }

    kprint("\\ IDE driver setup complete\n");
}

char read_write_buffer[512*8]; // buffer used in read/write operations

int read_sector(struct device_info device, int sector, uint8_t* buffer) {

    uint32_t es = 0;

    uint32_t real_sector = (sector * 512 / device.sector_size);
    uint32_t sector_size_rel = (device.sector_size / 512);
    uint32_t offset = (sector - real_sector * sector_size_rel) * 512;

    if(ide_read_sectors(device.ata_id, 1, real_sector, es, (uint32_t)read_write_buffer) != 0) {return 1;};

    memory_copy(read_write_buffer + offset, buffer, 512);

    return 0;
}

int write_sector(struct device_info device, int sector, uint8_t* buffer) {

    uint32_t es = 0;

    uint32_t real_sector = (sector * 512 / device.sector_size);
    uint32_t sector_size_rel = (device.sector_size / 512);
    uint32_t offset = (sector - real_sector * sector_size_rel) * 512;

    if(ide_read_sectors(device.ata_id, 1, real_sector, es, (uint32_t)read_write_buffer) != 0) { kprint("0. err\n"); return 1;}

    memory_copy(buffer, read_write_buffer + offset, 512);

    if (ide_write_sectors(device.ata_id, 1, real_sector, es, (uint32_t)read_write_buffer) != 0) {return 1;}

    return 0;
}

void test_read_access(struct device_info device) {
    kprint("    read access: ");

    if (read_sector(device, 0, ata_buff) == 0) {
	    kprint("[ OK ]\n");
    }

}

void storage_driver_init() {

    kprint("loading storage drivers\n");

    IDE_init();

    /* print info */
    kprint("\\ ");
    kprint_int(vdID);
    kprint(" recognized storage devices:\n");
    for (int c = 0; c < vdID; c++) {
	kprint("    vdID: ");
	kprint_int(c);
	kprint(" {");

	struct device_info device = vdevices[c];

        kprint(".type=");
	if (device.type == 0) {
	    kprint("ATA");
        } else if (device.type == 1) {
	    kprint("ATAPI");
	} else {
	    kprint("Unknown");
	}

	kprint(", .sector_size=");
	kprint_int(device.sector_size);

        kprint(", .ata_id=");
	kprint_int(device.ata_id);

	kprint("}\n");

	test_read_access(device);
    }

    kprint("\\ storage drivers loaded");
    kprint_at_col(STATUS_OK_MSG, STATUS_COL);

}
