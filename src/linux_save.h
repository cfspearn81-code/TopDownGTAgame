#ifndef DC_LINUX_SAVE_H
#define DC_LINUX_SAVE_H
int linux_disk_init(void);
void linux_disk_shutdown(void);
int disk_read(void *bytes,unsigned size);
int disk_write(const void *bytes,unsigned size);
int disk_create(const void *bytes,unsigned size);
int disk_present(void);
int disk_delete(void);
void disk_slot(int slot);
#endif
