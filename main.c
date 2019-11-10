#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("amnesiafs");
MODULE_AUTHOR("Louis Taylor");
MODULE_LICENSE("GPL");

static int __init amnesiafs_init(void)
{
	pr_debug("amnesiafs init\n");
	return 0;
}

static void __exit amnesiafs_exit(void)
{
	pr_debug("amnesiafs exit\n");
}

module_init(amnesiafs_init);
module_exit(amnesiafs_exit);
