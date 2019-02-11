/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "hdy-combo-row.h"

#include <glib/gi18n.h>
#include "hdy-list-box.h"

/**
 * SECTION:hdy-combo-row
 * @short_description: A #GtkListBox row used to choose from a list of items
 * @Title: HdyComboRow
 *
 * The #HdyComboRow widget allows the user to choose from a list of valid
 * choices. The row displays the selected choice. When activated, the row
 * displays a popover which allows the user to make a new choice.
 *
 * The #HdyComboRow uses the model-view pattern; the list of valid choices
 * is specified in the form of a #GListModel, and the display of the choices can
 * be adapted to the data in the model via widget creation functions.
 *
 * Since: 0.0.6
 */

/*
 * This was mostly inspired by code from the display panel from GNOME Settings.
 */

typedef struct
{
  GtkBox *current;
  GtkImage *image;
  GtkListBox *list;
  GtkPopover *popover;
  gint selected_index;

  GListModel *bound_model;
  GtkListBoxCreateWidgetFunc create_list_widget_func;
  GtkListBoxCreateWidgetFunc create_current_widget_func;
  gpointer create_widget_func_data;
} HdyComboRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyComboRow, hdy_combo_row, HDY_TYPE_ACTION_ROW)

enum {
  PROP_0,
  PROP_SELECTED_INDEX,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

typedef struct
{
  HdyComboRowGetNameFunc get_name_func;
  gpointer get_name_func_data;
  GDestroyNotify get_name_func_data_destroy;
} HdyComboRowCreateLabelData;

static GtkWidget *
create_list_label (gpointer item,
                   gpointer user_data)
{
  HdyComboRowCreateLabelData *data = user_data;
  g_autofree gchar *name = data->get_name_func (item, data->get_name_func_data);

  return g_object_new (GTK_TYPE_LABEL,
                       "ellipsize", PANGO_ELLIPSIZE_END,
                       "label", name,
                       "margin", 12,
                       "max-width-chars", 20,
                       "visible", TRUE,
                       "width-chars", 20,
                       "xalign", 0.0,
                        NULL);
}

static GtkWidget *
create_current_label (gpointer item,
                      gpointer user_data)
{
  HdyComboRowCreateLabelData *data = user_data;
  g_autofree gchar *name = NULL;

  if (data->get_name_func)
    name = data->get_name_func (item, data->get_name_func_data);

  return g_object_new (GTK_TYPE_LABEL,
                       "ellipsize", PANGO_ELLIPSIZE_END,
                       "halign", GTK_ALIGN_END,
                       "label", name,
                       "valign", GTK_ALIGN_CENTER,
                       "visible", TRUE,
                       "xalign", 0.0,
                        NULL);
}

static void
create_label_data_free (gpointer user_data)
{
  HdyComboRowCreateLabelData *data = user_data;

  if (data == NULL)
    return;

  if (data->get_name_func_data_destroy)
    data->get_name_func_data_destroy (data->get_name_func_data);
  data->get_name_func_data = NULL;
  data->get_name_func_data_destroy = NULL;

  g_free (data);
}

static void
update (HdyComboRow *self)
{
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);
  g_autoptr(GObject) item = NULL;
  GtkWidget *widget;

  gtk_container_foreach (GTK_CONTAINER (priv->current), (GtkCallback) gtk_widget_destroy, NULL);

  if (priv->bound_model == NULL || g_list_model_get_n_items (priv->bound_model) == 0) {
    gtk_widget_set_sensitive (GTK_WIDGET (self), FALSE);
    g_assert (priv->selected_index == -1);

    return;
  }

  g_assert (priv->selected_index >= 0 && priv->selected_index <= g_list_model_get_n_items (priv->bound_model));

  gtk_widget_set_sensitive (GTK_WIDGET (self), TRUE);

  item = g_list_model_get_item (priv->bound_model, priv->selected_index);
  widget = priv->create_current_widget_func (item, priv->create_widget_func_data);
  gtk_container_add (GTK_CONTAINER (priv->current), widget);
}

static void
bound_model_changed (GListModel *list,
                     guint       index,
                     guint       removed,
                     guint       added,
                     gpointer    user_data)
{
  gint new_idx;
  HdyComboRow *self = HDY_COMBO_ROW (user_data);
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  /* Selection is in front of insertion/removal point, nothing to do */
  if (priv->selected_index > 0 && priv->selected_index < index)
    return;

  if (priv->selected_index < index + removed) {
    /* The item selected item was removed (or none is selected) */
    new_idx = -1;
  } else {
    /* The item selected item was behind the insertion/removal */
    new_idx = priv->selected_index + added - removed;
  }

  /* Select the first item if none is selected. */
  if (new_idx == -1 && g_list_model_get_n_items (list) > 0)
    new_idx = 0;

  hdy_combo_row_set_selected_index (self, new_idx);
}

static void
row_activated_cb (HdyComboRow   *self,
                  GtkListBoxRow *row)
{
  hdy_combo_row_set_selected_index (self, gtk_list_box_row_get_index (row));
}

static void
hdy_combo_row_activate (HdyActionRow *row)
{
  HdyComboRow *self = HDY_COMBO_ROW (row);
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  gtk_popover_popup (priv->popover);
}

static void
destroy_model (HdyComboRow *self)
{
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  if (priv->bound_model) {
    /* Disconnect the bound model *before* releasing it. */
    g_signal_handlers_disconnect_by_func (priv->bound_model, bound_model_changed, self);

    /* Destroy the model and the user data. */
    if (priv->list)
      gtk_list_box_bind_model (priv->list, NULL, NULL, NULL, NULL);

    priv->bound_model = NULL;
    priv->create_list_widget_func = NULL;
    priv->create_current_widget_func = NULL;
    priv->create_widget_func_data = NULL;
  }
}

static void
hdy_combo_row_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  HdyComboRow *self = HDY_COMBO_ROW (object);

  switch (prop_id) {
  case PROP_SELECTED_INDEX:
    g_value_set_int (value, hdy_combo_row_get_selected_index (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_combo_row_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  HdyComboRow *self = HDY_COMBO_ROW (object);

  switch (prop_id) {
  case PROP_SELECTED_INDEX:
    hdy_combo_row_set_selected_index (self, g_value_get_int (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_combo_row_dispose (GObject *object)
{
  HdyComboRow *self = HDY_COMBO_ROW (object);

  destroy_model (self);

  G_OBJECT_CLASS (hdy_combo_row_parent_class)->dispose (object);
}

typedef struct {
  HdyComboRow *row;
  GtkCallback callback;
  gpointer callback_data;
} ForallData;

static void
for_non_internal_child (GtkWidget *widget,
                        gpointer   callback_data)
{
  ForallData *data = callback_data;
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (data->row);

  if (widget != (GtkWidget *) priv->current &&
      widget != (GtkWidget *) priv->image)
    data->callback (widget, data->callback_data);
}

static void
hdy_combo_row_forall (GtkContainer *container,
                      gboolean      include_internals,
                      GtkCallback   callback,
                      gpointer      callback_data)
{
  HdyComboRow *self = HDY_COMBO_ROW (container);
  ForallData data;

  if (include_internals) {
    GTK_CONTAINER_CLASS (hdy_combo_row_parent_class)->forall (GTK_CONTAINER (self), include_internals, callback, callback_data);

    return;
  }

  data.row = self;
  data.callback = callback;
  data.callback_data = callback_data;

  GTK_CONTAINER_CLASS (hdy_combo_row_parent_class)->forall (GTK_CONTAINER (self), include_internals, for_non_internal_child, &data);
}

static void
hdy_combo_row_class_init (HdyComboRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  HdyActionRowClass *row_class = HDY_ACTION_ROW_CLASS (klass);

  object_class->get_property = hdy_combo_row_get_property;
  object_class->set_property = hdy_combo_row_set_property;
  object_class->dispose = hdy_combo_row_dispose;

  container_class->forall = hdy_combo_row_forall;

  row_class->activate = hdy_combo_row_activate;

  /**
   * HdyComboRow:selected-index:
   *
   * The index of the selected item in its #GListModel.
   *
   * Since: 0.0.7
   */
  props[PROP_SELECTED_INDEX] =
      g_param_spec_int ("selected-index",
                        _("Selected index"),
                        _("The index of the selected item"),
                        -1, G_MAXINT, -1,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/dialer/ui/hdy-combo-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyComboRow, current);
  gtk_widget_class_bind_template_child_private (widget_class, HdyComboRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, HdyComboRow, list);
  gtk_widget_class_bind_template_child_private (widget_class, HdyComboRow, popover);
}

static void
list_init (HdyComboRow *self)
{
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);
  g_autoptr (GtkCssProvider) provider = gtk_css_provider_new ();

  /* This makes the list's background transparent. */
  gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),
                                   "list { border-style: none; background-color: transparent; }", -1, NULL);
  gtk_style_context_add_provider (gtk_widget_get_style_context (GTK_WIDGET (priv->list)),
                                  GTK_STYLE_PROVIDER (provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void
hdy_combo_row_init (HdyComboRow *self)
{
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  list_init (self);

  priv->selected_index = -1;

  gtk_list_box_set_header_func (priv->list, hdy_list_box_separator_header, NULL, NULL);
  g_signal_connect_object (priv->list, "row-activated", G_CALLBACK (gtk_widget_hide),
                           priv->popover, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->list, "row-activated", G_CALLBACK (row_activated_cb),
                           self, G_CONNECT_SWAPPED);

  update (self);
}

/**
 * hdy_combo_row_new:
 *
 * Creates a new #HdyComboRow.
 *
 * Returns: a new #HdyComboRow
 *
 * Since: 0.0.6
 */
HdyComboRow *
hdy_combo_row_new (void)
{
  return g_object_new (HDY_TYPE_COMBO_ROW, NULL);
}

/**
 * hdy_combo_row_get_model:
 * @self: a #HdyComboRow
 *
 * Gets the model bound to @self, or %NULL if none is bound.
 *
 * Returns: (transfer none) (nullable): the #GListModel bound to @self or %NULL
 *
 * Since: 0.0.6
 */
GListModel *
hdy_combo_row_get_model (HdyComboRow *self)
{
  HdyComboRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_COMBO_ROW (self), NULL);

  priv = hdy_combo_row_get_instance_private (self);

  return priv->bound_model;
}

/**
 * hdy_combo_row_bind_model:
 * @self: a #HdyComboRow
 * @model: (nullable): the #GListModel to be bound to @self
 * @create_list_widget_func: (nullable) (scope call): a function that creates
 *   widgets for items to display in the list, or %NULL in case you also passed
 *   %NULL as @model
 * @create_current_widget_func: (nullable) (scope call): a function that creates
 *   widgets for items to display as the seleted item, or %NULL in case you also
 *   passed %NULL as @model
 * @user_data: user data passed to @create_list_widget_func and
 *   @create_current_widget_func
 * @user_data_free_func: function for freeing @user_data
 *
 * Binds @model to @self.
 *
 * If @self was already bound to a model, that previous binding is destroyed.
 *
 * The contents of @self are cleared and then filled with widgets that represent
 * items from @model. @self is updated whenever @model changes. If @model is
 * %NULL, @self is left empty.
 *
 * Since: 0.0.6
 */
void
hdy_combo_row_bind_model (HdyComboRow                *self,
                          GListModel                 *model,
                          GtkListBoxCreateWidgetFunc  create_list_widget_func,
                          GtkListBoxCreateWidgetFunc  create_current_widget_func,
                          gpointer                    user_data,
                          GDestroyNotify              user_data_free_func)
{
  HdyComboRowPrivate *priv;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));
  g_return_if_fail (model == NULL || G_IS_LIST_MODEL (model));
  g_return_if_fail (model == NULL || create_list_widget_func != NULL);
  g_return_if_fail (model == NULL || create_current_widget_func != NULL);

  priv = hdy_combo_row_get_instance_private (self);

  destroy_model (self);

  gtk_container_foreach (GTK_CONTAINER (priv->current), (GtkCallback) gtk_widget_destroy, NULL);
  priv->selected_index = -1;

  if (model == NULL) {
    update (self);

    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_INDEX]);
    return;
  }

  gtk_list_box_bind_model (priv->list, model, create_list_widget_func, user_data, user_data_free_func);

  /* We don't need take a reference as the list box holds one for us. */
  priv->bound_model = model;
  priv->create_list_widget_func = create_list_widget_func;
  priv->create_current_widget_func = create_current_widget_func;
  priv->create_widget_func_data = user_data;

  g_signal_connect (priv->bound_model, "items-changed", G_CALLBACK (bound_model_changed), self);

  if (g_list_model_get_n_items (priv->bound_model) > 0)
    priv->selected_index = 0;

  update (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_INDEX]);
}

/**
 * hdy_combo_row_bind_name_model:
 * @self: a #HdyComboRow
 * @model: (nullable): the #GListModel to be bound to @self
 * @get_name_func: (nullable): a function that creates names for items, or %NULL
 *   in case you also passed %NULL as @model
 * @user_data: user data passed to @get_name_func
 * @user_data_free_func: function for freeing @user_data
 *
 * Binds @model to @self.
 *
 * If @self was already bound to a model, that previous binding is destroyed.
 *
 * The contents of @self are cleared and then filled with widgets that represent
 * items from @model. @self is updated whenever @model changes. If @model is
 * %NULL, @self is left empty.
 *
 * This is more conventient to use than hdy_combo_row_bind_model() if you want
 * to represent items of the model with names.
 *
 * Since: 0.0.6
 */
void
hdy_combo_row_bind_name_model (HdyComboRow            *self,
                               GListModel             *model,
                               HdyComboRowGetNameFunc  get_name_func,
                               gpointer                user_data,
                               GDestroyNotify          user_data_free_func)
{
  HdyComboRowCreateLabelData *data;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));
  g_return_if_fail (model == NULL || G_IS_LIST_MODEL (model));
  g_return_if_fail (model == NULL || get_name_func != NULL);

  data = g_new0 (HdyComboRowCreateLabelData, 1);
  data->get_name_func = get_name_func;
  data->get_name_func_data = user_data;
  data->get_name_func_data_destroy = user_data_free_func;

  hdy_combo_row_bind_model (self, model, create_list_label, create_current_label, data, create_label_data_free);
}

/**
 * hdy_combo_row_set_for_enum:
 * @self: a #HdyComboRow
 * @enum_type: the enumeration #GType to be bound to @self
 * @get_name_func: (nullable): a function that creates names for items, or %NULL
 *   in case you also passed %NULL as @model
 * @user_data: user data passed to @get_name_func
 * @user_data_free_func: function for freeing @user_data
 *
 * Creates a model for @enum_type and binds it to @self. The items of the model
 * will be #HdyEnumValueObject objects.
 *
 * If @self was already bound to a model, that previous binding is destroyed.
 *
 * The contents of @self are cleared and then filled with widgets that represent
 * items from @model. @self is updated whenever @model changes. If @model is
 * %NULL, @self is left empty.
 *
 * This is more conventient to use than hdy_combo_row_bind_name_model() if you
 * want to represent values of an enumeration with names.
 *
 * See hdy_enum_value_row_name().
 *
 * Since: 0.0.6
 */
void
hdy_combo_row_set_for_enum (HdyComboRow                     *self,
                            GType                            enum_type,
                            HdyComboRowGetEnumValueNameFunc  get_name_func,
                            gpointer                         user_data,
                            GDestroyNotify                   user_data_free_func)
{
  g_autoptr (GListStore) store = g_list_store_new (HDY_TYPE_ENUM_VALUE_OBJECT);
  /* g_autoptr for GEnumClass would require glib > 2.56 */
  GEnumClass *enum_class = NULL;
  gsize i;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));

  enum_class = g_type_class_ref (enum_type);
  for (i = 0; i < enum_class->n_values; i++)
    {
      g_autoptr(HdyEnumValueObject) obj = hdy_enum_value_object_new (&enum_class->values[i]);

      g_list_store_append (store, obj);
    }

  hdy_combo_row_bind_name_model (self, G_LIST_MODEL (store), (HdyComboRowGetNameFunc) get_name_func, user_data, user_data_free_func);
  g_type_class_unref (enum_class);
}

/**
 * hdy_combo_row_get_selected_index:
 * @self: a #GtkListBoxRow
 *
 * Gets the index of the selected item in its #GListModel.
 *
 * Returns: the index of the selected item, or -1 if no item is selected
 *
 * Since: 0.0.7
 */
gint
hdy_combo_row_get_selected_index (HdyComboRow *self)
{
  HdyComboRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_COMBO_ROW (self), -1);

  priv = hdy_combo_row_get_instance_private (self);

  return priv->selected_index;
}

/**
 * hdy_combo_row_set_selected_index:
 * @self: a #HdyComboRow
 * @selected_index: the index of the selected item
 *
 * Sets the index of the selected item in its #GListModel.
 *
 * Since: 0.0.7
 */
void
hdy_combo_row_set_selected_index (HdyComboRow *self,
                                  gint         selected_index)
{
  HdyComboRowPrivate *priv;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));
  g_return_if_fail (selected_index >= -1);

  priv = hdy_combo_row_get_instance_private (self);

  g_return_if_fail (selected_index >= 0 || priv->bound_model == NULL || g_list_model_get_n_items (priv->bound_model) == 0);
  g_return_if_fail (selected_index == -1 || (priv->bound_model != NULL && selected_index < g_list_model_get_n_items (priv->bound_model)));

  if (priv->selected_index == selected_index)
    return;

  priv->selected_index = selected_index;
  update (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_INDEX]);
}

/**
 * hdy_enum_value_row_name:
 * @value: the value from the enum from which to get a name
 * @user_data: (closure): unused user data
 *
 * This is a default implementation of #HdyComboRowGetEnumValueNameFunc to be
 * used with hdy_combo_row_set_for_enum(). If the enumeration has a nickname, it
 * will return it, otherwise it will return its name.
 *
 * Returns: (transfer full): a newly allocated displayable name that represents @value
 *
 * Since: 0.0.6
 */
gchar *
hdy_enum_value_row_name (HdyEnumValueObject *value,
                         gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  return g_strdup (hdy_enum_value_object_get_nick (value) != NULL ?
                   hdy_enum_value_object_get_nick (value) :
                   hdy_enum_value_object_get_name (value));
}
