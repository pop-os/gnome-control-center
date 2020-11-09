#include "cc-charge-threshold-dialog.h"
#include "cc-charge-threshold-row.h"
#include "list-box-helper.h"

ChargeProfile charge_profile_from_thresholds (guchar start, guchar end) {
  if (start == 50 && end == 60)
    return CHARGE_PROFILE_MAX_LIFESPAN;
  else if (start == 85 && end == 90)
    return CHARGE_PROFILE_BALANCED;
  else if (start == 96 && end == 100)
    return CHARGE_PROFILE_FULL_CHARGE;
  else
    return CHARGE_PROFILE_UNDEFINED;
}

void charge_profile_get_thresholds (ChargeProfile profile, guchar *start, guchar *end) {
  switch (profile) {
    case CHARGE_PROFILE_MAX_LIFESPAN:
      *start = 50;
      *end = 60;
      break;
    case CHARGE_PROFILE_BALANCED:
      *start = 85;
      *end = 90;
      break;
    case CHARGE_PROFILE_FULL_CHARGE:
    default:
      *start = 96;
      *end = 100;
      break;
  }
}

gchar *charge_profile_title_from_thresholds (guchar start, guchar end) {
  switch (charge_profile_from_thresholds (start, end)) {
    case CHARGE_PROFILE_MAX_LIFESPAN:
      return g_strdup ("Max lifespan");
    case CHARGE_PROFILE_BALANCED:
      return g_strdup ("Balanced");
    case CHARGE_PROFILE_FULL_CHARGE:
      return g_strdup ("Full Charge");
    default:
      return g_strdup_printf ("Custom (%d%% - %d%%)", start, end);
  }
}

struct _CcChargeThresholdDialog {
  GtkDialog parent_instance;

  S76PowerDaemon *power_proxy;
  GtkScale *scale;
  GtkListBox *listbox;
  GtkListBoxRow *max_lifespan_row;
  GtkListBoxRow *balanced_row;
  GtkListBoxRow *full_charge_row;
  GtkRadioButton *max_lifespan_radio;
  GtkRadioButton *balanced_radio;
  GtkRadioButton *full_charge_radio;
  GtkRadioButton *previous_radio;
  GtkRadioButton *hidden_radio;
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
  ChargeProfile profile;
  guchar start, end;
  GVariant *thresholds = NULL;
  gboolean state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio));

  if (!state)
    {
      self->previous_radio = radio;
      return;
    }

  if (radio == self->max_lifespan_radio)
    profile = CHARGE_PROFILE_MAX_LIFESPAN;
  else if (radio == self->balanced_radio)
    profile = CHARGE_PROFILE_BALANCED;
  else if (radio == self->full_charge_radio)
    profile = CHARGE_PROFILE_FULL_CHARGE;
  else
    return;

  charge_profile_get_thresholds (profile, &start, &end);
  thresholds = g_variant_new ("(yy)", start, end);

  s76_power_daemon_call_set_charge_thresholds (self->power_proxy, thresholds, NULL, set_charge_thresholds_ready, self);
}

static void
row_activated_cb (CcChargeThresholdDialog *self, GtkListBoxRow *row)
{
  if (row == self->max_lifespan_row)
    gtk_button_clicked (GTK_BUTTON (self->max_lifespan_radio));
  else if (row == self->balanced_row)
    gtk_button_clicked (GTK_BUTTON (self->balanced_radio));
  else if (row == self->full_charge_row)
    gtk_button_clicked (GTK_BUTTON (self->full_charge_radio));
}

static void
cc_charge_threshold_dialog_class_init (CcChargeThresholdDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_type_ensure (CC_TYPE_CHARGE_THRESHOLD_ROW);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/power/cc-charge-threshold-dialog.ui");

  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, listbox);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, max_lifespan_row);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, balanced_row);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, full_charge_row);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, max_lifespan_radio);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, balanced_radio);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdDialog, full_charge_radio);
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
cc_charge_threshold_dialog_new (S76PowerDaemon *power_proxy, ChargeProfile profile)
{
  GtkRadioButton *radio = NULL;
  CcChargeThresholdDialog *dialog = g_object_new (CC_TYPE_CHARGE_THRESHOLD_DIALOG,
                                                  "use-header-bar", 1,
                                                  NULL);
  dialog->power_proxy = power_proxy;

  switch (profile) {
    case CHARGE_PROFILE_MAX_LIFESPAN:
      radio = dialog->max_lifespan_radio;
      break;
    case CHARGE_PROFILE_BALANCED:
      radio = dialog->balanced_radio;
      break;
    case CHARGE_PROFILE_FULL_CHARGE:
      radio = dialog->full_charge_radio;
      break;
    default:
      break;
  }

  if (radio)
    {
      g_signal_handlers_block_by_func (radio, radio_toggled_cb, dialog);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
      g_signal_handlers_unblock_by_func (radio, radio_toggled_cb, dialog);
    }
  else
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->hidden_radio), TRUE);
    }

  return dialog;
}
