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


extern struct inode_operations wbfs_dir_inode_ops;
extern struct file_operations wbfs_dir_operations;

struct inode *wbfs_iget(struct super_block *, unsigned long);

#define DEBUG(f,a...) \
        do { \
            printk("%d %s: ",__LINE__,__FILE__); \
            printk(f,## a); \
        } while(0)

#define WBFS_MAGIC                0x7778BEBA
#define WBFS_NAMELEN              28
#define WBFS_MAXINODES            32
#define WBFS_MAXBLOCKS            470
#define WBFS_BLOCKSIZE            512
#define WBFS_INODE_BLOCK          8
#define WBFS_FIRST_DATA_BLOCK     50
#define WBFS_ROOT_INO             2
#define WBFS_DIRS_PER_BLOCK       16
#define WBFS_DIRECT_BLOCKS        16
#define WBFS_CLEAN                0
#define WBFS_DIRTY                1

#define WBFS_INODE_FREE           0
#define WBFS_INODE_INUSE          1
#define WBFS_BLOCK_FREE           0
#define WBFS_BLOCK_INUSE          1

struct wbfs_superblock {
        __u32 s_magic;
        __u32 s_version;
        __u32 s_mod;                            // Filesystem is dirty
        __u32 s_ifree;                          // Number of free inodes
        __u32 s_bfree;                          // Number of free blocks
        __u32 s_inode_map[WBFS_MAXINODES];    // List of inodes
        __u32 s_block_map[WBFS_MAXBLOCKS];    // List of blocks
};

struct wbfs_inode {
        __u32 i_mode;
        __u32 i_nlink;
        __u32 i_atime;
        __u32 i_mtime;
        __u32 i_ctime;
        __u32 i_uid;
        __u32 i_gid;
        __u32 i_size;
        __u32 i_blocks;
        __u32 i_addr[WBFS_DIRECT_BLOCKS];
};

struct wbfs_directory {
        __u32 d_ino;
        char d_name[WBFS_NAMELEN];
};

struct wbfs_fs {
        struct wbfs_superblock *u_sb;
        struct buffer_head *u_sbh;
};
