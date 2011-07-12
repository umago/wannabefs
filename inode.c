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

struct inode *wbfs_iget(struct super_block *sb, unsigned long ino)
{
	struct buffer_head *bh;
	struct wbfs_inode *di;
	struct inode *inode;
	int block;

	inode = iget_locked(sb, ino);
	if (!inode)
		return ERR_PTR(-ENOMEM);
	if (!(inode->i_state & I_NEW))
		return inode;

	if (ino < WBFS_ROOT_INO || ino > WBFS_MAXINODES) {
		return ERR_PTR(-EIO);
	}

	block = WBFS_INODE_BLOCK + ino;
	bh = sb_bread(inode->i_sb, block);
	if (!bh) {
		return ERR_PTR(-EIO);
	}

	di = (struct wbfs_inode *)(bh->b_data);
	inode->i_mode = di->i_mode;
	if (di->i_mode & S_IFDIR) {
		inode->i_mode |= S_IFDIR;
		inode->i_op = &wbfs_dir_inode_ops;
		inode->i_fop = &wbfs_dir_operations;
	} else if (di->i_mode & S_IFREG) {
		inode->i_mode |= S_IFREG;
		//inode->i_op = &ux_file_inops;
		//inode->i_fop = &ux_file_operations;
		//inode->i_mapping->a_ops = &ux_aops;
	}
	inode->i_uid = di->i_uid;
	inode->i_gid = di->i_gid;
	inode->i_nlink = di->i_nlink;
	inode->i_size = di->i_size;
	inode->i_blocks = di->i_blocks;
	inode->i_blkbits = WBFS_BLOCKSIZE;
	inode->i_atime.tv_sec = di->i_atime;
	inode->i_mtime.tv_sec = di->i_mtime;
	inode->i_ctime.tv_sec = di->i_ctime;
	memcpy(&inode->i_private, di, sizeof(struct wbfs_inode));

	brelse(bh);

	unlock_new_inode(inode);
	return inode;
}

int wbfs_find_entry(struct inode *dip, char *name)
{
	struct wbfs_inode *uip = (struct wbfs_inode *)&dip->i_private;
	struct super_block *sb = dip->i_sb;
	struct buffer_head *bh = NULL;
	struct wbfs_directory *dirent;
	int i, blk = 0;

	for (blk = 0; blk < uip->i_blocks; blk++) {
		bh = sb_bread(sb, uip->i_addr[blk]);
		dirent = (struct wbfs_directory *)bh->b_data;
		for (i = 0; i < WBFS_DIRS_PER_BLOCK; i++) {
			if (strcmp(dirent->d_name, name) == 0) {
				brelse(bh);
				return dirent->d_ino;
			}
			dirent++;
		}
	}
	if (bh)
		brelse(bh);

	return 0;
}

struct dentry *wbfs_lookup(struct inode *dip, struct dentry *dentry,
			 struct nameidata *nd)
{
        DEBUG("Look up\n");
	struct inode *inode = NULL;
	int inum;

	if (dentry->d_name.len > WBFS_NAMELEN)
		return ERR_PTR(-ENAMETOOLONG);

	inum = wbfs_find_entry(dip, (char *)dentry->d_name.name);
	if (inum) {
		inode = wbfs_iget(dip->i_sb, inum);
		if (!inode)
			return ERR_CAST(inode);
	}
	d_add(dentry, inode);
        DEBUG("FIM Look up\n");
	return NULL;
}

struct inode_operations wbfs_dir_inode_ops = {
            .lookup         = wbfs_lookup,
};
