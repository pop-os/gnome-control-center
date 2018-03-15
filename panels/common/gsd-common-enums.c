


#include "gsd-device-manager.h"
#include "gsd-common-enums.h"

/* enumerations from "./gsd-device-manager.h" */
GType
gsd_device_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GSD_DEVICE_TYPE_MOUSE, "GSD_DEVICE_TYPE_MOUSE", "mouse" },
      { GSD_DEVICE_TYPE_KEYBOARD, "GSD_DEVICE_TYPE_KEYBOARD", "keyboard" },
      { GSD_DEVICE_TYPE_TOUCHPAD, "GSD_DEVICE_TYPE_TOUCHPAD", "touchpad" },
      { GSD_DEVICE_TYPE_TABLET, "GSD_DEVICE_TYPE_TABLET", "tablet" },
      { GSD_DEVICE_TYPE_TOUCHSCREEN, "GSD_DEVICE_TYPE_TOUCHSCREEN", "touchscreen" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GsdDeviceType", values);
  }
  return etype;
}



