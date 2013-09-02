# Core module
obj-$(CONFIG_FB_TFT)             += fbtft.o
fbtft-y                          += fbtft-core.o fbtft-sysfs.o fbtft-bus.o fbtft-io.o

# drivers
obj-$(CONFIG_FB_TFT_HX8340BN)    += fb_hx8340bn.o
obj-$(CONFIG_FB_TFT_HX8347D)     += fb_hx8347d.o
obj-$(CONFIG_FB_TFT_ILI9320)     += fb_ili9320.o
obj-$(CONFIG_FB_TFT_ILI9325)     += fb_ili9325.o
obj-$(CONFIG_FB_TFT_ILI9341)     += fb_ili9341.o
obj-$(CONFIG_FB_TFT_PCD8544)     += fb_pcd8544.o
obj-$(CONFIG_FB_TFT_SSD1289)     += fb_ssd1289.o
obj-$(CONFIG_FB_TFT_SSD1351)     += fb_ssd1351.o
obj-$(CONFIG_FB_TFT_ST7735R)     += fb_st7735r.o
obj-$(CONFIG_FB_ST7565)          += fb_st7565.o
obj-$(CONFIG_FB_FLEX)            += flexfb.o

# Device modules
obj-$(CONFIG_FB_TFT_FBTFT_DEVICE) += fbtft_device.o
obj-$(CONFIG_TOUCHSCREEN_ADS7846_DEVICE) += ads7846_device.o
