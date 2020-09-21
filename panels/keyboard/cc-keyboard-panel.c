/* cc-keyboard-panel.c
 *
 * Copyright (C) 2010 Intel, Inc
 * Copyright (C) 2016 Endless, Inc
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
 * Author: Thomas Wood <thomas.wood@intel.com>
 *         Georges Basile Stavracas Neto <gbsneto@gnome.org>
 *         Ian Douglas Scott <idscott@system76.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <glib/gi18n.h>

#include "list-box-helper.h"
#include "cc-xkb-modifier-dialog.h"
#include "cc-keyboard-item.h"
#include "cc-keyboard-option.h"
#include "cc-keyboard-panel.h"
#include "cc-keyboard-resources.h"
#include "cc-keyboard-shortcut-dialog.h"
#include "cc-input-list-box.h"

#include "keyboard-shortcuts.h"

struct _CcKeyboardPanel
{
  CcPanel             parent_instance;

  GtkListBox          *input_source_list;

  GtkRadioButton      *per_window_source;
  GtkRadioButton      *same_source;
  GtkLabel            *value_input_switch;
  GSettings           *keybindings_settings;

  /* "Type Special Characters" section */
  CcXkbModifierDialog *alt_chars_dialog;
  CcXkbModifierDialog *compose_dialog;
  GSettings           *input_source_settings;
  GtkListBox          *special_chars_list;
  GtkListBoxRow       *alt_chars_row;
  GtkListBoxRow       *compose_row;
  GtkWidget           *value_alternate_chars;
  GtkWidget           *value_compose;

  GtkListBoxRow       *common_shortcuts_row;
};

CC_PANEL_REGISTER (CcKeyboardPanel, cc_keyboard_panel)

enum {
  PROP_0,
  PROP_PARAMETERS
};

static const XkbModifier LV3_MODIFIER = {
  "lv3:",
  N_("Alternate Characters Key"),
  N_("The alternate characters key can be used to enter additional characters. These are sometimes printed as a third-option on your keyboard."),
  (XkbOption[]){
    { NC_("keyboard key", "Left Alt"),    "lv3:lalt_switch" },
    { NC_("keyboard key", "Right Alt"),   "lv3:ralt_switch" },
    { NC_("keyboard key", "Left Super"),  "lv3:lwin_switch" },
    { NC_("keyboard key", "Right Super"), "lv3:rwin_switch" },
    { NC_("keyboard key", "Menu key"),    "lv3:menu_switch" },
    { NC_("keyboard key", "Right Ctrl"),  "lv3:switch" },
    { NULL,                               NULL }
  },
  NULL,
};

static const XkbModifier COMPOSE_MODIFIER = {
  "compose:",
  N_("Compose Key"),
  N_("The compose key allows a wide variety of characters to be entered. To use it, press compose then a sequence of characters. "
     " For example, compose key followed by <b>C</b> and <b>o</b> will enter <b>©</b>, "
     "<b>a</b> followed by <b>'</b> will enter <b>á</b>."),
  (XkbOption[]){
    { NC_("keyboard key", "Left Alt"),     "compose:lalt" },
    { NC_("keyboard key", "Right Alt"),    "compose:ralt" },
    { NC_("keyboard key", "Left Super"),   "compose:lwin" },
    { NC_("keyboard key", "Right Super"),  "compose:rwin" },
    { NC_("keyboard key", "Menu key"),     "compose:menu" },
    { NC_("keyboard key", "Right Ctrl"),   "compose:rctrl" },
    { NC_("keyboard key", "Caps Lock"),    "compose:caps" },
    { NC_("keyboard key", "Scroll Lock"),  "compose:sclk" },
    { NC_("keyboard key", "Print Screen"), "compose:prsc" },
    { NULL,                                NULL }
  },
  NULL,
};

static const gchar *custom_css =
".keyboard-panel-radio-button {"
"    padding-left: 6px;"
"    padding-right: 12px;"
"    padding-top: 12px;"
"    padding-bottom: 12px;"
"}";

static void
special_chars_activated (GtkWidget       *button,
                         GtkListBoxRow   *row,
                         CcKeyboardPanel *self)
{
  GtkWindow *window, *dialog;

  window = GTK_WINDOW (cc_shell_get_toplevel (cc_panel_get_shell (CC_PANEL (self))));

  if (row == self->alt_chars_row)
    dialog = GTK_WINDOW (self->alt_chars_dialog);
  else if (row == self->compose_row)
    dialog = GTK_WINDOW (self->compose_dialog);
  else
    return;

  gtk_window_set_transient_for (dialog, window);
  gtk_widget_show (GTK_WIDGET (dialog));
}

static void
keyboard_shortcuts_activated (GtkWidget       *button,
                              GtkListBoxRow   *row,
                              CcKeyboardPanel *self)
{
  GtkWindow *window;
  GtkWidget *shortcut_dialog;

  if (row == self->common_shortcuts_row)
    {
      window = GTK_WINDOW (cc_shell_get_toplevel (cc_panel_get_shell (CC_PANEL (self))));

      shortcut_dialog = cc_keyboard_shortcut_dialog_new ();
      gtk_window_set_transient_for (GTK_WINDOW (shortcut_dialog), window);
      gtk_widget_show (GTK_WIDGET (shortcut_dialog));
    }
}

static void
cc_keyboard_panel_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_PARAMETERS:
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static const char *
cc_keyboard_panel_get_help_uri (CcPanel *panel)
{
  return "help:gnome-help/keyboard";
}

static void
cc_keyboard_panel_finalize (GObject *object)
{
  CcKeyboardPanel *self = CC_KEYBOARD_PANEL (object);

  g_clear_object (&self->input_source_settings);
  g_clear_object (&self->keybindings_settings);

  cc_keyboard_option_clear_all ();

  G_OBJECT_CLASS (cc_keyboard_panel_parent_class)->finalize (object);
}

static void
cc_keyboard_panel_class_init (CcKeyboardPanelClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  CcPanelClass *panel_class = CC_PANEL_CLASS (klass);

  panel_class->get_help_uri = cc_keyboard_panel_get_help_uri;

  object_class->set_property = cc_keyboard_panel_set_property;
  object_class->finalize = cc_keyboard_panel_finalize;

  g_object_class_override_property (object_class, PROP_PARAMETERS, "parameters");

  // TODO better way?
  CC_TYPE_INPUT_LIST_BOX;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/keyboard/cc-keyboard-panel.ui");

  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, input_source_list);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, per_window_source);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, same_source);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, special_chars_list);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, alt_chars_row);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, compose_row);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, value_alternate_chars);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, value_compose);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, value_input_switch);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardPanel, common_shortcuts_row);

  gtk_widget_class_bind_template_callback (widget_class, special_chars_activated);
  gtk_widget_class_bind_template_callback (widget_class, keyboard_shortcuts_activated);
}

static gboolean
translate_switch_input_source (GValue *value,
                               GVariant *variant,
                               gpointer user_data)
{
  g_autofree const gchar **strv = NULL;
  g_autofree gchar *label = NULL;
  CcKeyCombo combo = { 0 };

  strv = g_variant_get_strv (variant, NULL);

  gtk_accelerator_parse (strv[0] ? strv[0] : "", &combo.keyval, &combo.mask);
  label = convert_keysym_state_to_string (&combo);

  g_value_set_string (value, label);

  return TRUE;
}

static void
cc_keyboard_panel_init (CcKeyboardPanel *self)
{
  GtkCssProvider *provider;

  g_resources_register (cc_keyboard_get_resource ());

  gtk_widget_init_template (GTK_WIDGET (self));

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider, custom_css, -1, NULL);

  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
  g_object_unref (provider);

  gtk_list_box_set_header_func (self->input_source_list, cc_list_box_update_header_func, NULL, NULL);
  gtk_list_box_set_header_func (self->special_chars_list, cc_list_box_update_header_func, NULL, NULL);

  self->input_source_settings = g_settings_new ("org.gnome.desktop.input-sources");

  /* "Input Source Switching" section */
  g_settings_bind (self->input_source_settings, "per-window",
                   self->per_window_source, "active",
                   G_SETTINGS_BIND_DEFAULT);
  g_settings_bind (self->input_source_settings, "per-window",
                   self->same_source, "active",
                   G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_INVERT_BOOLEAN);
  self->keybindings_settings = g_settings_new ("org.gnome.desktop.wm.keybindings");
  g_settings_bind_with_mapping (self->keybindings_settings, "switch-input-source",
		                self->value_input_switch, "label",
				G_SETTINGS_BIND_GET,
				translate_switch_input_source,
				NULL, NULL, NULL);

  /* "Type Special Characters" section */
  g_settings_bind_with_mapping (self->input_source_settings,
                                "xkb-options",
                                self->value_alternate_chars,
                                "label",
                                G_SETTINGS_BIND_GET,
                                xcb_modifier_transform_binding_to_label,
                                NULL,
                                (gpointer)&LV3_MODIFIER,
                                NULL);
  g_settings_bind_with_mapping (self->input_source_settings,
                                "xkb-options",
                                self->value_compose,
                                "label",
                                G_SETTINGS_BIND_GET,
                                xcb_modifier_transform_binding_to_label,
                                NULL,
                                (gpointer)&COMPOSE_MODIFIER,
                                NULL);

  self->alt_chars_dialog = cc_xkb_modifier_dialog_new (self->input_source_settings, &LV3_MODIFIER);
  self->compose_dialog = cc_xkb_modifier_dialog_new (self->input_source_settings, &COMPOSE_MODIFIER);
}
