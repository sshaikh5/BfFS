#include "BfFS.h"

/**
 * main program
 * author: Sohail Shaikh (mshaikh5@gmu.edu)
 * date: May 22, 2024
 * 
 * gcc main.c subs.c -o main -lz -lm
    
    sudo umount Data
    sudo mkfs.ext4 -n -N 2001121408 -b 4096 /dev/sda		# dry run -n
    sudo mkfs.ext4 -N 2001121408 -b 4096 /dev/sda		    # 1.2 billion inodes
    mkdir Data
    sudo mount -t ext4 /dev/sda Data				        # data storage
    sudo chmod 775 Data
    sudo chown -R sohail:sohail Data
    df -i
        /dev/sda       2001121408     11 2001121397    1% /home/sohail/Data

    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./main

*/
int main(int argc, char *argv[]) {

    int progress_bar_div = MAX_FILES_PER_FOLDER/100;
    char folder_path[128];
    char folder_name[16];
    char file_path[128];
    char file_name[16];
    unsigned char random_data[16*1024];   // 16KB, random data buffer
    unsigned char file_buffer[MAX_FILE_SIZE+CRC_SIZE];
    int rv;
    size_t file_size;
    const char* folder_storage = "/home/sohail/Data/bfs/test1/";
    bool writer = true;
    bool reader = true;
    char fs_size[6] = "10M";        // acceptable values: 10M, 100M, 1B
    char root_folder[128];
    char sub_folder[128];
    time_t t;
    struct tm tm;
    struct statvfs fs_temp;

    /**
     * Initialization
    */
    STATISTICS *statistics = calloc(1, sizeof(STATISTICS));
    srand((unsigned int)time(NULL));    
    FILE *fptr = fopen("random_data_16kb.bin","rb");    // r for read, b for binary
    fread(random_data, sizeof(random_data)-1, 1, fptr); // read all bytes
    fclose(fptr);
    printf("Statistics Size (bytes): %ld\n", sizeof(STATISTICS));
    printf("Total files: %d\n", TOTAL_FILES);
    printf("Folders: %d\n", MAX_FOLDERS);
    printf("Max files per sub-folder: %d\n", MAX_FILES_PER_SUB_FOLDER);
    printf("Subfolders per folder: %d\n", SUB_FOLDERS);
    printf("Max files per folder: %d\n", MAX_FILES_PER_FOLDER);    
    
    statistics->min_folder_create_time = ULONG_MAX;
    statistics->min_folder_read_time = ULONG_MAX;
    statistics->min_file_create_time = ULONG_MAX;
    statistics->min_file_read_time = ULONG_MAX;
    
    /**
     * Run time collection
    */
    struct timespec run_time_start, run_time_stop;
    clock_gettime(CLOCK_REALTIME, &run_time_start);  // Start the timer to collect run time!

    // Get filesystem information
    rv = statvfs(folder_storage, &statistics->fs_before.fs);
    if(rv == -1) {
        printf("Unable to read filesystem information. Exiting program\n");
        free(statistics);
        return -1;
    }

    // rv = statvfs(folder_storage, &fs_temp);
    // printf("Space: Total=%lu, Available=%lu, Used=%lu\n", 
    // fs_temp.f_blocks*fs_temp.f_bsize,   // total size
    // fs_temp.f_bavail*fs_temp.f_bsize,   // avaiable
    // fs_temp.f_blocks*fs_temp.f_bsize-fs_temp.f_bfree*fs_temp.f_bsize);  // Used
    // printf("Blocks: Total=%lu, Available=%lu, Used=%lu\n", 
    // fs_temp.f_blocks, fs_temp.f_bavail, fs_temp.f_blocks-fs_temp.f_bavail);
    // printf("Inodes: Total=%lu, Available=%lu, used=%lu\n", 
    // fs_temp.f_files, fs_temp.f_favail, fs_temp.f_files-fs_temp.f_favail);
    //return 0;

    /**
     * Create Folders and Files
    */
    if (writer == true) {
        struct timespec folder_create_start_time, folder_create_stop_time, folder_create_time, 
                        file_create_open_start_time, file_create_open_stop_time, file_create_open_time,
                        file_create_start_time, file_create_stop_time, file_create_time;

        /**
         * Create folders
        */
        statistics->total_folder_create_time = 0;
        printf("[Date & Time] Create Folder: - ....................................................................................................[Size,Blocks,Open,Write,Folder Create]\n");
        for (int folder_id = 0; folder_id < MAX_FOLDERS; folder_id++) {
            
            t = time(NULL);
            tm = *localtime(&t);
            
            statistics->folder[folder_id].folder_create_time = 0;
            strcpy(folder_path, folder_storage);
            sprintf(root_folder, "%d", folder_id);
            strcat(folder_path, root_folder);
            clock_gettime(CLOCK_REALTIME, &folder_create_start_time);  // Start root folder create timer!
            if (mkdir(folder_path, 0777) != 0) {
                printf("Error creating root folder: %d\n", folder_id);
                free(statistics);
                return -1;
            }
            clock_gettime(CLOCK_REALTIME, &folder_create_stop_time);  // Stop root folder create timer!
            sub_timespec(folder_create_start_time, folder_create_stop_time, &folder_create_time);
            unsigned long _folder_create_time = get_time_in_usec(folder_create_time);
            statistics->folder[folder_id].folder_create_time += _folder_create_time;
            statistics->total_folder_create_time += _folder_create_time;

            printf("[%d-%02d-%02d %02d:%02d:%02d] Create folder: %d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, folder_id);

            /**
             * Create sub folders
            */
            statistics->folder[folder_id].file_create_size = 0;
            statistics->folder[folder_id].file_create_block = 0;
            statistics->folder[folder_id].file_create_open_time = 0;
            statistics->folder[folder_id].file_create_time = 0;
            for (int sub_folder_id = 0; sub_folder_id < SUB_FOLDERS; sub_folder_id++) {
                
                strcpy(folder_path, folder_storage);
                sprintf(root_folder, "%d", folder_id);
                strcat(folder_path, root_folder);
                strcat(folder_path, "/");
                sprintf(sub_folder, "%d", sub_folder_id);
                strcat(folder_path, sub_folder);
                clock_gettime(CLOCK_REALTIME, &folder_create_start_time);  // Start root folder create timer!
                if (mkdir(folder_path, 0777) != 0) {
                    printf("Error creating subfolder: %d in root folder: %d\n", sub_folder_id, folder_id);
                    return 0;
                }
                clock_gettime(CLOCK_REALTIME, &folder_create_stop_time);  // Stop root folder create timer!
                sub_timespec(folder_create_start_time, folder_create_stop_time, &folder_create_time);
                unsigned long _folder_create_time = get_time_in_usec(folder_create_time);
                statistics->folder[folder_id].folder_create_time += _folder_create_time;
                statistics->total_folder_create_time += _folder_create_time;

                // if (SUB_FOLDERS < 10)
                //     printf("|");

                /**
                 * Create files
                */
                for (int file_id = 0; file_id < MAX_FILES_PER_SUB_FOLDER; file_id++) {

                    // Create file path and name
                    strcpy(file_path, folder_path);
                    strcat(file_path, "/");
                    sprintf(file_name, "%s%d", "file-", file_id);
                    strcat(file_path, file_name);
                    strcat(file_path, ".bin");

                    //size_t file_size = 2048;
                    //memset(file_buffer, 0x55, file_size);                     // For testing only
                    file_size = (size_t) normal_random((double)5500.0, (double)1000.0, (double)MIN_FILE_SIZE, (double)MAX_FILE_SIZE) - CRC_SIZE;          // generate random file size in min-max range
                    if (file_size < MIN_FILE_SIZE || file_size > MAX_FILE_SIZE)
                        file_size = 5500;
                    memcpy(file_buffer, random_data, file_size);                // copy random data to file buffer
                    get_crc(file_buffer, file_size, file_buffer+file_size);     // generate CRC32 and append to the file buffer

                    // Create file
                    clock_gettime(CLOCK_REALTIME, &file_create_open_start_time);
                    int fd = open(file_path, O_CREAT | O_RDWR, 0777);
                    if (fd == -1) {
                        printf("Unable to open the file for writing: %s\n", file_path);
                        free(statistics);
                        return -1;                    
                    }
                    clock_gettime(CLOCK_REALTIME, &file_create_open_stop_time);

                    // Write data to the file
                    clock_gettime(CLOCK_REALTIME, &file_create_start_time);     // Start the timer for file creation!
                    ssize_t bytesWritten = write(fd, file_buffer, file_size+CRC_SIZE);
                    if (bytesWritten == -1 || bytesWritten != file_size+CRC_SIZE) {
                        printf("File bytes writing problem: %ld, %ld\n", bytesWritten, file_size+CRC_SIZE);
                        free(statistics);
                        return -1;                    
                    }
                    close(fd);
                    clock_gettime(CLOCK_REALTIME, &file_create_stop_time);      // Stop the timer for file creation!

                    // File statistics
                    statistics->folder[folder_id].file_create_size      += file_size+CRC_SIZE;
                    statistics->folder[folder_id].file_create_block     += ((file_size+CRC_SIZE)/statistics->fs_before.fs.f_bsize)+1;
                    sub_timespec(file_create_open_start_time, file_create_open_stop_time, &file_create_open_time);
                    unsigned long _file_create_open_time = get_time_in_usec(file_create_open_time);
                    statistics->folder[folder_id].file_create_open_time += _file_create_open_time;

                    sub_timespec(file_create_start_time, file_create_stop_time, &file_create_time);
                    unsigned long _file_create_time = get_time_in_usec(file_create_time);
                    statistics->folder[folder_id].file_create_time += _file_create_time;
                    if (_file_create_time < statistics->min_file_create_time) statistics->min_file_create_time = _file_create_time;
                    if (_file_create_time > statistics->max_file_create_time) statistics->max_file_create_time = _file_create_time;
                    
                    statistics->file_write_speed_histogram_bucket[get_file_speed_histogram_index(_file_create_time)]++;
                    //statistics->file_write_speed_histogram_bucket[0]++;
                    statistics->file_size_histogram_bucket[get_file_size_histogram_index(file_size)]++;

                    if (file_id % progress_bar_div == 0)
                        printf(".");

                } // File iterator
            } // Sub folder iterator

            statistics->folder[folder_id].folder_write_throughput = statistics->folder[folder_id].file_create_size / statistics->folder[folder_id].file_create_time;
            
            printf("[%ld,%d,%ld,%ld,%ld,%ld,%ld,%ld]",  statistics->folder[folder_id].file_create_size, 
                                                        statistics->folder[folder_id].file_create_block, 
                                                        statistics->folder[folder_id].file_create_open_time,
                                                        statistics->folder[folder_id].file_create_open_time/(MAX_FILES_PER_FOLDER),             // (MAX_FILES_PER_FOLDER/SUB_FOLDERS)
                                                        statistics->folder[folder_id].file_create_time,
                                                        statistics->folder[folder_id].file_create_time/(MAX_FILES_PER_FOLDER),                  // (MAX_FILES_PER_FOLDER/SUB_FOLDERS)
                                                        statistics->folder[folder_id].folder_create_time,
                                                        statistics->folder[folder_id].folder_write_throughput);
            printf("\n");
        } // folder iterator
        rv = statvfs(folder_storage, &statistics->fs_after.fs);
        if(rv == -1) {
            printf("Unable to read filesystem information. Exiting program\n");
            free(statistics);
            return -1;
        }

        // rv = statvfs(folder_storage, &fs_temp);
        // printf("Space: Total=%lu, Available=%lu, Used=%lu\n", 
        // fs_temp.f_blocks*fs_temp.f_bsize,   // total size
        // fs_temp.f_bavail*fs_temp.f_bsize,   // avaiable
        // fs_temp.f_blocks*fs_temp.f_bsize-fs_temp.f_bfree*fs_temp.f_bsize);  // Used
        // printf("Blocks: Total=%lu, Available=%lu, Used=%lu\n", 
        // fs_temp.f_blocks, fs_temp.f_bavail, fs_temp.f_blocks-fs_temp.f_bavail);
        // printf("Inodes: Total=%lu, Available=%lu, used=%lu\n", 
        // fs_temp.f_files, fs_temp.f_favail, fs_temp.f_files-fs_temp.f_favail);

    }

    /**
     * Read Folders and Files
    */
    if (reader == true) {
        struct timespec folder_read_start_time, folder_read_stop_time, folder_read_time, 
                        file_read_open_start_time, file_read_open_stop_time, file_read_open_time,
                        file_read_start_time, file_read_stop_time, file_read_time;
        struct stat st;
        int file_count = 0;
        struct dirent *entry;
        CRC32 crc32_file, crc32_computed;

        /**
         * Read folders
        */
        statistics->total_folder_read_time = 0;
        printf("[Date & Time] Read Folder: - ....................................................................................................[Size,Blocks,Open,Read,Folder Read]\n");
        for (int folder_id = 0; folder_id < MAX_FOLDERS; folder_id++) {
            
            t = time(NULL);
            tm = *localtime(&t);
            statistics->folder[folder_id].folder_read_time = 0;
            printf("[%d-%02d-%02d %02d:%02d:%02d] Read folder: %d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, folder_id);

            /**
             * Read sub folders
            */
            statistics->folder[folder_id].file_read_size = 0;
            statistics->folder[folder_id].file_read_block = 0;
            statistics->folder[folder_id].file_read_open_time = 0;
            statistics->folder[folder_id].file_read_time = 0;
            for (int sub_folder_id = 0; sub_folder_id < SUB_FOLDERS; sub_folder_id++) {
                
                strcpy(folder_path, folder_storage);
                sprintf(root_folder, "%d", folder_id);
                strcat(folder_path, root_folder);
                strcat(folder_path, "/");
                sprintf(sub_folder, "%d", sub_folder_id);
                strcat(folder_path, sub_folder);
                clock_gettime(CLOCK_REALTIME, &folder_read_start_time);  // Start folder read timer!
                DIR *dirp = opendir(folder_path);
                if (dirp == NULL) {
                    printf("Unable to open the folder for reading: %s\n", file_path);
                    free(statistics);
                    return -1;                    
                }
                closedir(dirp);
                clock_gettime(CLOCK_REALTIME, &folder_read_stop_time);  // Stop the timer!
                sub_timespec(folder_read_start_time, folder_read_stop_time, &folder_read_time);
                unsigned long _folder_read_time = get_time_in_usec(folder_read_time);
                statistics->folder[folder_id].folder_read_time += _folder_read_time;
                statistics->total_folder_read_time += _folder_read_time;
                
                // if (SUB_FOLDERS < 10)
                //     printf("|");

                /**
                 * Read files
                */
                for (int file_id = 0; file_id < MAX_FILES_PER_SUB_FOLDER; file_id++) {

                    // Read file path and name
                    strcpy(file_path, folder_path);
                    strcat(file_path, "/");
                    sprintf(file_name, "%s%d", "file-", file_id);
                    strcat(file_path, file_name);
                    strcat(file_path, ".bin");

                    clock_gettime(CLOCK_REALTIME, &file_read_open_start_time);
                    int fd = open(file_path, O_RDONLY);
                    if (fd == -1) {
                        printf("Unable to open the file for reading: %s\n", file_path);
                        free(statistics);
                        return -1;                    
                    }
                    clock_gettime(CLOCK_REALTIME, &file_read_open_stop_time);

                    clock_gettime(CLOCK_REALTIME, &file_read_start_time);
                    file_size = read(fd, file_buffer, sizeof(file_buffer));
                    if (file_size == -1) {
                        printf("Unable to read file\n");
                        free(statistics);
                        return -1;                    
                    }
                    close(fd);
                    clock_gettime(CLOCK_REALTIME, &file_read_stop_time);

                    // File statistics
                    statistics->folder[folder_id].file_read_size        += file_size;
                    statistics->folder[folder_id].file_read_block       += ((file_size)/statistics->fs_before.fs.f_bsize)+1;

                    memcpy(crc32_file.c, file_buffer+file_size-CRC_SIZE, sizeof(unsigned long));    // Read CRC-32 from the file
                    crc32_computed.l = get_crc(file_buffer, file_size-CRC_SIZE, crc32_computed.c);  // Compute CRC-32 of the buffer minus CRC-32 bytes at the end
                    if (crc32_computed.l != crc32_file.l) {
                        statistics->folder[folder_id].crc_failure_count++;
                        printf("CRC problem\n");
                    }
                    sub_timespec(file_read_open_start_time, file_read_open_stop_time, &file_read_open_time);
                    unsigned long _file_read_open_time = get_time_in_usec(file_read_open_time);
                    statistics->folder[folder_id].file_read_open_time   += _file_read_open_time;

                    sub_timespec(file_read_start_time, file_read_stop_time, &file_read_time);
                    unsigned long _file_read_time = get_time_in_usec(file_read_time);
                    statistics->folder[folder_id].file_read_time        += _file_read_time;

                    if (_file_read_time < statistics->min_file_read_time) statistics->min_file_read_time = _file_read_time;
                    if (_file_read_time > statistics->max_file_read_time) statistics->max_file_read_time = _file_read_time;

                    statistics->file_read_speed_histogram_bucket[get_file_speed_histogram_index(_file_read_time)]++;
                    //statistics->file_read_speed_histogram_bucket[0]++;
                    
                    if (file_id % progress_bar_div == 0)
                        printf(".");
                    
                } // File iterator
            } // Sub folder iterator

            statistics->folder[folder_id].folder_read_throughput = statistics->folder[folder_id].file_read_size / statistics->folder[folder_id].file_read_time;

            if (statistics->folder[folder_id].file_read_size  != statistics->folder[folder_id].file_create_size ||
                statistics->folder[folder_id].file_read_block != statistics->folder[folder_id].file_create_block) {
                printf("[%ld,%ld,%ld|%d,%d,%d]",    statistics->folder[folder_id].file_create_size,
                                                    statistics->folder[folder_id].file_read_size,
                                                    statistics->folder[folder_id].file_create_size - statistics->folder[folder_id].file_read_size,
                                                    statistics->folder[folder_id].file_create_block,
                                                    statistics->folder[folder_id].file_read_block,
                                                    statistics->folder[folder_id].file_create_block - statistics->folder[folder_id].file_read_block);
            }
            printf("[%ld,%d,%ld,%ld,%ld,%ld,%ld,%ld]",  statistics->folder[folder_id].file_read_size, 
                                                        statistics->folder[folder_id].file_read_block,
                                                        statistics->folder[folder_id].file_read_open_time,
                                                        statistics->folder[folder_id].file_read_open_time/(MAX_FILES_PER_FOLDER),       // (MAX_FILES_PER_FOLDER/SUB_FOLDERS)
                                                        statistics->folder[folder_id].file_read_time,
                                                        statistics->folder[folder_id].file_read_time/(MAX_FILES_PER_FOLDER),            // (MAX_FILES_PER_FOLDER/SUB_FOLDERS)
                                                        statistics->folder[folder_id].folder_read_time,
                                                        statistics->folder[folder_id].folder_read_throughput);

            printf("\n");

            // Build folder statistics
            // if (file_count != MAX_FILES_PER_FOLDER)
            //     statistics->folder[folder_id].incorrect_file_count  = MAX_FILES_PER_SUB_FOLDER - file_count;
            statistics->folder[folder_id].incorrect_file_count = 0;
            if (statistics->folder[folder_id].file_read_size != statistics->folder[folder_id].file_create_size)
                statistics->folder[folder_id].incorrect_file_size++;
            if (statistics->folder[folder_id].file_read_block != statistics->folder[folder_id].file_create_block)
                statistics->folder[folder_id].incorrect_file_block++;

        } // Folder iterator

        // rv = statvfs(folder_storage, &fs_temp);
        // printf("Space: Total=%lu, Available=%lu, Used=%lu\n", 
        // fs_temp.f_blocks*fs_temp.f_bsize,   // total size
        // fs_temp.f_bavail*fs_temp.f_bsize,   // avaiable
        // fs_temp.f_blocks*fs_temp.f_bsize-fs_temp.f_bfree*fs_temp.f_bsize);  // Used
        // printf("Blocks: Total=%lu, Available=%lu, Used=%lu\n", 
        // fs_temp.f_blocks, fs_temp.f_bavail, fs_temp.f_blocks-fs_temp.f_bavail);
        // printf("Inodes: Total=%lu, Available=%lu, used=%lu\n", 
        // fs_temp.f_files, fs_temp.f_favail, fs_temp.f_files-fs_temp.f_favail);

    }

    rv = statvfs(folder_storage, &statistics->fs_after.fs);
    if(rv == -1) {
        printf("Unable to read filesystem information. Exiting program\n");
        free(statistics);
        return -1;
    }

    // Run time stops
    clock_gettime(CLOCK_REALTIME, &run_time_stop);  // Start the timer to collect run time!
    sub_timespec(run_time_start, run_time_stop, &statistics->run_time);

    /**
     * Process statistics
    */
    for (int folder_id = 0; folder_id < MAX_FOLDERS; folder_id++) {
        statistics->total_file_create_size          += statistics->folder[folder_id].file_create_size;
        statistics->total_file_create_block         += statistics->folder[folder_id].file_create_block;        
        statistics->total_file_create_time          += statistics->folder[folder_id].file_create_time;
        statistics->total_file_create_open_time     += statistics->folder[folder_id].file_create_open_time;
        statistics->total_folder_create_time        += statistics->folder[folder_id].folder_create_time;

        statistics->total_file_read_size            += statistics->folder[folder_id].file_read_size;
        statistics->total_file_read_block           += statistics->folder[folder_id].file_read_block;        
        statistics->total_file_read_time            += statistics->folder[folder_id].file_read_time;
        statistics->total_file_read_open_time       += statistics->folder[folder_id].file_read_open_time;
        statistics->total_folder_read_time          += statistics->folder[folder_id].folder_read_time;

        statistics->total_crc_failure_count         += statistics->folder[folder_id].crc_failure_count;        
        statistics->total_incorrect_file_count      += statistics->folder[folder_id].incorrect_file_count;        
        statistics->total_incorrect_file_size       += statistics->folder[folder_id].incorrect_file_size;
        statistics->total_incorrect_file_block      += statistics->folder[folder_id].incorrect_file_block;

        // Folder write & read throughput
        // statistics->folder[folder_id].folder_write_throughput = statistics->folder[folder_id].file_create_size / statistics->folder[folder_id].file_create_time;
        // statistics->folder[folder_id].folder_read_throughput = statistics->folder[folder_id].file_read_size / statistics->folder[folder_id].file_read_time;
    }
    
    statistics->ave_file_create_open_time           = statistics->total_file_create_open_time / TOTAL_FILES;        //(MAX_FOLDERS * MAX_FILES_PER_FOLDER);
    statistics->ave_file_read_open_time             = statistics->total_file_read_open_time / TOTAL_FILES;          //(MAX_FOLDERS * MAX_FILES_PER_FOLDER);
    statistics->ave_file_create_time                = statistics->total_file_create_time / TOTAL_FILES;             //(MAX_FOLDERS * MAX_FILES_PER_FOLDER);
    statistics->ave_file_read_time                  = statistics->total_file_read_time / TOTAL_FILES;               //(MAX_FOLDERS * MAX_FILES_PER_FOLDER);
    
    if (statistics->total_file_read_size != statistics->total_file_create_size) {
        printf("Error: Total files size did not match: Read:%lu, Write:%lu, Diff:%lu\n", 
                                            statistics->total_file_read_size, 
                                            statistics->total_file_create_size,
                                            statistics->total_file_read_size-statistics->total_file_create_size);
    }
    if (statistics->total_file_read_block != statistics->total_file_create_block) {
        printf("Error: Total files blocks did not match: Read:%lu, Write:%lu, Diff:%lu\n", 
                                            statistics->total_file_read_block, 
                                            statistics->total_file_create_block,
                                            statistics->total_file_read_block-statistics->total_file_create_block);
    }

    /**
     * BEFORE
    */
    // Storage
    statistics->fs_before.space_total    = statistics->fs_before.fs.f_blocks  * statistics->fs_before.fs.f_bsize;
    statistics->fs_before.space_free     = statistics->fs_before.fs.f_bfree   * statistics->fs_before.fs.f_bsize;
    statistics->fs_before.space_used     = statistics->fs_before.space_total  - statistics->fs_before.space_free;
    // Blocks
    statistics->fs_before.blocks_total   = statistics->fs_before.fs.f_blocks;
    statistics->fs_before.blocks_free    = statistics->fs_before.fs.f_bfree;
    statistics->fs_before.blocks_used    = statistics->fs_before.blocks_total - statistics->fs_before.blocks_free;
    // Inodes
    statistics->fs_before.inodes_total   = statistics->fs_before.fs.f_files;
    statistics->fs_before.inodes_free    = statistics->fs_before.fs.f_ffree;
    statistics->fs_before.inodes_used    = statistics->fs_before.inodes_total - statistics->fs_before.inodes_free;

    /**
     * AFTER
    */
    // Storage
    statistics->fs_after.space_total     = statistics->fs_after.fs.f_blocks   * statistics->fs_after.fs.f_bsize;
    statistics->fs_after.space_free      = statistics->fs_after.fs.f_bfree    * statistics->fs_after.fs.f_bsize;
    statistics->fs_after.space_used      = statistics->fs_after.space_total   - statistics->fs_after.space_free;
    // Blocks
    statistics->fs_after.blocks_total    = statistics->fs_after.fs.f_blocks;
    statistics->fs_after.blocks_free     = statistics->fs_after.fs.f_bfree;
    statistics->fs_after.blocks_used     = statistics->fs_after.blocks_total  - statistics->fs_after.blocks_free;
    // Inodes
    statistics->fs_after.inodes_total    = statistics->fs_after.fs.f_files;
    statistics->fs_after.inodes_free     = statistics->fs_after.fs.f_ffree;
    statistics->fs_after.inodes_used     = statistics->fs_after.inodes_total  - statistics->fs_after.inodes_free;

    /**
     * Used
    */
    //statistics->fs_used.space_used       = statistics->fs_after.space_used  - statistics->fs_before.space_used;
    //statistics->fs_used.blocks_used      = statistics->fs_after.blocks_used - statistics->fs_before.blocks_used;
    //statistics->fs_used.inodes_used      = statistics->fs_after.inodes_used - statistics->fs_before.inodes_used;
    statistics->fs_used.space_used       = (statistics->fs_after.fs.f_blocks*statistics->fs_after.fs.f_bsize - statistics->fs_after.fs.f_bfree*statistics->fs_after.fs.f_bsize) - 
                                           (statistics->fs_before.fs.f_blocks*statistics->fs_before.fs.f_bsize - statistics->fs_before.fs.f_bfree*statistics->fs_before.fs.f_bsize);
    statistics->fs_used.blocks_used      = (statistics->fs_after.fs.f_blocks - statistics->fs_after.fs.f_bfree) - (statistics->fs_before.fs.f_blocks - statistics->fs_before.fs.f_bfree);
    statistics->fs_used.inodes_used      = (statistics->fs_after.fs.f_files - statistics->fs_after.fs.f_favail) - (statistics->fs_before.fs.f_files - statistics->fs_before.fs.f_favail);

    // rv = statvfs(folder_storage, &fs_temp);
    // printf("Space: Total=%lu, Available=%lu, Used=%lu\n", 
    // fs_temp.f_blocks*fs_temp.f_bsize,   // total size
    // fs_temp.f_bavail*fs_temp.f_bsize,   // avaiable
    // fs_temp.f_blocks*fs_temp.f_bsize-fs_temp.f_bfree*fs_temp.f_bsize);  // Used
    // printf("Blocks: Total=%lu, Available=%lu, Used=%lu\n", 
    // fs_temp.f_blocks, fs_temp.f_bavail, fs_temp.f_blocks-fs_temp.f_bavail);
    // printf("Inodes: Total=%lu, Available=%lu, used=%lu\n", 
    // fs_temp.f_files, fs_temp.f_favail, fs_temp.f_files-fs_temp.f_favail);

    /**
     * Display statistics
    */
    int divisor = MAX_FOLDERS / 20;
    printf("Total Files (TF): %d\n", TOTAL_FILES);
    printf("Folders: %d\n", MAX_FOLDERS);
    printf("Files per Subfolder: %d\n", MAX_FILES_PER_SUB_FOLDER);
    printf("Subfolder(s) per Folder: %d\n", SUB_FOLDERS);
    printf("Files per Folder: %d\n", MAX_FILES_PER_FOLDER);
    printf("Statistics Size (bytes): %ld\n", sizeof(STATISTICS));
    printf("File Block Size: %lu\n", statistics->fs_after.fs.f_bsize);
    //printf("Folders: %d\n", MAX_FOLDERS);
    //printf("Files/Folder: %d\n", MAX_FILES_PER_FOLDER);
    //printf("Total Files Created: %d\n", TOTAL_FILES);           //MAX_FOLDERS*MAX_FILES_PER_FOLDER);
    // File sizes histogram
    printf("File Sizes Histogram Buckets:");
    for (int i = 0; i < FILE_SIZE_HISTOGRAM_BUCKETS; i++)
        printf(" %d,", statistics->file_size_histogram_bucket[i]);
    printf("\n");
    printf("Total Run Time (TRT): %llu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
        get_time_in_usec(statistics->run_time), 
        get_time_in_usec(statistics->run_time)/1000000.0, 
        get_time_in_usec(statistics->run_time)/1000000.0/60.0, 
        get_time_in_usec(statistics->run_time)/1000000.0/60.0/60.0);
    printf("CRC-32 Failure: %d\n", statistics->total_crc_failure_count);
    printf("Incorrect File Count: %d\n", statistics->total_incorrect_file_count);
    printf("File Sizes Mismatch: %d\n", statistics->total_incorrect_file_size);
    printf("File Blocks Mismatch: %d\n", statistics->total_incorrect_file_block);
    printf("Space Used Before: %lu, After: %lu \n", statistics->fs_before.space_used, statistics->fs_after.space_used);
    printf("Blocks Used Before: %lu, After: %lu \n", statistics->fs_before.blocks_used, statistics->fs_after.blocks_used);
    printf("Inodes Used Before: %lu, After: %lu \n", statistics->fs_before.inodes_used, statistics->fs_after.inodes_used);
    printf("Space Used: %lu (bytes), %0.2fKB, %0.2fMB %0.2fGB \n", 
        statistics->fs_used.space_used,
        (float)(statistics->fs_used.space_used)/1000,
        (float)(statistics->fs_used.space_used)/1000000,
        (float)(statistics->fs_used.space_used)/1000000000);
    printf("Blocks Used: %lu, %0.2f thousand, %0.2f million\n", 
        statistics->fs_used.blocks_used, 
        (float)statistics->fs_used.blocks_used/1000,
        (float)statistics->fs_used.blocks_used/1000000);
    printf("Inodes Used: %lu, %0.2f thousand, %0.2f million, %0.2f billion\n",
        statistics->fs_used.inodes_used,
        (float)statistics->fs_used.inodes_used/1000,
        (float)statistics->fs_used.inodes_used/1000000,
        (float)statistics->fs_used.inodes_used/1000000000);
    printf("Disk Space Utilization Overheads: %0.1f%%\n", (float)((float)(statistics->fs_used.space_used - statistics->total_file_create_size) / 
                                                            (float)statistics->fs_used.space_used) * 100);
    printf("\n");

    // Write Statistics
    printf("Write statistics...\n");
    printf("File Write Speed Histogram:");
    long write_file_count = 0;
    for (int i = 0; i < FILE_SPEED_HISTOGRAM_BUCKETS; i++) {
        write_file_count += statistics->file_write_speed_histogram_bucket[i];
        printf(" %d,", statistics->file_write_speed_histogram_bucket[i]);
    }
    printf("[%ld]\n", write_file_count);
    printf("Total File Create for Write Time (TFCWT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
        statistics->total_file_create_open_time,
        (float)statistics->total_file_create_open_time/1000000.0,
        (float)statistics->total_file_create_open_time/1000000.0/60.0,
        (float)statistics->total_file_create_open_time/1000000.0/60.0/60.0);
    printf("File Create for Write Time_AVE (FCWT, usec): %lu\n", statistics->ave_file_create_open_time);
    printf("File Write Time_MIN (FWT, usec): %lu\n", statistics->min_file_create_time);
    printf("File Write Time_AVE (FWT, usec): %lu\n", statistics->ave_file_create_time);
    printf("File Write Time_MAX (FWT, usec): %lu\n", statistics->max_file_create_time);
    // printf("File Write Time_TOTAL (FWT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
    //     statistics->total_file_create_time, 
    //     (float)statistics->total_file_create_time/1000000.0, 
    //     (float)statistics->total_file_create_time/1000000.0/60.0, 
    //     (float)statistics->total_file_create_time/1000000.0/60.0/60.0);
    printf("Write Throughput (WTh, bytes/usec): %lu\n", (statistics->total_file_create_size / statistics->total_file_create_time)/*1048576*/);
    printf("Total Folder Write Time (TFWT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
        statistics->total_folder_create_time,
        (float)statistics->total_folder_create_time/1000000.0,
        (float)statistics->total_folder_create_time/1000000.0/60.0,
        (float)statistics->total_folder_create_time/1000000.0/60.0/60.0);
    printf("Total Files Write Time (TfWT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
        statistics->total_file_create_time,
        (float)statistics->total_file_create_time/1000000.0,
        (float)statistics->total_file_create_time/1000000.0/60.0,
        (float)statistics->total_file_create_time/1000000.0/60.0/60.0);
    printf("Total Write Time (TWT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
        (statistics->total_folder_create_time + statistics->total_file_create_time), 
        (float)(statistics->total_folder_create_time + statistics->total_file_create_time)/1000000.0, 
        (float)(statistics->total_folder_create_time + statistics->total_file_create_time)/1000000.0/60.0, 
        (float)(statistics->total_folder_create_time + statistics->total_file_create_time)/1000000.0/60.0/60.0);
    printf("Files Written Per Second (FWs): %0.2f\n", (double)1000000*(MAX_FOLDERS*MAX_FILES_PER_FOLDER)/statistics->total_file_create_time);
    printf("Blocks Written (Bk): %ld\n", statistics->total_file_create_block);
    printf("Blocks Written Per Second (BkWs): %ld\n", statistics->total_file_create_block * 1000000 / statistics->total_file_create_time);
    printf("Total Bytes Written (TBW): %lu (bytes), %0.2fKB, %0.2fMB %0.2fGB \n", 
        statistics->total_file_create_size, 
        (float)statistics->total_file_create_size/1000, 
        (float)statistics->total_file_create_size/1000000,
        (float)statistics->total_file_create_size/1000000000);
    printf("Folders Write Throughput (FWTh, bytes/usec): ");
    for (int i = divisor; i <= MAX_FOLDERS; i++) {
        if (i % divisor == 0)
            printf("%lu, ", statistics->folder[i-1].folder_write_throughput);
    }
    printf("\n");

    // Read Statistics
    printf("\nRead statistics...\n");
    printf("File Read Speed Histogram:");
    long read_file_count = 0;
    for (int i = 0; i < FILE_SPEED_HISTOGRAM_BUCKETS; i++) {
        read_file_count += statistics->file_read_speed_histogram_bucket[i];
        printf(" %d,", statistics->file_read_speed_histogram_bucket[i]);
    }
    printf("[%ld]\n", read_file_count);
    printf("Total File Open for Read Time (TFORT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n",
        statistics->total_file_read_open_time,
        (float)statistics->total_file_read_open_time/1000000.0,
        (float)statistics->total_file_read_open_time/1000000.0/60.0,
        (float)statistics->total_file_read_open_time/1000000.0/60.0/60.0);
    printf("File Open for Read Time_AVE (FORT, usec): %lu\n", statistics->ave_file_read_open_time);
    printf("File Read Time_MIN (FRT, usec): %lu\n", statistics->min_file_read_time);
    printf("File Read Time_AVE (FRT, usec): %lu\n", statistics->ave_file_read_time);
    printf("File Read Time_MAX (FRT, usec): %lu\n", statistics->max_file_read_time);
    // printf("File Read Time_TOTAL (FRT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
    //     statistics->total_file_read_time, 
    //     statistics->total_file_read_time/1000000.0, 
    //     statistics->total_file_read_time/1000000.0/60.0, 
    //     statistics->total_file_read_time/1000000.0/60.0/60.0);
    printf("Read Throughput (RTh, bytes/usec): %lu\n", (statistics->total_file_read_size / statistics->total_file_read_time)/*1048576*/);
    printf("Total Folder Read Time (TFRT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
        statistics->total_folder_read_time,
        statistics->total_folder_read_time/1000000.0,
        statistics->total_folder_read_time/1000000.0/60.0,
        statistics->total_folder_read_time/1000000.0/60.0/60.0);
    printf("Total Files Read Time (TfRT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
        statistics->total_file_read_time,
        statistics->total_file_read_time/1000000.0,
        statistics->total_file_read_time/1000000.0/60.0,
        statistics->total_file_read_time/1000000.0/60.0/60.0);
    printf("Total Read Time (TRT): %lu usec, %0.2f sec, %0.2f min, %0.2f hr\n", 
        (statistics->total_folder_read_time + statistics->total_file_read_time), 
        (statistics->total_folder_read_time + statistics->total_file_read_time)/1000000.0, 
        (statistics->total_folder_read_time + statistics->total_file_read_time)/1000000.0/60.0, 
        (statistics->total_folder_read_time + statistics->total_file_read_time)/1000000.0/60.0/60.0);
    printf("Files Read Per Second (FRs): %0.2f\n", (double)1000000*(MAX_FOLDERS*MAX_FILES_PER_FOLDER)/statistics->total_file_read_time);
    printf("Blocks Read (B): %ld\n", statistics->total_file_read_block);
    printf("Blocks Read Per Second (BkRs): %ld\n", statistics->total_file_read_block * 1000000 / statistics->total_file_read_time);
    printf("Total Bytes Read (TBR):  %lu (bytes), %0.2fKB, %0.2fMB %0.2fGB \n", 
        statistics->total_file_read_size, 
        (float)statistics->total_file_read_size/1000, 
        (float)statistics->total_file_read_size/1000000, 
        (float)statistics->total_file_read_size/1000000000);
    printf("Folders Read Throughput (FRTh, bytes/usec): ");
    for (int i = divisor; i <= MAX_FOLDERS; i++) {
        if (i % divisor == 0)
            printf("%lu, ", statistics->folder[i-1].folder_read_throughput);
    }
    printf("\n\n");

    /**
     * Log
    */
    printf("%ld,", sizeof(STATISTICS));
    printf("%lu,", statistics->fs_after.fs.f_bsize);
    printf("%d,", MAX_FOLDERS);
    printf("%d,", MAX_FILES_PER_FOLDER);
    printf("%d,", MAX_FOLDERS*MAX_FILES_PER_FOLDER);
    for (int i = 0; i < FILE_SIZE_HISTOGRAM_BUCKETS; i++)
        printf("%d,", statistics->file_size_histogram_bucket[i]);
    printf("%llu,", get_time_in_usec(statistics->run_time));
    printf("%d,", statistics->total_incorrect_file_count);
    printf("%d,", statistics->total_incorrect_file_size);
    printf("%d,", statistics->total_incorrect_file_block);
    printf("%lu,", statistics->fs_used.space_used);
    printf("%lu,", statistics->fs_used.blocks_used);
    printf("%lu,", statistics->fs_used.inodes_used);
    printf("%0.1f%%,", (float)((float)(statistics->fs_used.space_used - statistics->total_file_create_size) / 
                                                            (float)statistics->fs_used.space_used) * 100);
    // Write Statistics
    printf("%lu,", statistics->total_file_create_open_time);
    printf("%lu,", statistics->ave_file_create_open_time);
    printf("%lu,", statistics->min_file_create_time);
    printf("%lu,", statistics->ave_file_create_time);
    printf("%lu,", statistics->max_file_create_time);
    //printf("%lu,", statistics->total_file_create_time);
    printf("%lu,", (statistics->total_file_create_size / statistics->total_file_create_time));
    printf("%lu,", statistics->total_folder_create_time);
    printf("%lu,", statistics->total_file_create_time);
    printf("%lu,", (statistics->total_folder_create_time + statistics->total_file_create_time));
    printf("%0.2f,", (double)1000000*(MAX_FOLDERS*MAX_FILES_PER_FOLDER)/statistics->total_file_create_time);
    printf("%ld,", statistics->total_file_create_block);
    printf("%ld,", statistics->total_file_create_block * 1000000 / statistics->total_file_create_time);
    printf("%lu,", statistics->total_file_create_size);
    for (int i = 0; i < FILE_SPEED_HISTOGRAM_BUCKETS; i++)
        printf("%d,", statistics->file_write_speed_histogram_bucket[i]);
    for (int i = divisor; i <= MAX_FOLDERS; i++) {
        if (i % divisor == 0)
            printf("%lu,", statistics->folder[i-1].folder_write_throughput);
    }

    // Read Statistics
    printf("%lu,", statistics->total_file_read_open_time);
    printf("%lu,", statistics->ave_file_read_open_time);
    printf("%lu,", statistics->min_file_read_time);
    printf("%lu,", statistics->ave_file_read_time);
    printf("%lu,", statistics->max_file_read_time);
    //printf("%lu,", statistics->total_file_read_time);
    printf("%lu,", (statistics->total_file_read_size / statistics->total_file_read_time));
    printf("%lu,", statistics->total_folder_read_time);
    printf("%lu,", statistics->total_file_read_time);
    printf("%lu,", (statistics->total_folder_read_time + statistics->total_file_read_time));
    printf("%0.2f,", (double)1000000*(MAX_FOLDERS*MAX_FILES_PER_FOLDER)/statistics->total_file_read_time);
    printf("%ld,", statistics->total_file_read_block);
    printf("%ld,", statistics->total_file_read_block * 1000000 / statistics->total_file_read_time);
    printf("%lu,", statistics->total_file_read_size);
    for (int i = 0; i < FILE_SPEED_HISTOGRAM_BUCKETS; i++)
        printf("%d,", statistics->file_read_speed_histogram_bucket[i]);
    for (int i = divisor; i <= MAX_FOLDERS; i++) {
        if (i % divisor == 0)
            printf("%lu,", statistics->folder[i-1].folder_read_throughput);
    }

    printf("\n");

    /**
     * Free allocated memory
    */
    free(statistics);
    return 0;

}