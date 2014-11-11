/*
  * *  Modified EMGD Backlight extra driver module for EMGD v.1.5.2
  * *
  * *  Contributers: Adrian Cheater <adrian dot cheater at gmail dot com>
  * *
  * *  Copyright (c) 2011 Tista
  * *
  * *  Based onprogear_bl.c driver by Marcin Juszkiewicz
  * *  <linux at hrw dot one dot pl>
  * *
  * *  Based on Progear LCD driver by M Schacht
  * *  <mschacht at alumni dot washington dot edu>
  * *
  * *  Based on Sharp's Corgi Backlight Driver
  * *  Based on Backlight Driver for HP Jornada 680
  * *
  * *  Documentation on backlight operation provided by Intel:
  * *  Internal LVDS Dynamic Backlight Brightness Control <http://edc.intel.com/Link.aspx?id=3930>
  * *
  * *  The Poulsbo supports LVDS Backlight control via the config byte at 0xF4,
  * *  providing 255 levels of brightness. However, brightness 0 (off)
  * *  is not generally useful unless blanking the display (Which should
  * *  ideally be powering down the LCD at the same time).
  * *
  * *  This driver creates 17 levels of brightness in steps of 15.
  * *  e.g. echo 0 > brightness will set the intensity to 15, 1 to 30, 2 to 45,
  * *  16 to 255
  * *
  * *  This program is free software; you can redistribute it and/or modify
  * *  it under the terms of the GNU General Public License version 2 as
  * *  published by the Free Software Foundation.
  * *
  * * --------------------------------------------------------------------
  * *
  * */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/pci.h>

static struct pci_dev *gfx_dev = NULL;
static u8 last_intensity_set;
static struct platform_device *emgdbl_device;

static int emgdbl_set_intensity(struct backlight_device *bd)
{
	u8 intensity;
	intensity = bd->props.brightness;

	if (bd->props.power != FB_BLANK_UNBLANK || bd->props.fb_blank != FB_BLANK_UNBLANK )
	{
		intensity = 0;
	} else
	{
		++intensity;
	}

	if( intensity * 15 != last_intensity_set )
	{
		last_intensity_set = intensity * 15;
		pci_write_config_byte( gfx_dev, 0xF4, last_intensity_set );
	}
	return 0;
}

static int emgdbl_get_intensity(struct backlight_device *bd)
{
	u8 intensity;
	pci_read_config_byte( gfx_dev, 0xF4, &intensity );
	last_intensity_set = intensity;
	if( intensity == 0 ) return 0;
	return (intensity / 15) - 1;
}

static struct backlight_ops emgdbl_ops =
{
	.get_brightness = emgdbl_get_intensity,
	.update_status = emgdbl_set_intensity,
};

static int emgdbl_probe(struct platform_device *pdev)
{
	struct backlight_properties props;
	struct backlight_device *emgd_backlight_device = NULL; 

	gfx_dev = pci_get_device(PCI_VENDOR_ID_INTEL, 0x8108, NULL);
	if (!gfx_dev)
	{
		printk("Intel SCH Poulsbo graphics controller not found.\n");
		return -ENODEV;
	}

	memset(&props, 0, sizeof(struct backlight_properties));
	props.type = BACKLIGHT_PLATFORM;
	props.max_brightness = 16;
	props.power = FB_BLANK_UNBLANK;
	props.brightness = emgdbl_get_intensity(NULL);

	emgd_backlight_device = backlight_device_register("emgd_psb", &pdev->dev, NULL, &emgdbl_ops, &props);
	if (IS_ERR(emgd_backlight_device)) 
	{
		pci_dev_put(gfx_dev);
		return PTR_ERR(emgd_backlight_device);
	}

	platform_set_drvdata(pdev, emgd_backlight_device);
	return 0;
}

static int emgdbl_remove(struct platform_device *pdev)
{
	struct backlight_device *bd = platform_get_drvdata(pdev);
	backlight_device_unregister(bd);
	pci_dev_put(gfx_dev);
	return 0;
}

static struct platform_driver emgdbl_driver =
{
	.probe = emgdbl_probe,
	.remove = emgdbl_remove,
	.driver = {
		.name = "emgd_psb",
	},
};

static int __init emgdbl_init(void)
{
	int ret = platform_driver_register(&emgdbl_driver);
	if (ret) return ret;
	emgdbl_device = platform_device_register_simple("emgd_psb", -1, NULL, 0);
	if (IS_ERR(emgdbl_device))
	{
		platform_driver_unregister(&emgdbl_driver);
		return PTR_ERR(emgdbl_device);
	}

	return 0;
}

static void __exit emgdbl_exit(void)
{
	platform_device_unregister(emgdbl_device);
	platform_driver_unregister(&emgdbl_driver);
}

module_init(emgdbl_init);
module_exit(emgdbl_exit);

MODULE_AUTHOR("Adrian Cheater<adrian.cheater@gmail.com>");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("EMGD Poulsbo Backlight Driver");
MODULE_LICENSE("GPL");
