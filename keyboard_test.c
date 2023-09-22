#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define KEYBOARD_IOCTL_TEST _IOR(0, 6, char)

static int num_char = 200;
static int shift_key = 0;

int main () {  
  char buffer[num_char], *read, *write, value;
  char *out_of_buffer = &buffer[num_char];
  read = write = buffer;
  value = (char) 0;
  int fd = open ("/proc/keyboard_ioctl_test", O_RDONLY);
  int response;
  printf("***HW1***\nPlease type some characters then press the Enter key.\n");
  
  while (value != '\n') {
	  /* write the character */
	  //printf("IOCTL call\n");
	  response = ioctl (fd, KEYBOARD_IOCTL_TEST, &value);
	  //printf("Returned to user-level program\n");
	  if (response < 0) {
		  printf("Something went wrong...\n");
		  exit(-1);
	  }
	  //printf("Value received: %c\n", value);
	  if (value != '\n') {
		  // handle backspace
		  if (value == '\b') {
			  // shift write pointer to previous space
			  write--;
		  } else {
			  *write++ = value;
			  if (write == out_of_buffer) {
				  break;
			  }
		  }
	  }
  }
  
  printf("You have typed:\n");
  /* read the character */
  while (read != write) {
	  if (read == out_of_buffer) {
		  break;
	  }
	  
	  putchar(*read);
	  read++;
  }
  putchar('\n');

  return 0;
}