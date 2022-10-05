# FAT32-reader

## Description
compile with `gcc -o fat32-reader.exe fat32-reader.c`

usage: `fat32-reader.exe <image file>`

## TODO
- [ ] auto detect FAT32 partition number
- [ ] make the output clearer
- [ ] read the file content
## FIXME
- [ ] read DBR to get partition info
- [ ] make file name copying more concise
## DONE
- [x] Read files from a specific partition
- [x] Find the cluster number list of a file