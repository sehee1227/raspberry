#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sehee Lee");
MODULE_DESCRIPTION("RASPBERRY PI GPIO MODULE");
MODULE_INFO(intree, "Y");

#define BCM_IO_BASE	0xFE000000
#define	GPIO_BASE	(BCM_IO_BASE + 0x200000)

#define GPIO_SIZE	(256)
#define GPIO_IN(g)	(*(gpio + (g/10)) &= ~(7 << ((g%10)*3)))
#define GPIO_OUT(g)	(*(gpio + (g/10)) &= 1 << ((g%10)*3))

#define GPIO_SET(g)	(*(gpio + 7) = (1 << g))
#define GPIO_CLR(g)	(*(gpio +10) = (1 << g))
#define GPIO_GET(g)	(*(gpio +13) & (1 << g))

#define GPIO_MAJOR	200
#define	GPIO_MINOR	0
#define	GPIO_DEVICE	"gpioled"
#define GPIO_LED  18

volatile unsigned *gpio;
static char msg[BLOCK_SIZE] = {0};

static int gpio_open(struct inode*, struct file*);
static ssize_t gpio_read(struct file*, char*, size_t, loff_t*); 
static ssize_t gpio_write(struct file*, const char*, size_t, loff_t*); 
static int gpio_close(struct inode*, struct file*);

static struct file_operations gpio_fops = {
	.owner = THIS_MODULE,
	.read = gpio_read,
	.write = gpio_write,
	.open = gpio_open,
	.release = gpio_close,
};

struct cdev gpio_cdev;

int init_modules(void)
{
	dev_t devno;
	unsigned int count;
	static void *map;
	int err;

	printk(KERN_INFO, "Hello Module");

	devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
	err = register_chrdev_region(devno, 1, GPIO_DEVICE);

	if(err <0){
		printk("ERROR :Device register\n");
		return -1;
	}
	cdev_init(&gpio_cdev, &gpio_fops);

	gpio_cdev.owner = THIS_MODULE;
	gpio_cdev.ops = &gpio_fops;
	count = 1;

	err = cdev_add(&gpio_cdev, devno, count);
	if(err <0){
		printk("ERROR :Device Add\n");
		return -1;
	}

	printk("'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
	printk("'chmod 666 /dev/%s'\n", GPIO_DEVICE);

	map = ioremap(GPIO_BASE, GPIO_SIZE);

	if(!map){
		printk("ERROR :mapping GPIO memory\n");
		iounmap(map);
		return -EBUSY;
	}

	gpio = (volatile unsigned int*)map;

	GPIO_IN(GPIO_LED);
	GPIO_OUT(GPIO_LED);

	return 0;
}

void cleanup_modules(void)
{
	dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
	unregister_chrdev_region(devno, 1);

	cdev_del(&gpio_cdev);

	if(gpio){
		iounmap(gpio);
	}

	printk(KERN_INFO "Good-bye module!\n");

	return;
}

module_init(init_modules);
module_exit(cleanup_modules);


static int gpio_open(struct inode* inode, struct file* file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static ssize_t gpio_read(struct file* inode, char* buf, size_t len, loff_t* off)
{
	int count;
	strcat(msg, "from Kernel");
	count = copy_to_user(buf, msg, strlen(msg)+1);
	return count;
}

static ssize_t gpio_write(struct file* file, const char* buf, size_t len, loff_t* off) 
{
	short count;
	memset(msg, 0, BLOCK_SIZE);
	count = copy_from_user(msg, buf, len);

	(!strcmp(msg, "0")) ? GPIO_CLR(GPIO_LED) : GPIO_SET(GPIO_LED);
	printk("GPIO Device write %s(%d)\n", msg, len);

	return count;
}
static int gpio_close(struct inode* inode, struct file* file)
{

	module_put(THIS_MODULE);
	return 0;
}


