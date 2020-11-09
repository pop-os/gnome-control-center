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
#include "cc-system76-power-generated.h"

G_BEGIN_DECLS

typedef enum {
  CHARGE_PROFILE_MAX_LIFESPAN,
  CHARGE_PROFILE_BALANCED,
  CHARGE_PROFILE_FULL_CHARGE,
  CHARGE_PROFILE_UNDEFINED,
} ChargeProfile;

#define CC_TYPE_CHARGE_THRESHOLD_DIALOG (cc_charge_threshold_dialog_get_type())
G_DECLARE_FINAL_TYPE (CcChargeThresholdDialog, cc_charge_threshold_dialog, CC, CHARGE_THRESHOLD_DIALOG, GtkDialog)
CcChargeThresholdDialog* cc_charge_threshold_dialog_new (S76PowerDaemon *power_proxy, ChargeProfile profile);
ChargeProfile charge_profile_from_thresholds (guchar start, guchar end);
void charge_profile_get_thresholds (ChargeProfile profile, guchar *start, guchar *end);
gchar *charge_profile_title_from_thresholds (guchar start, guchar end);

G_END_DECLS
