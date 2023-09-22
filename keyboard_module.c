#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>

MODULE_LICENSE("GPL");
  
#define KEYBOARD_IOCTL_TEST _IOR(0, 6, char)

// for IRQ 21
#define KEYBOARD 21

// Add (){}"!@#$%^&_|<>,/?
static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

// store the scancode received
static unsigned char scancode_received;

static unsigned char status;

static bool key_pressed = false;

// declare wait queue
static DECLARE_WAIT_QUEUE_HEAD(kbd_waitq);

static int pseudo_device_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

static inline unsigned char inb( unsigned short usPort ) {
    unsigned char uch;
   
    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

// keyboard interrupt handler
static irqreturn_t kbd_irq_handler(int irq, void* dev_id) {
	printk("<1> Handler called\n");
	// get data from keyboard data port
	status = inb(0x64);
	
	// Check bit 0 to see if data is present in data register or not
	if (status & 0x01) {
		scancode_received = inb(0x60);
		printk("<1> Raw Scancode received: 0x%02x\n", scancode_received);
		
		key_pressed = true;
		
		// wake up waiting processes
		wake_up_interruptible(&kbd_waitq);
		
		return IRQ_HANDLED;
	}
	
	return IRQ_NONE;
}

static int __init initialization_routine(void) {
  printk("<1> Loading module\n");
  
  // free IRQ 21
  //free_irq(KEYBOARD, NULL);
  
  if (request_irq(KEYBOARD, kbd_irq_handler, IRQF_SHARED,"keyboard",
  (void *)kbd_irq_handler)) {
	  printk("<1> Registering keyboard interrupt handler failed!\n");
	  return -EBUSY;
  }
  
  // initialize the wait queue
  init_waitqueue_head(&kbd_waitq);
  
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
  // Unregister keyboard interrupt handler
  free_irq(KEYBOARD, (void *)kbd_irq_handler);
  
  remove_proc_entry("keyboard_ioctl_test", NULL);

  return;
}

char my_getchar ( void ) {
  // backspace, shift, ctrl (r, p, R, P)
  int result;
  
  printk("<1> In my_getchar\n");
  
  // put current process to sleep
  result = wait_event_interruptible(kbd_waitq, key_pressed);
  key_pressed = false;
  
  if (result == -ERESTARTSYS) {
	  printk("<1>Interrupted by a signal\n");
	  return -EINTR;
  } else if (result == 0) {
	  printk("<1> Key pressed: %d\n", (int) scancode_received);
	  return scancode[ (int)scancode_received ];
  } else {
	  printk("<1> Error occurred while waiting\n");
	  return -EFAULT;
  }
  
  return 0;
}

/***
 * ioctl() entry point...
 */
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg) {
		
  int num_bytes_not_copied; 
  char kioc = (char) 0;
  
  switch (cmd){
  case KEYBOARD_IOCTL_TEST:
	printk("<1> In pseudo device ioctl\n");
	// Get the current scan code
	kioc = my_getchar();
	printk("<1> KIOC: %c\n", kioc);
	num_bytes_not_copied = copy_to_user((char*)arg, &kioc, sizeof(char));
		   
	if (num_bytes_not_copied > 0) {
		printk("<1> All bytes not copied!\n");
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