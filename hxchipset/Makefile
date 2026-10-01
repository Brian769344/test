# Makefile for the Himax touchscreen drivers.

ifndef CONFIG_HIMAX_SUPPORT_KERNEL_419
ifneq ($(filter y, $(CONFIG_FB)),)
	ccflags-y += -DHX_CONFIG_FB
endif

ifneq ($(filter y, $(CONFIG_DRM)),)
	ccflags-y += -DHX_CONFIG_DRM
endif
endif

ifneq ($(filter m, $(CONFIG_TOUCHSCREEN_HIMAX_COMMON)),)
  ccflags-y += -D__HIMAX_MOD__
endif
# obj-y += himax_modular_table.o

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_DEBUG)),)
	himax_mmi-objs += himax_debug.o
endif

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_SELF_TEST)),)
	himax_mmi-objs += himax_self_test.o
endif

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_COMMON)),)
	himax_mmi-objs += himax_ic_core.o
endif


ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_IC_HX83192)),)
  ccflags-y += -D__HIMAX_HX83192_MOD__
	himax_mmi-objs += himax_ic_HX83192.o
endif

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_IC_HX83193)),)
  ccflags-y += -D__HIMAX_HX83193_MOD__
	himax_mmi-objs += himax_ic_HX83193.o
endif

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_IC_HX83194)),)
  ccflags-y += -D__HIMAX_HX83194_MOD__
	himax_mmi-objs += himax_ic_HX83194.o
endif

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_IC_HX83195)),)
  ccflags-y += -D__HIMAX_HX83195_MOD__
	himax_mmi-objs += himax_ic_HX83195.o
endif

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_IC_HX83180)),)
  ccflags-y += -D__HIMAX_HX83180_MOD__
	himax_mmi-objs += himax_ic_HX83180.o
endif

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_IC_HX83181)),)
  ccflags-y += -D__HIMAX_HX83181_MOD__
	himax_mmi-objs += himax_ic_HX83181.o
endif

ifneq ($(filter y m, $(CONFIG_TOUCHSCREEN_HIMAX_IC_HX8530)),)
  ccflags-y += -D__HIMAX_HX8530_MOD__
	himax_mmi-objs += himax_ic_HX8530.o
endif

ifneq ($(filter m, $(CONFIG_TOUCHSCREEN_HIMAX_COMMON)),)
  ccflags-y += -D__HIMAX_MOD__
  himax_mmi-objs += himax_common.o
  himax_mmi-objs += himax_platform.o
  ifneq ($(filter y m,$(CONFIG_TOUCHSCREEN_HIMAX_OFR_SPI)),)
    himax_mmi-objs += himax_OFR_platform.o
  endif
  obj-m += himax_mmi.o
endif
ifneq ($(filter y, $(CONFIG_TOUCHSCREEN_HIMAX_COMMON)),)
  himax_mmi-objs += himax_common.o
  himax_mmi-objs += himax_platform.o
  ifneq ($(filter y m,$(CONFIG_TOUCHSCREEN_HIMAX_OFR_SPI)),)
    himax_mmi-objs += himax_OFR_platform.o
  endif
  obj-y += himax_mmi.o
endif

