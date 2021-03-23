#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

unsigned char *pbuffer;
int rd_offset=0;
int wr_offset=0;
int buflen=0;
struct cdev cdev;
dev_t pdevid;
int ndevices=1;
struct device *pdev; 
struct class *pclass;
int ret;
#define MAX_SIZE 1024
char* tbuf;


int pseudo_open(struct inode* inode , struct file* file)
{
 printk("Pseudo--open method\n");
 return 0;
}

int pseudo_close(struct inode* inode , struct file* file)
{
 printk("Pseudo --release method\n");
 return 0;
}

ssize_t pseudo_read(struct file* file, char __user *ubuf , size_t size, loff_t * off)
{
 int rcount;
 if(buflen==0)
//wr_offset-rd_offset==0
 {
   printk("buffer is empty\n");
   return 0;
 }
 rcount = size;
 if(rcount > buflen)
 rcount = buflen;
//min of buflen, size
 ret=copy_to_user(ubuf, pbuffer + rd_offset,rcount);
 if(ret)
 {
   printk("copy to user failed\n");
   return -EFAULT;
 }
 rd_offset+=rcount;
 buflen -= rcount;
 printk("Pseudo--read method\n");
 return 0;
}

ssize_t pseudo_write(struct file* file,const  char __user * ubuf , size_t size, loff_t * off) 
{
 int wcount;
 if(wr_offset >= MAX_SIZE)
 {
   printk("buffer is full\n");
   return -ENOSPC;
 }
 wcount = size;
 if(wcount > MAX_SIZE - wr_offset)
    wcount = MAX_SIZE - wr_offset;
//min
 ret=copy_from_user(ubuf, pbuffer + wr_offset,wcount);
 
 if(ret)
 {
   printk("copy from user failed\n");
   return -EFAULT;
 }
 wr_offset+=wcount;
 buflen += wcount;
 printk("Pseudo--write method\n");
 return -ENOSPC;
}


struct file_operations fops ={
 .open     = pseudo_open,
 .release  = pseudo_close,
 .write    = pseudo_write,
 .read     = pseudo_read
};

static int __init pseudo_init(void)
{
 int ret,i=0;
  ret=alloc_chrdev_region(&pdevid, 0, ndevices, "pseudo_sample");
  if(ret) 
  {
   printk("Pseudo: Failed to register driver\n");
   return -EINVAL;
  }
 cdev_init(&cdev,&fops);
 kobject_set_name(&cdev.kobj ,"pdevice%d",i);
 ret = cdev_add(&cdev,pdevid,1);

 pclass = class_create(THIS_MODULE, "pseudo_class");
 pdev = device_create(pclass, NULL, pdevid, NULL, "psample%d",i);

 pbuffer = kmalloc(MAX_SIZE, GFP_KERNEL);

 printk("Successfully registered,major=%d,minor=%d\n",MAJOR(pdevid),MINOR(pdevid));
 printk("Pseudo Driver Sample..welcome\n");
 return 0;
}
static void __exit pseudo_exit(void)
{

cdev_del(&cdev);
unregister_chrdev_region(pdevid, ndevices);
printk("Pseudo Driver Sample..Bye\n");
device_destroy(pclass, pdevid);
class_destroy(pclass);
kfree(pbuffer);

}

module_init(pseudo_init);
module_exit(pseudo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sayan Dey");
MODULE_DESCRIPTION("A Hello, World Module");



