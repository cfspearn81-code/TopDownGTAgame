#ifndef DC_LINUX_SAVE_H
#define DC_LINUX_SAVE_H
int linux_disk_init(void);
int linux_disk_lock(void);
int linux_disk_backup_legacy(void);
int linux_disk_backup_upgrade(int stock_patches);
void linux_disk_shutdown(void);
int linux_export_frame_times(const void *bytes,unsigned size);
int disk_read(void *bytes,unsigned size);
int disk_write(const void *bytes,unsigned size);
int disk_create(const void *bytes,unsigned size);
int disk_present(void);
int disk_delete(void);
void disk_slot(int slot);
#endif
