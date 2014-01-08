#!/bin/sh

# Support legacy applications that still refer to gnome-control-center in Unity
if [ "$XDG_CURRENT_DESKTOP" = "Unity" ] && [ -x /usr/bin/unity-control-center ]; then
  exec /usr/bin/unity-control-center $@
else
  exec /usr/bin/gnome-control-center.real $@
fi
