#include "../cpu/type.h"

struct device_info {
    int vdID;
    int type; // ATA/ATAPI, SATA, etc.
    int ata_id; // for ATA devices
    int sector_size;
};
struct device_info vdevices[64];
int vdID; // virtual device id counter

void storage_driver_init();
int read_sector(struct device_info *device, int sector, uint8_t* buffer);
int write_sector(struct device_info *device, int sector, uint8_t* buffer);

int read_sectors(struct device_info *device, int sector, int count, uint8_t* buffer);
int write_sectors(struct device_info *device, int sector, int count, uint8_t* buffer);
