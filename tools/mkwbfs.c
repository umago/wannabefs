/* Copyright (C) 2011 Lucas Alvares Gomes <lucasagomes@gmail.com>.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <linux/types.h>
#include "../wbfs.h"

int create_root_dir(int fd, struct wbfs_superblock *sb)
{
        struct wbfs_inode inode;
	struct wbfs_directory dir;
	time_t tm;
	char block[WBFS_BLOCKSIZE];

	time(&tm);
	memset((void *)&inode, 0, sizeof(struct wbfs_inode));
	inode.i_mode = S_IFDIR | 0755;
	inode.i_nlink = 2;
	inode.i_atime = tm;
	inode.i_mtime = tm;
	inode.i_ctime = tm;
	inode.i_uid = 0;
	inode.i_gid = 0;
	inode.i_size = WBFS_BLOCKSIZE;
	inode.i_blocks = 1;
	inode.i_addr[0] = WBFS_FIRST_DATA_BLOCK;

	lseek(fd, WBFS_INODE_BLOCK * WBFS_BLOCKSIZE + 1024, SEEK_SET);
	write(fd, (char *)&inode, sizeof(struct wbfs_inode));

	lseek(fd, WBFS_FIRST_DATA_BLOCK * WBFS_BLOCKSIZE, SEEK_SET);
	memset((void *)&block, 0, WBFS_BLOCKSIZE);
	write(fd, block, WBFS_BLOCKSIZE);
	lseek(fd, WBFS_FIRST_DATA_BLOCK * WBFS_BLOCKSIZE, SEEK_SET);
	dir.d_ino = 2;
	strcpy(dir.d_name, ".");
	write(fd, (char *)&dir, sizeof(struct wbfs_directory));
	dir.d_ino = 2;
	strcpy(dir.d_name, "..");
	write(fd, (char *)&dir, sizeof(struct wbfs_directory));

	return 0;
}


int main(int argc, const char *argv[]) {
        struct wbfs_superblock sb;
        int fd, i;

        if (argc != 2) {
                fprintf(stderr, "Need to specify device\n");
                goto error;
        }

        if((fd = open(argv[1], O_WRONLY)) < 0) {
                fprintf(stderr, "Failed to open device\n");
                goto error;
        }

        if ((lseek(fd, (off_t) (WBFS_MAXBLOCKS * WBFS_BLOCKSIZE), SEEK_SET)) == -1) {
                fprintf(stderr, "Device is too small\n");
                goto error;
        }
        lseek(fd, 0, SEEK_SET);

	sb.s_magic = WBFS_MAGIC;
	sb.s_mod = WBFS_CLEAN;
	sb.s_ifree = WBFS_MAXINODES - 3;
	sb.s_bfree = WBFS_MAXBLOCKS - 1;

	sb.s_inode_map[0] = WBFS_INODE_INUSE; // Reserved
	sb.s_inode_map[1] = WBFS_INODE_INUSE; // Reserved
	sb.s_inode_map[2] = WBFS_INODE_INUSE; // Root directory

	for (i = 3; i < WBFS_MAXINODES; i++)
		sb.s_inode_map[i] = WBFS_INODE_FREE;

	sb.s_block_map[0] = WBFS_BLOCK_INUSE; // Superblock

	for (i = 1; i < WBFS_MAXBLOCKS; i++)
		sb.s_block_map[i] = WBFS_BLOCK_FREE;

	write(fd, (char *)&sb, sizeof(struct wbfs_superblock));

        if ((create_root_dir(fd, &sb)) != 0)
                goto error;

        exit(0);

    error:
        fprintf(stderr, "\nCannot create filesystem on this device.\n");
        exit(1);
}
