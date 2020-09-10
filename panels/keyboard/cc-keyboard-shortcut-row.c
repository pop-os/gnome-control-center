/* cc-keyboard-shortcut-row.c
 *
 * Copyright (C) 2020 System76, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <glib/gi18n.h>
#include "cc-keyboard-shortcut-row.h"
#include "keyboard-shortcuts.h"

struct _CcKeyboardShortcutRow
{
  GtkListBoxRow  parent_instance;

  GtkButton     *add_shortcut_button;
  GtkLabel      *add_shortcut_button_label;
  GtkLabel      *description;
  GtkBox        *edit_keybinding_box;
  GtkButton     *reset_shortcut_button;
  GtkLabel      *reset_shortcut_button_label;
  GtkBox        *shortcut_box;

  CcKeyboardItem  *item;
  CcKeyboardManager  *manager;
  CcKeyboardShortcutEditor *shortcut_editor;
  gboolean is_custom_shortcut;
};

G_DEFINE_TYPE (CcKeyboardShortcutRow, cc_keyboard_shortcut_row, GTK_TYPE_LIST_BOX_ROW)

static void add_shortcut_cb (GtkWidget*, gpointer);
static void reset_shortcut_cb (GtkWidget*, gpointer);

static void
cc_keyboard_shortcut_row_class_init (CcKeyboardShortcutRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/keyboard/cc-keyboard-shortcut-row.ui");

  gtk_widget_class_bind_template_child (widget_class, CcKeyboardShortcutRow, add_shortcut_button);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardShortcutRow, add_shortcut_button_label);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardShortcutRow, description);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardShortcutRow, edit_keybinding_box);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardShortcutRow, reset_shortcut_button);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardShortcutRow, reset_shortcut_button_label);
  gtk_widget_class_bind_template_child (widget_class, CcKeyboardShortcutRow, shortcut_box);

  gtk_widget_class_bind_template_callback (widget_class, add_shortcut_cb);
  gtk_widget_class_bind_template_callback (widget_class, reset_shortcut_cb);
}

static void
cc_keyboard_shortcut_row_init (CcKeyboardShortcutRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
add_shortcut_cb (GtkWidget      *button,
                 gpointer        data)
{
  CcKeyboardShortcutRow *self;
  GtkPopover *popover;
  GtkWidget *relative_to;

  popover = GTK_POPOVER (gtk_widget_get_ancestor (button, GTK_TYPE_POPOVER));
  relative_to = gtk_popover_get_relative_to (popover);
  self = CC_KEYBOARD_SHORTCUT_ROW (gtk_widget_get_ancestor (relative_to, CC_TYPE_KEYBOARD_SHORTCUT_ROW));

  gtk_popover_popdown (popover);

  cc_keyboard_shortcut_editor_set_mode (self->shortcut_editor, CC_SHORTCUT_EDITOR_EDIT);
  cc_keyboard_shortcut_editor_set_item (self->shortcut_editor, self->item);
  gtk_widget_show (GTK_WIDGET (self->shortcut_editor));
}

static void
remove_shortcut_cb (GtkWidget        *button,
                    CcKeyCombo       *combo)
{
  CcKeyboardShortcutRow *self;
  GtkPopover *popover;
  GtkWidget *relative_to;

  popover = GTK_POPOVER (gtk_widget_get_ancestor (button, GTK_TYPE_POPOVER));
  relative_to = gtk_popover_get_relative_to (popover);
  self = CC_KEYBOARD_SHORTCUT_ROW (gtk_widget_get_ancestor (relative_to, CC_TYPE_KEYBOARD_SHORTCUT_ROW));

  gtk_popover_popdown (popover);

  cc_keyboard_item_remove_key_combo (self->item, combo);
}

static void
reset_shortcut_cb (GtkWidget      *button,
                   gpointer        data)
{
  CcKeyboardShortcutRow *self;
  GtkPopover *popover;
  GtkWidget *relative_to;

  popover = GTK_POPOVER (gtk_widget_get_ancestor(button, GTK_TYPE_POPOVER));
  relative_to = gtk_popover_get_relative_to (popover);
  self = CC_KEYBOARD_SHORTCUT_ROW (gtk_widget_get_ancestor (relative_to, CC_TYPE_KEYBOARD_SHORTCUT_ROW));

  gtk_popover_popdown (popover);

  if (self->is_custom_shortcut)
    cc_keyboard_manager_remove_custom_shortcut (self->manager, self->item);
  else
    cc_keyboard_manager_reset_shortcut (self->manager, self->item);
}

static void
update_bindings (CcKeyboardShortcutRow *self)
{
  GList *key_combos;
  g_autoptr(GList) shortcut_box_children = NULL, menu_children = NULL;
  CcKeyCombo *combo;
  GtkWidget *shortcut_label, *label, *button, *box;
  PangoAttrList *attrs;
  PangoWeight weight;
  gboolean is_default, has_one_binding;

  is_default = cc_keyboard_item_is_value_default (self->item);

  // Set weight to bold if value differs from default
  weight = is_default ? PANGO_WEIGHT_NORMAL : PANGO_WEIGHT_BOLD;
  attrs = pango_attr_list_new ();
  pango_attr_list_insert (attrs, pango_attr_weight_new (weight));
  gtk_label_set_attributes (self->description, attrs);
  pango_attr_list_unref (attrs);

  // Reset button only visible if shortcut changed from default
  gtk_widget_set_visible (GTK_WIDGET (self->reset_shortcut_button), !is_default || self->is_custom_shortcut);

  // Clear contents of shortcut_box
  shortcut_box_children = gtk_container_get_children (GTK_CONTAINER (self->shortcut_box));
  for (GList *l = shortcut_box_children; l != NULL; l = l->next) {
    gtk_widget_destroy (GTK_WIDGET (l->data));
  }

  // Clear contents of menu, except add shortcut and reset
  menu_children = gtk_container_get_children (GTK_CONTAINER (self->edit_keybinding_box));
  for (GList *l = menu_children; l != NULL; l = l->next) {
    if (l->data != self->add_shortcut_button && l->data != self->reset_shortcut_button)
      gtk_widget_destroy (GTK_WIDGET (l->data));
  }

  key_combos = cc_keyboard_item_get_key_combos (self->item);

  if (key_combos == NULL)
    {
      shortcut_label = g_object_new (GTK_TYPE_LABEL,
                                    "visible", 1,
                                    "xalign", 1.0,
                                    "label", _("Disabled"),
                                    NULL);
      gtk_style_context_add_class (gtk_widget_get_style_context (shortcut_label), "dim-label");
      gtk_container_add (GTK_CONTAINER (self->shortcut_box), shortcut_label);
    }

  has_one_binding = (key_combos != NULL && key_combos->next == NULL);

  // Iterate over combos that are set
  for (GList *l = key_combos; l != NULL; l = l->next) {
    g_autofree gchar *accel = NULL;

    combo = l->data;

    accel = convert_keysym_state_to_string (combo);

    // Populate shortcut_box based on item
    shortcut_label = g_object_new (GTK_TYPE_LABEL,
                                   "visible", 1,
                                   "xalign", 1.0,
                                   "label", accel,
                                   NULL);
    gtk_style_context_add_class (gtk_widget_get_style_context (shortcut_label), "dim-label");
    gtk_container_add (GTK_CONTAINER (self->shortcut_box), shortcut_label);

    if (has_one_binding && self->is_custom_shortcut)
      break;

    // Add option to remove the shortcut to menu
    button = gtk_button_new ();
    button = g_object_new (GTK_TYPE_BUTTON,
                           "visible", 1,
                           "relief", GTK_RELIEF_NONE,
                           "hexpand", 1,
                           NULL);
    g_signal_connect (button, "clicked", G_CALLBACK (remove_shortcut_cb), combo);
    gtk_container_add (GTK_CONTAINER (self->edit_keybinding_box), button);
    box = g_object_new (GTK_TYPE_BOX,
                        "visible", 1,
                        "spacing", 4,
                        "halign", GTK_ALIGN_START,
                        NULL);
    gtk_container_add (GTK_CONTAINER (button), box);
    label = g_object_new (GTK_TYPE_LABEL,
                          "visible", 1,
                          "label", _("Remove"),
                          NULL);
    gtk_container_add (GTK_CONTAINER (box), label);
    if (!has_one_binding)
      {
        shortcut_label = g_object_new (GTK_TYPE_LABEL,
                                       "visible", 1,
                                       "label", accel,
                                       NULL);
        gtk_style_context_add_class (gtk_widget_get_style_context (shortcut_label), "dim-label");
        gtk_container_add (GTK_CONTAINER (box), shortcut_label);
      }
  }
}

CcKeyboardShortcutRow *
cc_keyboard_shortcut_row_new (CcKeyboardItem *item,
                              CcKeyboardManager *manager,
                              CcKeyboardShortcutEditor *shortcut_editor)
{
  CcKeyboardShortcutRow *self;

  self = g_object_new (CC_TYPE_KEYBOARD_SHORTCUT_ROW, NULL);
  self->item = item;
  self->manager = manager;
  self->shortcut_editor = shortcut_editor;
  self->is_custom_shortcut = cc_keyboard_item_get_item_type (item) == CC_KEYBOARD_ITEM_TYPE_GSETTINGS_PATH;

  if (cc_keyboard_item_can_set_multiple (item))
    gtk_label_set_text (self->add_shortcut_button_label, _("Add another shortcut"));
  else
    gtk_label_set_text (self->add_shortcut_button_label, _("Modify shortcut"));

  if (!self->is_custom_shortcut)
    gtk_label_set_text (self->reset_shortcut_button_label, _("Reset to default"));
  else
    gtk_label_set_text (self->reset_shortcut_button_label, _("Remove"));

  gtk_label_set_text (self->description, cc_keyboard_item_get_description (item));

  g_signal_connect_object (item, "notify::key-combos", G_CALLBACK (update_bindings), self, G_CONNECT_SWAPPED);

  update_bindings(self);

  return self;
}
