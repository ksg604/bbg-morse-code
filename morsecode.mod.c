#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xb0826d43, "module_layout" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xb2f60bf1, "led_trigger_unregister_simple" },
	{ 0x702fc0f0, "misc_deregister" },
	{ 0x9c30e5c3, "led_trigger_register_simple" },
	{ 0xc5850110, "printk" },
	{ 0xb0beb5e8, "misc_register" },
	{ 0xc0a3a705, "led_trigger_event" },
	{ 0xf9a482f9, "msleep" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x4578f528, "__kfifo_to_user" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

