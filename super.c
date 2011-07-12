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
#include <linux/slab.h>
#include "wbfs.h"

int wbfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct wbfs_superblock *nfs_sb;
	struct wbfs_fs *fs;
	struct buffer_head *bh;
	struct inode *inode;

	sb_set_blocksize(sb, WBFS_BLOCKSIZE);
	sb->s_blocksize = WBFS_BLOCKSIZE;
	sb->s_blocksize_bits = 9;

	bh = sb_bread(sb, 0);
	if (!bh)
		return -ENOMEM;

	nfs_sb = (struct wbfs_superblock *)bh->b_data;
	if (nfs_sb->s_magic != WBFS_MAGIC) {
                DEBUG("Its not a wannabefs filesystem\n");
		return -EINVAL;
	}

	if (nfs_sb->s_mod == WBFS_DIRTY) {
		DEBUG("Filesystem is not clean\n");
		return -ENOMEM;
	}

	fs = kzalloc(sizeof(struct wbfs_fs), GFP_KERNEL);
	fs->u_sb = nfs_sb;
	fs->u_sbh = bh;
	sb->s_fs_info = fs;

	sb->s_magic = WBFS_MAGIC;
	//sb->s_op = &uxfs_sops;

	inode = wbfs_iget(sb, WBFS_ROOT_INO);
	if (!inode)
		return -ENOMEM;
	sb->s_root = d_alloc_root(inode);
	if (!sb->s_root) {
		iput(inode);
		return -EINVAL;
	}

	if (!(sb->s_flags & MS_RDONLY)) {
		mark_buffer_dirty(bh);
		sb->s_dirt = 1;
	}
	return 0;
}



int wbfs_get_sb(struct file_system_type *fs_type, int flags,
                  const char *dev_name, void *data, struct vfsmount *mnt)
{
	return get_sb_bdev(fs_type, flags, dev_name, data, wbfs_fill_super, mnt);
}

static struct file_system_type wbfs_type = {
	.owner = THIS_MODULE,
	.name = "wannabefs",
	.get_sb = wbfs_get_sb,
	.kill_sb = kill_block_super,
	.fs_flags = FS_REQUIRES_DEV,
};

static int __init wbfs_init(void)
{
        DEBUG("wannabefs init\n");
	return register_filesystem(&wbfs_type);
}

static void __exit wbfs_exit(void)
{
        DEBUG("wannabefs exit\n");
	unregister_filesystem(&wbfs_type);
}

module_init(wbfs_init)
module_exit(wbfs_exit)
