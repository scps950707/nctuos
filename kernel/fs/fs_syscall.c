/* This file use for NCTU OSDI course */


// It's handel the file system APIs 
#include <inc/stdio.h>
#include <inc/syscall.h>
#include <fs.h>
#include <kernel/fs/fat/ff.h>

/*TODO: Lab7, file I/O system call interface.*/
/*Note: Here you need handle the file system call from user.
 *       1. When user open a new file, you can use the fd_new() to alloc a file object(struct fs_fd)
 *       2. When user R/W or seek the file, use the fd_get() to get file object.
 *       3. After get file object call file_* functions into VFS level
 *       4. Update the file objet's position or size when user R/W or seek the file.(You can find the useful marco in ff.h)
 *       5. Remember to use fd_put() to put file object back after user R/W, seek or close the file.
 *       6. Handle the error code, for example, if user call open() but no fd slot can be use, sys_open should return -STATUS_ENOSPC.
 *
 *  Call flow example:
 *        ┌──────────────┐
 *        │     open     │
 *        └──────────────┘
 *               ↓
 *        ╔══════════════╗
 *   ==>  ║   sys_open   ║  file I/O system call interface
 *        ╚══════════════╝
 *               ↓
 *        ┌──────────────┐
 *        │  file_open   │  VFS level file API
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │   fat_open   │  fat level file operator
 *        └──────────────┘
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

// Below is POSIX like I/O system call 
int sys_open(const char *file, int flags, int mode)
{
    //We dont care the mode.
/* TODO */
	int fd = fd_new();
	if(fd<0)
		return -STATUS_ENOSPC;
	struct fs_fd *fsfd = fd_get(fd);
	int res = file_open(fsfd, file, flags);
	fd_put(fsfd);
	if(res<0)
	{
		/* printk("res:%d\n",res); */
		switch(res)
		{
			case -FR_NO_FILE:
				return -STATUS_ENOENT;
			case -FR_EXIST:
				return -STATUS_EEXIST;
		}
	}
	else
	{
		return fd;
	}
}

int sys_close(int fd)
{
/* TODO */
	struct fs_fd *fsfd = fd_get(fd);
	if(fsfd==NULL)
		return -STATUS_EINVAL;

	int res = file_close(fsfd);

	/* fsfd */
	fd_put(fsfd);
	/* original */
	fd_put(fsfd);
	return res;
}
int sys_read(int fd, void *buf, size_t len)
{
/* TODO */
	struct fs_fd *fsfd = fd_get(fd);
	if(fsfd==NULL)
		return -STATUS_EBADF;
	if(buf==NULL||(int32_t)len<0)
		return -STATUS_EINVAL;

	int res = file_read(fsfd, buf, len);

	fd_put(fsfd);
	return res;
}
int sys_write(int fd, const void *buf, size_t len)
{
/* TODO */
	struct fs_fd *fsfd = fd_get(fd);
	if(fsfd==NULL)
		return -STATUS_EBADF;
	if(buf==NULL||(int32_t)len<0)
		return -STATUS_EINVAL;

	int res = file_write(fsfd, buf, len);

	fd_put(fsfd);
	return res;
}

/* Note: Check the whence parameter and calcuate the new offset value before do file_seek() */
off_t sys_lseek(int fd, off_t offset, int whence)
{
/* TODO */
	struct fs_fd *fsfd = fd_get(fd);
	if(fsfd==NULL)
		return -STATUS_EBADF;
	if(offset<0)
		return -STATUS_EINVAL;

	int newOffset=0;
	switch (whence) {
	case SEEK_SET:
		newOffset=offset;
		break;
	case SEEK_CUR:
		newOffset = fsfd->pos+offset;
		break;
	case SEEK_END:
		newOffset = fsfd->size+offset;
		break;
	}

	fd_put(fsfd);
	int res = file_lseek(fsfd, newOffset);
	return res;
}

int sys_unlink(const char *pathname)
{
/* TODO */ 
	int res = file_unlink(pathname);
	if(res==-FR_NO_FILE)
		return -STATUS_ENOENT;
	return res;
}

int sys_ls(const char *pathname)
{
	int res = file_ls(pathname);
	if(res==-FR_NO_PATH)
		return -STATUS_ENOENT;
	return res;
}

int sys_rm(const char *pathname)
{
	int res = file_rm(pathname);
	if(res==-FR_NO_FILE)
		return -STATUS_ENOENT;
	return res;
}

int sys_touch(const char *pathname)
{
	return file_touch(pathname);
}
