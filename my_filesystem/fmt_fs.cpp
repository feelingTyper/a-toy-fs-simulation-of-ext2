int fmt_fs()//格式化文件系统
{
    memset(super_blk.inode_map, 0, sizeof(super_blk.inode_map));
    memset(super_blk.blk_map, 0, sizeof(super_blk.blk_map));
    super_blk.inode_map[0] = 1;
    super_blk.inode_used = 1;
    super_blk.blk_map[0] = 1;
    super_blk.blk_used = 1;
    cdir.inode_rank = 0;
    cdir.dir_num = 2;
    strcpy(cdir.dir_item[0].filename, ".");
    cdir.dir_item[0].inode_rank = 0;
    strcpy(cdir.dir_item[1].filename, "..");
    cdir.dir_item[1].inode_rank = 0;
    cdir.curr_inode.file_size = cdir.dir_num * sizeof(FCB);
    cdir.curr_inode.file_blk_rank[0] = 0;
    cdir.curr_inode.blk_num = 1;
    strcpy(cdir.curr_inode.owner, username);
    strcpy(cdir.curr_inode.mode, "rwx");
    set_time(cdir.curr_inode.create_time);
    set_time(cdir.curr_inode.modified_time);
    cdir.curr_inode.type = Dictionary;
    rewrite_superblk();
    fseek(Disk, InodeBlkBase, SEEK_SET);
    fwrite(&cdir.curr_inode, sizeof(Inode), 1, Disk);
    fseek(Disk, DataBlkBase, SEEK_SET);
    fwrite(cdir.dir_item, cdir.curr_inode.file_size, 1, Disk);
    strcpy(cdir.curr_path, "~");
    printf("%s-------------------------------------------------------------%s\n",_RED_,_END_);
    printf("%s\tformat filesystem complete!%s\n", _RED_, _END_);
    printf("%s-------------------------------------------------------------%s\n",_RED_,_END_);
    return 0;
}