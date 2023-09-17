/*
 *  ioctl test module -- Rich West.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
  
#define KEYBOARD_IOCTL_TEST _IOR(0, 6, char)

static int pseudo_device_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

static int KEYBOARD = 1;

static int __init initialization_routine(void) {
  printk("<1> Loading module\n");
  
  // disable IRQ1
  disable_irq(KEYBOARD);
  
  // function in kernel module
  pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl; 

  /* Start create proc entry */
  proc_entry = create_proc_entry("keyboard_ioctl_test", 0444, NULL);
  if(!proc_entry) {
    printk("<1> Error creating /proc entry.\n");
    return 1;
  }

  proc_entry->proc_fops = &pseudo_dev_proc_operations;

  return 0;
}

/* 'printk' version that prints to active tty. */
void my_printk(char *string) {
  struct tty_struct *my_tty;

  my_tty = current->signal->tty;

  if (my_tty != NULL) {
    (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
    (*my_tty->driver->ops->write)(my_tty, "\015\012", 2);
  }
} 


static void __exit cleanup_routine(void) {
  printk("<1> Dumping module\n");
  // enable IRQ1
  enable_irq(KEYBOARD);
  remove_proc_entry("keyboard_ioctl_test", NULL);

  return;
}

static inline unsigned char inb( unsigned short usPort ) {
    unsigned char uch;
   
    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {
    asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
}

char my_getchar ( void ) {
  //char c;

  //static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
  
  printk(KERN_ALERT "In my_getchar\n");
  /* Poll keyboard status register at port 0x64 checking bit 0 to see if
   * output buffer is full. We continue to poll if the msb of port 0x60
   * (data port) is set, as this indicates out-of-band data or a release
   * keystroke
   */
  //while( !(inb( 0x64 ) & 0x1) || ( ( c = inb( 0x60 ) ) & 0x80 ) );

  //return scancode[ (int)c ];
  
  return 'a';
}

/***
 * ioctl() entry point...
 */
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg) {
		
  int num_bytes_not_copied; 
  //int keyboard_port = 0x60;
  char kioc = (char) 0;
  
  switch (cmd){
  case KEYBOARD_IOCTL_TEST:
	printk(KERN_ALERT "In pseudo device ioctl\n");
	// Get the current scan code
	kioc = my_getchar();
	printk("KIOC: %c\n", kioc);
	num_bytes_not_copied = copy_to_user((char*)arg, &kioc, sizeof(char));
		   
	if (num_bytes_not_copied > 0) {
		printk("All bytes not copied!\n");
	} 
	break;
  
  default:
	return -EINVAL;
	break;
  }
  
  return 0;
}

module_init(initialization_routine); 
module_exit(cleanup_routine); 