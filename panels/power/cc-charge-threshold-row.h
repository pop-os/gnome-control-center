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

G_BEGIN_DECLS

typedef struct {
  gchar *id;
  gchar *title;
  gchar *description;
  guchar start;
  guchar end;
} ChargeProfile;

#define CC_TYPE_CHARGE_THRESHOLD_ROW (cc_charge_threshold_row_get_type())
G_DECLARE_FINAL_TYPE (CcChargeThresholdRow, cc_charge_threshold_row, CC, CHARGE_THRESHOLD_ROW, GtkListBoxRow)
CcChargeThresholdRow* cc_charge_threshold_row_new (ChargeProfile *profile);
GtkRadioButton* cc_charge_threshold_row_get_radio (CcChargeThresholdRow *self);
ChargeProfile* cc_charge_threshold_row_get_profile (CcChargeThresholdRow *self);

ChargeProfile *charge_profiles_get (ChargeProfile **profiles, guchar start, guchar end);
ChargeProfile **charge_profiles_from_variant (GVariant *variant);
ChargeProfile **charge_profiles_from_variant (GVariant *variant);
void charge_profiles_free (ChargeProfile **profiles);

G_END_DECLS
