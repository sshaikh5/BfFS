# READ ME!

This program creates folders and files to exercise the target file system and capture various metrics for analsis as described in the paper. The goals is to capture a baseline for comparison using EXT4 file system and generate 10 million files of random sizes of normal distribution between 1KB and 10KB before attempting to create 100 million and 1 billion files.

Once the baseline is created, two other runs generate 100 million and 1 billion files.

For consistency between different total sizes, two-level folder hierarchy is created: 100 top level folders and then a computed number of subfolders under each of the top level folder using 100K files per subfolder.

- Total Files = 10 millions, 100 millions, 1 billion
- Total (top-level) Folders = 100
- Files per Subfolder = 100K
- Subfolders = Total Files / Total Folders / Files per Subfolder

## Analysis

### Overall Metrics
- Total Files (**TF**): Total number of files to be generated. The options are 10 million, 100 million and 1 billion files.
- Total Folders: Number of top level folders are fixed at 100.
- Files Per Subfolder: It is fixed at 100K files per subfolder.
- Files Per Folder: It is computed number using total number of files / 100 folders.
- File Size Histogram: It is a 9-bucket histogram to capture file sizes.
- Total Run Time (**TRT**): Timer start as soon as the command is executed until the command returns on the console.
- Number of CRC-32 Failures.
- Number of File Size Mismatch.
- Number of File Blocks Mismatch.
- Disk Space Used (**DSU**).
- File Blocks Used (**Bk**).
- Inode Used (**Inode**).
- Disk Utilization Overheads (**DSUO**).

### Create & Write Metrics
- File Write Speed Histogram: It is 20-bucket histogram to capture the file creating and writing speed metric.
- Total File Create for Write Time (**TFCWT**): Cumulative time to create files, it does not include the time to write the bytes in the file.
- File Create for Write Time_AVE (**FCWT**, usec): Average time to create a file, it does not include the time to write the bytes in the file.
- File Write Time_MIN (**FWT<sub>min</sub>**, usec): Minimum time taken to write the bytes in any file.
- File Write Time_AVE (**FWT<sub>ave</sub>**, usec): Average time taken to write the bytes in any file.
- File Write Time_MAX (**FWT<sub>max</sub>**, usec): Maximum time taken to write the bytes in any file. Please note that these usually are outliers.
- Write Throughput (**WTh**, bytes/usec): It shows how fast the bytes are written (total number of bytes / time take to write them).
- Total Folder Write Time (**TFWT**): This is the time to create folders and subfolders where the files are going to be generated. It is typically a very small duration in comparison with file creation and writing.
- Total Files Write Time (**TfWT**): This is the time to create and write file of Gaussian distributed size with random data along with CRC-32 check.
- Total Write Time (**TWT**): This is the cumulative time where the application is creating folders, subfolders, and files during the entire run.
- Files Written Per Second (**FWs**): This a computed metrics to show the writing performance using total number of files generated / total file creation time.
- Blocks Written (**Bk**): 4K blocks used during files creation.
- Blocks Written Per Second (**BkWs**): 4K blocks writing performance.
- Total Bytes Written (**TBW**): Total number of bytes written by accumulating random generated file sizes during the run.
- Folders Write Throughput (**FWTh**, bytes/usec): It is 20-bucket histogram to capture peformance of file creation from every 5th folders from 100 top level folders (and subfolders underneath) to show read performance trend.

### Open & Read Metrics
- File Read Speed Histogram: It is 20-bucket histogram to capture the file opening and read speed metric. It is typically more than writing speed since page cache is not involved and all the "new" bytes need to be read to calculate the CRC-32 checksum for verification.
- Total File Open for Read Time (**TFORT**): Cumulative time to open files, it does not include the time to read the bytes in the file.
- File Open for Read Time_AVE (**FORT**, usec): Average time to open a file, it does not include the time to read the bytes in the file.
- File Read Time_MIN (**FRT**, usec): Minimum time taken to read the bytes in any file.
- File Read Time_AVE (**FRT**, usec): Average time taken to read the bytes in any file.
- File Read Time_MAX (**FRT**, usec): Maximum time taken to read the bytes in any file. Please note that these usually are outliers.
- Read Throughput (**RTh**, bytes/usec): It shows how fast the bytes are read (total number of bytes / time take to read them).
- Total Folder Read Time (**TFRT**): This is the time to open folders and subfolders where the files are going to be read from. It is typically a very small duration in comparison with file openning and reading.
- Total Files Read Time (**TfRT**): This is the time to open and read file and generate CRC-32 check to compare with CRC-32 bytes in the file.
- Total Read Time (**TRT**): This is the cumulative time where the application is searching folders, subfolders, and opening files during the entire run.
- Files Read Per Second (**FRs**): This a computed metrics to show the reading performance using total number of files generated / total file reading time.
- Blocks Read (**Bk**): 4K blocks determined during files read.
- Blocks Read Per Second (**BkRs**): 4K blocks reading performance.
- Total Bytes Read (**TBR**): Total number of bytes read by accumulating file sizes during the run.
- Folders Read Throughput (**FRTh**, bytes/usec): It is 20-bucket histogram to capture peformance of files read from every 5th folders from 100 top level folders (and subfolders underneath) to show read performance trend.

### Formulas
- Files:    TRT     = TFCWT     + TWT   + TFORT     + TRT   + Processing Overheads => %age

**Examples**
- 100M:     14560   = 2425      + 2872  + 1351      + 6285  + CPU-PO => 14560   - 12933 = 1627  = 11%
- 10M:      571     = 224       + 124   + 23        + 53    + CPU-PO => 571     - 424   = 147   = 25%

## Code Compilation
gcc main.c subs.c -o main -lz -lm

## Code Execution
./main

## Console logs
All times captured are in microseconds. During creation following metrics are captured and displayed on the console for each folder:
- Cumulative sizes of the files in that folder/subfolders
- Cumulative 4K blocks occupied by the files in that folder/subfolders
- Cumulative time to create the files in the folder/subfolders
```sh
[Date & Time] Create Folder: <folder no> ......[Size,Blocks,Open,Write,Folder Create]
[2024-06-13 15:13:57] Create folder: 6 ........[5500004864,1918134,21447529,21,11757493,11,326,467]
[Date & Time] Read Folder: <folder no> ........[Size,Blocks,Open,Read,Folder Read]
[2024-06-13 17:02:58] Read folder: 6 ..........[5500004864,1918134,13819820,13,59770979,59,467,92]
```

## Environment Setup

To exercise the file system using above paramters, an additional 14TB hard disk is used and installed on a an Ubuntu 22.04 machine with no OS or file system level customization. The machine used for these experiments is 32-core 2x E5-2630 Xeon processors, with 256GB RAM, 1TB SSD for OS, 2TB SSD for applications, and a 14TB hard disk.

Following file systems are exercised.

### EXT4
Since the goal is to generate 1 billion files and additional folders to hold these files, we need 1 billion plus inodes therefore 1.2 billion inodes are created. Following commands are used to create and configure EXT4 file system with required inodes. A 14TB hard disk is provided as /dev/**sda**. For consistency, blocks of 4KB are used:

- sudo umount Data*
- sudo mkfs.ext4 -N 1200005248 -b 4096 /dev/**sda**
```sh
   mke2fs 1.46.5 (30-Dec-2021)
   /dev/sda contains a ext4 file system
       last mounted on /home/sohail/Data on Tue Jun 11 16:52:12 2024
   Proceed anyway? (y,N) y
   Creating filesystem with 3418095616 4k blocks and 1200005248 inodes
   Filesystem UUID: b4c04b87-3c00-41b8-9cf3-7c234e74c8c1
   Superblock backups stored on blocks: 
        32768, 98304, 163840, 229376, 294912, 819200, 884736, 1605632, 2654208, 
        4096000, 7962624, 11239424, 20480000, 23887872, 71663616, 78675968, 
        102400000, 214990848, 512000000, 550731776, 644972544, 1934917632, 
        2560000000
```
- mkdir Data*
- sudo mount -t ext4 /dev/sda Data
- sudo chmod 775 Data
- sudo chown -R sohail:sohail Data
- mkdir -p /home/sohail/Data/bfs/test1
  - This folder needs to be create as it is used by the program. It can be changed but that requires changing the path in the code and recompilation. 
- df -i
  - /dev/sda    1200005248  11  1200005237  1% /home/sohail/Data
- df -B4k
  - /dev/sda    3342575344  9   3171666459  1% /home/sohail/Data
- df -h
  - /dev/sda    13T         28K 12T         1% /home/sohail/Data
- df -h -B 1
  - /dev/sda    13691188609024  36864 12991145816064  1% /home/sohail/Data

[*] Optional, in case of repeating the steps during re-runs

### XFS
XFS is a high-performance 64-bit journaling file system created by Silicon Graphics, Inc (SGI) in 1993. Created with high-capacity devices in mind, XFS is best known for its high performance when dealing with massive amounts of data. This file system is typically found on servers, storage arrays, and less frequently – on consumer PCs. Its popularity has also increased with the widespread usage of NAS boxes, a big share of which are supplied with XFS from the manufacturer, including such brands as Buffalo LinkStation and TeraStation, NetGear, LaCie, Iomega and others (https://www.ufsexplorer.com/articles/storage-technologies/xfs-file-system/).

#### Install XFS Support
- sudo apt-get install xfsprogs
- sudo modprobe -v xfs
#### Create File System
- sudo umount Data*
- sudo mkfs.xfs -i maxpct=10 -f /dev/sda
    ```sh
    meta-data=/dev/sda              isize=512    agcount=13, agsize=268435455 blks
             =                      sectsz=4096  attr=2, projid32bit=1
             =                      crc=1        finobt=1, sparse=1, rmapbt=0
             =                      reflink=1    bigtime=0 inobtcount=0
    data     =                      bsize=4096   blocks=3418095616, imaxpct=10
             =                      sunit=0      swidth=0 blks
    naming   =version 2             bsize=4096   ascii-ci=0, ftype=1
    log      =internal log          bsize=4096   blocks=521728, version=2
             =                      sectsz=4096  sunit=1 blks, lazy-count=1
    realtime =none                  extsz=4096   blocks=0, rtextents=0
    ```
- mkdir Data*
- sudo mount -t xfs /dev/sda Data
- sudo chmod 775 Data
- sudo chown -R sohail:sohail Data
- mkdir -p /home/sohail/Data/bfs/test1
- df -i
  - /dev/sda        2734476480  5   2734476475    1% /home/sohail/Data
- df -h
  - /dev/sda        13T         91G     13T   1% /home/sohail/Data

[*] Optional, in case of repeating the steps during re-runs

### BtrFS
Btrfs is a computer storage format that combines a file system based on the copy-on-write (COW) principle with a logical volume manager. Btrfs is a modern copy-on-write file system for Linux with advanced features for fault tolerance, repair and easy administration. Btrfs uses copy-on-write for all files, which means if a file is modified/written to the storage, the file is not replaced but a copy of the file is created. This mechanism helps to create snapshots with minimum size as the unmodified files need not be copied for creating the snapshot.

#### Install BtrFS Support
- sudo apt install btrfs-progs
#### Create File System
- sudo umount Data*
- sudo mkfs.btrfs -f /dev/sda
  ```sh
    btrfs-progs v5.16.2
    See http://btrfs.wiki.kernel.org for more information.

    NOTE: several default settings have changed in version 5.15, please make sure
        this does not affect your deployments:
        - DUP for metadata (-m dup)
        - enabled no-holes (-O no-holes)
        - enabled free-space-tree (-R free-space-tree)

    Label:              (null)
    UUID:               479c36d6-93e3-41b8-8c0e-a0a0460115fd
    Node size:          16384
    Sector size:        4096
    Filesystem size:    12.73TiB
    Block group profiles:
    Data:             single            8.00MiB
    Metadata:         DUP               1.00GiB
    System:           DUP               8.00MiB
    SSD detected:       no
    Zoned device:       no
    Incompat features:  extref, skinny-metadata, no-holes
    Runtime features:   free-space-tree
    Checksum:           crc32c
    Number of devices:  1
    Devices:
    ID        SIZE  PATH
        1    12.73TiB  /dev/sda
  ```
- sudo mount -t btrfs -o rw,noatime /dev/sda Data
- sudo mount -t btrfs -o rw,noatime,compress=zstd:1 /dev/sda Data
- sudo chmod 775 Data
- sudo chown -R sohail:sohail Data
- mkdir -p /home/sohail/Data/bfs/test1
- df -i
  ```sh
    Filesystem        Inodes  IUsed     IFree IUse% Mounted on
    tmpfs           33001800   1351  33000449    1% /run
    /dev/sdb2       61022208 601552  60420656    1% /
    tmpfs           33001800   1174  33000626    1% /dev/shm
    tmpfs           33001800      4  33001796    1% /run/lock
    efivarfs               0      0         0     - /sys/firmware/efi/efivars
    /dev/sdb1              0      0         0     - /boot/efi
    tmpfs            6600360    166   6600194    1% /run/user/1000
    /dev/sdc       122101760  17020 122084740    1% /home/sohail/Projects
    /dev/sda               0      0         0     - /home/sohail/Data
  ```
- df -h
  ```sh
    Filesystem      Size  Used Avail Use% Mounted on
    tmpfs            26G  2.6M   26G   1% /run
    /dev/sdb2       916G   57G  813G   7% /
    tmpfs           126G  302M  126G   1% /dev/shm
    tmpfs           5.0M  4.0K  5.0M   1% /run/lock
    efivarfs        120K   97K   19K  85% /sys/firmware/efi/efivars
    /dev/sdb1       511M  6.1M  505M   2% /boot/efi
    tmpfs            26G  136K   26G   1% /run/user/1000
    /dev/sdc        1.8T   26G  1.7T   2% /home/sohail/Projects
    /dev/sda         13T  5.8M   13T   1% /home/sohail/Data
  ```

### F2FS

#### Install F2FS Support
- sudo modprobe f2fs
- sudo apt install f2fs-tools
#### Create File System
- sudo umount Data*
- sudo mkfs.f2fs -i -s 10 -z 10 -f /dev/sda
- sudo mkfs.f2fs -i -f /dev/sda
  ```sh
        F2FS-tools: mkfs.f2fs Ver: 1.15.0 (2022-05-13)

    Info: Disable heap-based policy
    Info: Debug level = 0
    Info: Trim is enabled
    Info: [/dev/sda] Disk Model: ST14000VN0008-2Y
      /dev/sda appears to contain an existing filesystem (f2fs).
    Info: Segments per section = 1
    Info: Sections per zone = 1
    Info: sector size = 512
    Info: total sectors = 27344764928 (13351936 MB)
    Info: zone aligned segment0 blkaddr: 512
    Info: format version with
      "Linux version 6.5.0-35-generic (buildd@lcy02-amd64-079) (x86_64-linux-gnu-gcc-12 (Ubuntu 12.3.0-1ubuntu1~22.04) 12.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #35~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Tue May  7 09:00:52 UTC 2"
    Info: [/dev/sda] Discarding device
    Info: This device doesn't support BLKSECDISCARD
    Info: This device doesn't support BLKDISCARD
    Info: Overprovision ratio = 0.060%
    Info: Overprovision segments = 7332 (GC reserved = 3341)
  ```
- sudo mount -t f2fs /dev/sda Data
- sudo chmod 775 Data
- sudo chown -R sohail:sohail Data
- mkdir -p /home/sohail/Data/bfs/test1
- df -i
  ```sh
    Filesystem        Inodes  IUsed     IFree IUse% Mounted on
    tmpfs           33001801   1337  33000464    1% /run
    /dev/sdb2       61022208 589708  60432500    1% /
    tmpfs           33001801    250  33001551    1% /dev/shm
    tmpfs           33001801      4  33001797    1% /run/lock
    efivarfs               0      0         0     - /sys/firmware/efi/efivars
    /dev/sdb1              0      0         0     - /boot/efi
    tmpfs            6600360    157   6600203    1% /run/user/1000
    /dev/sdc       122101760  17020 122084740    1% /home/sohail/Projects
    /dev/sda       683504637      3 683504634    1% /home/sohail/Data
  ```

### ZFS

#### Install ZFS Support
- sudo apt install zfsutils-linux
#### Create File System
- sudo umount Data*
- sudo zpool create -f zfs-pool /dev/sda
- sudo zpool status
- sudo zfs set mountpoint=~/Data zfs-pool
- sudo chmod 775 Data
- sudo chown -R sohail:sohail Data
- mkdir -p /home/sohail/Data/bfs/test1
- [AFTER EXERCISE REMOVE THE POOL]
- sudo zpool destroy zfs-pool
- df -i
  - zfs-pool       27044871168       8     27044871160   1% /home/sohail/Data
- df -B128K
  - zfs-pool         105644028       1       105644027   1% /home/sohail/Data
- df -h
  - zfs-pool               13T    128K             13T   1% /home/sohail/Data
- df -h -B 1
  - zfs-pool    13846974038016  131072  13846973906944   1% /home/sohail/Data

### JFS





