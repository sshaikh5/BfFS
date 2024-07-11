#define _GNU_SOURCE
#define _POSIX_C_SOURCE 1999309L
#define _POSIX_TIMERS
#define _REENTRANT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <zlib.h>
#include <math.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

// Baseline: Folders=100, Files = 10M files
// Midline: Folders=1000, Files = 100M files
// Final: Folders=10000, Files = 1B files

/**
 * variable: only change number of files
*/
//#define TOTAL_FILES 10000000                // 10M
//#define TOTAL_FILES 100000000               // 100M
#define TOTAL_FILES 1000000000              // 1B
#define MAX_FOLDERS 100                     // fixed
#define MAX_FILES_PER_SUB_FOLDER 100000     // fixed
#define SUB_FOLDERS (TOTAL_FILES/MAX_FOLDERS/MAX_FILES_PER_SUB_FOLDER)
#define MAX_FILES_PER_FOLDER (MAX_FILES_PER_SUB_FOLDER*SUB_FOLDERS)

#define BLOCK_SIZE 4096
#define MIN_FILE_SIZE 1*1024
#define AVE_FILE_SIZE 5500
#define MAX_FILE_SIZE 10*1024
#define CRC_SIZE 8
#define FILE_SIZE_HISTOGRAM_BUCKETS 9

// File read/write speed between 1 and 40 usec
#define FILE_SPEED_HISTOGRAM_BUCKETS 20
#define MIN_FILE_RW_SPEED 1     // usec
#define MAX_FILE_RW_SPEED 100   // usec

unsigned long long get_time_in_usec(struct timespec t);
void sub_timespec(struct timespec start, struct timespec stop, struct timespec *delta);
double normal_random(double mean, double stddev, double lower, double upper);
unsigned long get_crc(unsigned char *file_buffer, size_t file_size, unsigned char *crc_buffer);
int get_file_size_histogram_index(unsigned int file_size);
int get_file_speed_histogram_index(unsigned int read_speed);

typedef union {
 unsigned char c[8];
 unsigned long l;
} CRC32;

enum { NS_PER_SECOND = 1000000000 };

typedef struct fs {
    struct statvfs fs;
    unsigned long space_total;
    unsigned long space_used;
    unsigned long space_free;
    unsigned long blocks_total;
    unsigned long blocks_used;
    unsigned long blocks_free;
    unsigned long inodes_total;
    unsigned long inodes_used;
    unsigned long inodes_free;
} FS;

typedef struct statistics
{
    struct timespec run_time;

    // Folder
    unsigned long min_folder_create_time;
    unsigned long max_folder_create_time;
    unsigned long ave_folder_create_time;
    unsigned long total_folder_create_time;
    unsigned long min_folder_read_time;
    unsigned long max_folder_read_time;
    unsigned long ave_folder_read_time;
    unsigned long total_folder_read_time;

    // File
    unsigned long ave_file_create_open_time;
    unsigned long total_file_create_open_time;
    unsigned long ave_file_read_open_time;
    unsigned long total_file_read_open_time;
    
    unsigned long min_file_create_time;
    unsigned long max_file_create_time;
    unsigned long ave_file_create_time;
    unsigned long total_file_create_time;
    
    unsigned long min_file_read_time;
    unsigned long max_file_read_time;
    unsigned long ave_file_read_time;
    unsigned long total_file_read_time;

    unsigned int file_size_histogram_bucket[FILE_SIZE_HISTOGRAM_BUCKETS];
    unsigned int file_write_speed_histogram_bucket[FILE_SPEED_HISTOGRAM_BUCKETS];
    unsigned int file_read_speed_histogram_bucket[FILE_SPEED_HISTOGRAM_BUCKETS];

    unsigned long total_file_create_size;
    unsigned long total_file_create_block;
    unsigned long total_file_read_size;
    unsigned long total_file_read_block;
    
    unsigned int total_incorrect_file_count;
    unsigned int total_crc_failure_count;
    unsigned int total_incorrect_file_size;
    unsigned int total_incorrect_file_block;

    // File system statistics
    FS fs_before;
    FS fs_after;
    FS fs_used;

    struct folder {
        
        unsigned long folder_create_time;   // total folder creation time
        unsigned long folder_read_time;     // total folder read time
        
        unsigned long file_create_open_time;// total files open time during file creation
        unsigned long file_create_time;     // total files creation/write time
        unsigned long file_read_open_time;  // total files open time during file reading
        unsigned long file_read_time;       // total files read time

        unsigned long file_create_size;     // total bytes of the files (create)
        unsigned int  file_create_block;    // total blocks used by the files (create)
        unsigned long file_read_size;       // total bytes of the files (read)
        unsigned int  file_read_block;      // total blocks used by the files (read)

        unsigned int incorrect_file_count;
        unsigned int crc_failure_count;
        unsigned int incorrect_file_size;
        unsigned int incorrect_file_block;

        unsigned long folder_write_throughput;
        unsigned long folder_read_throughput;

    } folder[MAX_FOLDERS];

} STATISTICS;

