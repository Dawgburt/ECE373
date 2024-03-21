/*
Phil Nevins
Assignment 3 userspace program
This is a user space program to interact with a char device driver
This program will request a value from the user and update the
variable syscall_val in the char device file.
 Now for assignment 3 this will turn the LED0 of the network driver on for 2 seconds then off
ECE373
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
int fd;
uint32_t reg_val;


int main(int argc, char *argv[]){

    fd = open("/dev/hw3", O_RDWR);
    if(fd == -1){

        printf("Open failed!! Error num %d\n", errno);
        return -1;

    }

    /* Read and print value */
    read(fd, &reg_val, sizeof(uint32_t));
    printf("The value from the driver is %08x \n", reg_val);

    /* Turn on the LED0 */
    reg_val = 14;
    write(fd,&reg_val, sizeof(uint32_t));
    lseek(fd, 0, SEEK_SET);
    printf("Writing %08x to the driver to turn on LED0\n",reg_val);

    /* Sleep for 2  seconds */
    sleep(2);

    /* Turn off the LED0 */
    reg_val = 15;
    write(fd,&reg_val, sizeof(uint32_t));
    lseek(fd, 0, SEEK_SET);
    printf("Writing %08x to the driver to turn off LED0\n",reg_val);

    close(fd);
    return 0;
}}