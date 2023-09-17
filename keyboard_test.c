#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define KEYBOARD_IOCTL_TEST _IOR(0, 6, char)

static int num_char = 200;

int main () {  
  char buffer[num_char], *read, *write, ch, value;
  char *out_of_buffer = &buffer[num_char];
  read = write = buffer;
  value = (char) 0;
  int fd = open ("/proc/keyboard_ioctl_test", O_RDONLY);
  int response;

  if (fd < 0) {
	printf("Something went wrong with file open...\n");
	exit(-1);
  }
  
  printf("***HW1***\nPlease type some characters then press the Enter key.\n");
  
  while (1) {
	  response = ioctl (fd, KEYBOARD_IOCTL_TEST, &value);
	  if (response == -1) {
		printf("Something went wrong...\n");
		exit(-1);
	  }
	  /* write the character */
	  while (value != '\n') {
		  *write++ = value;
		  if (write == out_of_buffer) {
			  write = buffer;
		  }
	  }
	  printf("You have typed:\n");
	  /* read the character */
	  while (read != write) {
		  if (read == out_of_buffer) {
			  read = buffer;
		  }
		  
		  putchar(*read);
		  read++;
	  }
	  putchar('\n');
  }

  return 0;
}