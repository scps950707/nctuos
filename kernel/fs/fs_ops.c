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
	int res = f_mount(obj,fs->path,1);
	return -res;
}

/* Note: Just call f_mkfs at root path '/' */
int fat_mkfs(const char* device_name)
{
	int res = f_mkfs("/",0,0);
	return -res;
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
	{
		mode |= FA_READ;
	}
	if(file->flags&O_WRONLY)
	{
		mode |= FA_WRITE;
	}
	if(file->flags&O_RDWR)
	{
		mode |= (FA_READ|FA_WRITE);
	}
	if((file->flags&O_CREAT)&&(file->flags&O_TRUNC))
	{
		file->size=0;
		mode |= FA_CREATE_ALWAYS;
	}
	else if(file->flags&O_CREAT)
	{
		file->size=0;
		mode|=FA_CREATE_NEW;
	}

	file->pos=0;
	res = f_open((FIL*)file->data,file->path,mode);

	if(file->flags&O_APPEND)
	{
		f_lseek((FIL*)file->data,((FIL*)file->data)->obj.objsize);
		/* printk("((FIL*)file->data)->obj.objsize:%d\n",((FIL*)file->data)->obj.objsize); */
		/* printk("file->size:%d\n",file->size); */
	}
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
	return (res == FR_OK) ? br : -res;
}
int fat_write(struct fs_fd* file, const void* buf, size_t count)
{
	UINT bw;
	int res = f_write((FIL*)file->data,buf,count,&bw);
	file->pos += bw;
	if(file->pos>file->size)
		file->size=file->pos;

	return (res == FR_OK) ? bw : -res;
}
int fat_lseek(struct fs_fd* file, off_t offset)
{
	int res = f_lseek((FIL*)file->data,offset);
	return -res;
}
int fat_unlink(struct fs_fd* file, const char *pathname)
{
	int res = f_unlink(pathname);
	return -res;
}

static void printInfo(FILINFO *fno)
{
	printk("%s ",fno->fname);
	printk("type:");
	if(fno->fattrib&AM_DIR)
		printk("DIR");
	else
		printk("FILE");
	printk(" size:%d\n",fno->fsize);
}

int fat_ls(const char *pathname)
{
	FILINFO fno;
	int res = f_stat(pathname,&fno);
	if(res==FR_OK&&!(fno.fattrib&AM_DIR))
	{
		printInfo(&fno);
		return FR_OK;
	}

	DIR dp;
	res = f_opendir(&dp,pathname);
	if(res!=FR_OK)
		return -res;
	while(true)
	{
		res=f_readdir(&dp,&fno);
		if(res!=FR_OK)
			return -res;
		if(fno.fname[0]==0)
			break;
		else
			printInfo(&fno);
	}
	res = f_closedir(&dp);
	if(res!=FR_OK)
		return -res;

	return FR_OK;
}

int fat_rm(const char *pathname)
{
	int res = f_unlink(pathname);
	return -res;
}

int fat_touch(const char *pathname)
{
	FIL fil;
	int res = f_open(&fil,pathname,FA_OPEN_ALWAYS);
	return -res;
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
    .unlink = fat_unlink,
	.ls = fat_ls,
	.rm = fat_rm,
	.touch = fat_touch
};
