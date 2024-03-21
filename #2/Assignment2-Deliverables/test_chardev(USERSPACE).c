/*
Phil Nevins
Assignment 2 userspace program
This is a user space program to interact with a char device driver
This program will request a value from the user and update the
variable syscall_val in the char device file
ECE373
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/my_chardev"

int main() {
    int fd, ret;
    int old_value, new_value;

    // Open the character device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return -1;
    }

    //Read the current value of syscall_val
    ret = read(fd, &old_value, sizeof(int));
    if (ret < 0) {
        perror("Failed to read syscall_val");
        close(fd);
        return -1;
    }
    printf("Current value of syscall_val: %d\n", old_value);

    //Write a new value to change syscall_val
    printf("Enter new value for syscall_val: ");
    scanf("%d", &new_value);
    ret = write(fd, &new_value, sizeof(int));
    if (ret < 0) {
        perror("Failed to write new value");
        close(fd);
        return -1;
    }

    //Read value back out
    ret = read(fd, &new_value, sizeof(int));
    if (ret < 0) {
        perror("Failed to read new value");
        close(fd);
        return -1;
    }
    printf("New value of syscall_val: %d\n", new_value);

    //Close device file
    close(fd);

    return 0;
}