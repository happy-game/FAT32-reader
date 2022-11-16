# FAT32-reader

## Description
编译 `gcc -o fat32-reader.exe fat32-reader.c`

用法: `fat32-reader.exe <image file>`

## TODO
- [ ] 自动获取分区对应id
- [ ] 让输出内容清晰一点
- [ ] 读取文件内容
## FIXME
- [ ] 从DBR获取分区信息
- [ ] 优化文件名比对
## DONE
- [x] 从给定分区找到文件
- [x] 找到文件簇链
