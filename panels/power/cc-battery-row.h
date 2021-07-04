/* cc-brightness-scale.h
 *
 * Copyright (C) 2020 System76, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>
#include <libupower-glib/upower.h>

G_BEGIN_DECLS

#define CC_TYPE_BATTERY_ROW (cc_battery_row_get_type())
G_DECLARE_FINAL_TYPE (CcBatteryRow, cc_battery_row, CC, BATTERY_ROW, GtkListBoxRow)

CcBatteryRow* cc_battery_row_new                    (UpDevice *device,
                                                     gboolean  primary);

void          cc_battery_row_set_level_sizegroup     (CcBatteryRow *self,
                                                      GtkSizeGroup *sizegroup);

void          cc_battery_row_set_row_sizegroup       (CcBatteryRow *self,
                                                      GtkSizeGroup *sizegroup);

void          cc_battery_row_set_charge_sizegroup    (CcBatteryRow *self,
                                                      GtkSizeGroup *sizegroup);

void          cc_battery_row_set_battery_sizegroup   (CcBatteryRow *self,
                                                      GtkSizeGroup *sizegroup);

gboolean      cc_battery_row_get_primary             (CcBatteryRow *self);
UpDeviceKind  cc_battery_row_get_kind                (CcBatteryRow *self);

G_END_DECLS