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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>

#include "wbfs.h"

int wbfs_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
	unsigned long pos;
	struct inode *inode = filp->f_dentry->d_inode;
	struct wbfs_inode *uip = (struct wbfs_inode *)
	    &inode->i_private;
	struct wbfs_directory *udir;
	struct buffer_head *bh;
	__u32 blk;

 start_again:
	pos = filp->f_pos;
	if (pos >= inode->i_size)
		return 0;
	blk = (pos + 1) / WBFS_BLOCKSIZE;
	blk = uip->i_addr[blk];
	bh = sb_bread(inode->i_sb, blk);
	udir = (struct wbfs_directory *)(bh->b_data + pos % WBFS_BLOCKSIZE);

	/*
	 * Skip over 'null' directory entries.
	 */

	if (udir->d_ino == 0) {
		filp->f_pos += sizeof(struct wbfs_directory);
		brelse(bh);
		goto start_again;
	} else {
		filldir(dirent, udir->d_name,
			sizeof(udir->d_name), pos, udir->d_ino, DT_UNKNOWN);
	}
	filp->f_pos += sizeof(struct wbfs_directory);
	brelse(bh);
	return 0;
}


struct file_operations wbfs_dir_operations = {
	.read = generic_read_dir,
	.readdir = wbfs_readdir,
	.fsync = noop_fsync,
};
