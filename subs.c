#include "BfFS.h"

void sub_timespec(struct timespec start, struct timespec stop, struct timespec *delta)
{
    delta->tv_nsec = stop.tv_nsec - start.tv_nsec;
    delta->tv_sec  = stop.tv_sec - start.tv_sec;
    if (delta->tv_sec > 0 && delta->tv_nsec < 0)
    {
        delta->tv_nsec += NS_PER_SECOND;
        delta->tv_sec--;
    }
    else if (delta->tv_sec < 0 && delta->tv_nsec > 0)
    {
        delta->tv_nsec -= NS_PER_SECOND;
        delta->tv_sec++;
    }
}

unsigned long long get_time_in_usec(struct timespec t) {
    return (unsigned long long) (((t.tv_sec*1000000000) + t.tv_nsec) / 1000);
}

/**
 * This program uses the Box-Muller transform to generate random numbers following 
 * a normal distribution. The mean and standard deviation of the distribution are 
 * specified by the mean and stddev parameters, respectively. The lower and upper 
 * parameters specify the range of values that the random numbers should be within.
// */
double normal_random(double mean, double stddev, double lower, double upper) {
    double u, v, s, x;
    do {
        do {
            u = (double)rand() / RAND_MAX;
            v = 1.7156 * (double)rand() / RAND_MAX - 0.8578;
            s = u * u + v * v;
        } while (s >= 1 || s == 0);
        x = mean + stddev * v * sqrt(-2.0 * log(s) / s); 
    } while (x < lower && x > upper);
    return x;
}

/* uniform distribution, (0..1] */
double drand() {  
  return (rand()+1.0)/(RAND_MAX+1.0);
}

/* normal distribution, centered on 0, std dev 1 */
double normal_random2(double mean, double stddev, double lower, double upper) {
  return sqrt(-2*log(drand())) * cos(2*M_PI*drand());
}


unsigned long get_crc(unsigned char *file_buffer, size_t file_size, unsigned char *crc_buffer) {
    CRC32 crc;
    crc.l = crc32(0L, Z_NULL, 0);
    crc.l = crc32(crc.l, file_buffer, file_size);
    memcpy(crc_buffer, crc.c, sizeof(crc.l));
    return crc.l;
}

/**
 * File sizes histogram bucket look up, returns the index based on file size, 9 buckets
*/
int get_file_size_histogram_index(unsigned int file_size) {
    if (file_size >= 1 * 1024 && file_size <   2 * 1024) return 0;
    if (file_size >= 2 * 1024 && file_size <   3 * 1024) return 1;
    if (file_size >= 3 * 1024 && file_size <   4 * 1024) return 2;
    if (file_size >= 4 * 1024 && file_size <   5 * 1024) return 3;
    if (file_size >= 5 * 1024 && file_size <   6 * 1024) return 4;
    if (file_size >= 6 * 1024 && file_size <   7 * 1024) return 5;
    if (file_size >= 7 * 1024 && file_size <   8 * 1024) return 6;
    if (file_size >= 8 * 1024 && file_size <   9 * 1024) return 7;
    if (file_size >= 9 * 1024 && file_size <= 10 * 1024) return 8;
    //if (file_size >= 10)                                 return 9;
}

/**
 * File read/write speed histogram bucket lookup: from 1 to 100 usec in 20 histogram buckets
 * FILE_SPEED_HISTOGRAM_BUCKETS 20
 * 5,10,15,20,...
*/
// int get_file_speed_histogram_index(unsigned int speed) {
//     if (speed >= MIN_FILE_RW_SPEED && speed <= MAX_FILE_RW_SPEED)
//         return (int)((speed/5)-1);                                  // return (int)((speed/2)-1);
//     else
//         return FILE_SPEED_HISTOGRAM_BUCKETS-1;
// }
int get_file_speed_histogram_index(unsigned int write_speed) {
    if (write_speed >= 0   && write_speed < 5 ) return 0;
    if (write_speed >= 5   && write_speed < 10) return 1;
    if (write_speed >= 10  && write_speed < 15) return 2;
    if (write_speed >= 15  && write_speed < 20) return 3;
    if (write_speed >= 20  && write_speed < 25) return 4;
    if (write_speed >= 25  && write_speed < 30) return 5;
    if (write_speed >= 30  && write_speed < 35) return 6;
    if (write_speed >= 35  && write_speed < 40) return 7;
    if (write_speed >= 40  && write_speed < 45) return 8;
    if (write_speed >= 45  && write_speed < 50) return 9;
    if (write_speed >= 50  && write_speed < 55) return 10;
    if (write_speed >= 55  && write_speed < 60) return 11;
    if (write_speed >= 60  && write_speed < 65) return 12;
    if (write_speed >= 65  && write_speed < 70) return 13;
    if (write_speed >= 70  && write_speed < 75) return 14;
    if (write_speed >= 75  && write_speed < 80) return 15;
    if (write_speed >= 80  && write_speed < 85) return 16;
    if (write_speed >= 85  && write_speed < 90) return 17;
    if (write_speed >= 90  && write_speed < 95) return 18;
    if (write_speed >= 95)                      return 19;
}

/**
 *  File write speed histogram bucket lookup, file write speed is in microseconds
*/
// int get_file_write_speed_histogram_index(unsigned int write_speed) {
//     if (write_speed >= 20  && write_speed < 22) return 0;
//     if (write_speed >= 22  && write_speed < 24) return 1;
//     if (write_speed >= 24  && write_speed < 26) return 2;
//     if (write_speed >= 26  && write_speed < 29) return 3;
//     if (write_speed >= 29  && write_speed < 31) return 4;
//     if (write_speed >= 31  && write_speed < 35) return 5;
//     if (write_speed >= 35  && write_speed < 38) return 6;
//     if (write_speed >= 38  && write_speed < 42) return 7;
//     if (write_speed >= 42  && write_speed < 46) return 8;
//     if (write_speed >= 46  && write_speed < 50) return 9;
//     if (write_speed >= 50  && write_speed < 55) return 10;
//     if (write_speed >= 55  && write_speed < 61) return 11;
//     if (write_speed >= 61  && write_speed < 67) return 12;
//     if (write_speed >= 67  && write_speed < 73) return 13;
//     if (write_speed >= 73  && write_speed < 81) return 14;
//     if (write_speed >= 81  && write_speed < 89) return 15;
//     if (write_speed >= 89  && write_speed < 98) return 16;
//     if (write_speed >= 98  && write_speed < 107) return 17;
//     if (write_speed >= 107  && write_speed <118) return 18;
//     if (write_speed >= 118)                      return 19;
// }

/**
 *  File read speed histogram bucket lookup, file read speed is in microseconds
*/
// int get_file_read_speed_histogram_index(unsigned int read_speed) {
//     if (read_speed >= 10  && read_speed < 11) return 0;
//     if (read_speed >= 11  && read_speed < 12) return 1;
//     if (read_speed >= 12  && read_speed < 14) return 2;
//     if (read_speed >= 14  && read_speed < 15) return 3;
//     if (read_speed >= 15  && read_speed < 16) return 4;
//     if (read_speed >= 16  && read_speed < 18) return 5;
//     if (read_speed >= 18  && read_speed < 20) return 6;
//     if (read_speed >= 20  && read_speed < 22) return 7;
//     if (read_speed >= 22  && read_speed < 24) return 8;
//     if (read_speed >= 24  && read_speed < 26) return 9;
//     if (read_speed >= 26  && read_speed < 29) return 10;
//     if (read_speed >= 29  && read_speed < 31) return 11;
//     if (read_speed >= 31  && read_speed < 35) return 12;
//     if (read_speed >= 35  && read_speed < 38) return 13;
//     if (read_speed >= 38  && read_speed < 42) return 14;
//     if (read_speed >= 42  && read_speed < 46) return 15;
//     if (read_speed >= 46  && read_speed < 50) return 16;
//     if (read_speed >= 50  && read_speed < 55) return 17;
//     if (read_speed >= 55  && read_speed < 61) return 18;
//     if (read_speed >= 61)                     return 19;
// }