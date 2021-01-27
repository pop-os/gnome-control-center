#include "cc-charge-threshold-dialog.h"
#include "list-box-helper.h"

struct _CcChargeThresholdDialog {
  GtkDialog parent_instance;

  S76PowerDaemon *power_proxy;
  GtkScale *scale;
  GtkListBox *listbox;
  GtkRadioButton *hidden_radio;
  GtkRadioButton *previous_radio;
  GtkLabel       *custom_label;
};

G_DEFINE_TYPE (CcChargeThresholdDialog, cc_charge_threshold_dialog, GTK_TYPE_DIALOG)

static void radio_toggled_cb (CcChargeThresholdDialog *self, GtkRadioButton *radio);

static void
set_charge_thresholds_ready(GObject *source_object,
                            GAsyncResult *res,
                            gpointer user_data) {
  g_autoptr(GError) error = NULL;
  CcChargeThresholdDialog *self = CC_CHARGE_THRESHOLD_DIALOG (user_data);
  GtkRadioButton *previous_radio = self->previous_radio;

  s76_power_daemon_call_set_charge_thresholds_finish (self->power_proxy, res, &error);
  if (error) {
    g_signal_handlers_block_by_func (previous_radio, radio_toggled_cb, self);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (previous_radio), TRUE);
    g_signal_handlers_unblock_by_func (previous_radio, radio_toggled_cb, self);

    if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
      g_warning ("Failed to call set-charge-thresholds: %s", error->message);
    return;
  }
}

static void
radio_toggled_cb (CcChargeThresholdDialog *self, GtkRadioButton *radio)
{
  ChargeProfile *profile;
  GVariant *thresholds = NULL;
  GtkWidget *row;
  gboolean state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio));

  if (!state)
    {
      self->previous_radio = radio;
      return;
    }

  row = gtk_widget_get_ancestor (GTK_WIDGET (radio), CC_TYPE_CHARGE_THRESHOLD_ROW);
  if (row == NULL)
    return;

  profile = cc_charge_threshold_row_get_profile (CC_CHARGE_THRESHOLD_ROW (row));
  thresholds = g_variant_new ("(yy)", profile->start, profile->end);

  s76_power_daemon_call_set_charge_thresholds (self->power_proxy, thresholds, NULL, set_charge_thresholds_ready, self);
}

static void
row_activated_cb (CcChargeThresholdDialog *self, GtkListBoxRow *row)
{
  CcChargeThresholdRow *profile_row = CC_CHARGE_THRESHOLD_ROW (row);
  GtkRadioButton *radio = cc_charge_threshold_row_get_radio (profile_row);
  gtk_button_clicked (GTK_BUTTON (radio));
}

static void
cc_charge_threshold_dialog_class_init (CcChargeThresholdDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_type_ensure (CC_TYPE_CHARGE_THRESHOLD_ROW);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/power/cc-charge-threshold-dialog.ui");

  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, custom_label);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, listbox);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, hidden_radio);

  gtk_widget_class_bind_template_callback (widget_class, radio_toggled_cb);
  gtk_widget_class_bind_template_callback (widget_class, row_activated_cb);
}

static void
cc_charge_threshold_dialog_init (CcChargeThresholdDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->power_proxy = NULL;

  gtk_list_box_set_header_func (self->listbox, cc_list_box_update_header_func, NULL, NULL);
}

CcChargeThresholdDialog*
cc_charge_threshold_dialog_new (S76PowerDaemon *power_proxy, ChargeProfile **profiles, guint start, guint end)
{
  GtkRadioButton *selected_radio = NULL;
  CcChargeThresholdDialog *dialog = g_object_new (CC_TYPE_CHARGE_THRESHOLD_DIALOG,
                                                  "use-header-bar", 1,
                                                  NULL);
  g_autofree gchar *label = NULL;
  dialog->power_proxy = power_proxy;

  for (int i = 0; profiles[i] != NULL; i++)
    {
      CcChargeThresholdRow *row = cc_charge_threshold_row_new (profiles[i]);
      gtk_container_add (GTK_CONTAINER (dialog->listbox), GTK_WIDGET (row));

      GtkRadioButton *radio = cc_charge_threshold_row_get_radio (row);
      gtk_radio_button_join_group (radio, dialog->hidden_radio);
      g_signal_connect_object (radio, "toggled", G_CALLBACK (radio_toggled_cb), dialog, G_CONNECT_SWAPPED);

      if (profiles[i]->start == start && profiles[i]->end == end)
        selected_radio = radio;
    }

  if (selected_radio)
    {
      g_signal_handlers_block_by_func (selected_radio, radio_toggled_cb, dialog);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (selected_radio), TRUE);
      g_signal_handlers_unblock_by_func (selected_radio, radio_toggled_cb, dialog);
    }
  else
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->hidden_radio), TRUE);
      label = g_strdup_printf ("Custom battery charge threshold (%d%% – %d%%) "
                               "set via command line. Selecting one of the "
                               "profiles above will override custom setting.",
                               start, end);
      gtk_label_set_text (dialog->custom_label, label);
    }

  return dialog;
}
