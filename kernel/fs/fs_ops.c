/* This file use for NCTU OSDI course */
/* It's contants fat file system operators */

#include <inc/stdio.h>
#include <fs.h>
#include <fat/ff.h>
#include <diskio.h>

extern struct fs_dev fat_fs;

/*TODO: Lab7, fat level file operator.
 *       Implement below functions to support basic file system operators by using the elmfat's API(f_xxx).
 *       Reference: http://elm-chan.org/fsw/ff/00index_e.html (or under doc directory (doc/00index_e.html))
 *
 *  Call flow example:
 *        ┌──────────────┐
 *        │     open     │
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │   sys_open   │  file I/O system call interface
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │  file_open   │  VFS level file API
 *        └──────────────┘
 *               ↓
 *        ╔══════════════╗
 *   ==>  ║   fat_open   ║  fat level file operator
 *        ╚══════════════╝
 *               ↓
 *        ┌──────────────┐
 *        │    f_open    │  FAT File System Module
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │    diskio    │  low level file operator
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │     disk     │  simple ATA disk dirver
 *        └──────────────┘
 */

/* Note: 1. Get FATFS object from fs->data
*        2. Check fs->path parameter then call f_mount.
*/
int fat_mount(struct fs_dev *fs, const void* data)
{
	FATFS *obj = (FATFS*)fs->data;
	if(fs->path[0]=='\0')
	{
		printk("fat_mount fs->path==NULL\n");
	}
	return f_mount(obj,fs->path,1);
}

/* Note: Just call f_mkfs at root path '/' */
int fat_mkfs(const char* device_name)
{
	f_mkfs("/",0,0);
}

/* Note: Convert the POSIX's open flag to elmfat's flag.
*        Example: if file->flags == O_RDONLY then open_mode = FA_READ
*                 if file->flags & O_APPEND then f_seek the file to end after f_open
*/
int fat_open(struct fs_fd* file)
{
	int res=0;
	BYTE mode=0;

	if(file->flags==O_RDONLY)
		mode |= FA_READ;
	if(file->flags&O_WRONLY)
		mode |= FA_WRITE;
	if(file->flags&O_RDWR)
		mode |= (FA_READ|FA_WRITE);
	if((file->flags&O_CREAT)&&(file->flags&O_TRUNC))
		mode |= FA_CREATE_ALWAYS;
	else if(file->flags&O_CREAT)
		mode|=FA_CREATE_NEW;

	res = f_open((FIL*)file->data,file->path,mode);

	if(file->flags&O_APPEND)
		f_lseek((FIL*)file->data,((FIL*)file->data)->obj.objsize);
	return -res;
}

int fat_close(struct fs_fd* file)
{
	int res = f_close(file->data);
	return -res;
}
int fat_read(struct fs_fd* file, void* buf, size_t count)
{
	UINT br;
	int res = f_read((FIL*)file->data,buf,count,&br);
	return -res;
}
int fat_write(struct fs_fd* file, const void* buf, size_t count)
{
}
int fat_lseek(struct fs_fd* file, off_t offset)
{
}
int fat_unlink(struct fs_fd* file, const char *pathname)
{
}

struct fs_ops elmfat_ops = {
    .dev_name = "elmfat",
    .mount = fat_mount,
    .mkfs = fat_mkfs,
    .open = fat_open,
    .close = fat_close,
    .read = fat_read,
    .write = fat_write,
    .lseek = fat_lseek,
    .unlink = fat_unlink
};



