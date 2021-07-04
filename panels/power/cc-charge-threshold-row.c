#include "cc-charge-threshold-row.h"

struct _CcChargeThresholdRow {
  GtkListBoxRow parent_instance;
  GtkRadioButton *radio;
  ChargeProfile *profile;
  GtkLabel *title_label;
  GtkLabel *description_label;
};

G_DEFINE_TYPE (CcChargeThresholdRow, cc_charge_threshold_row, GTK_TYPE_LIST_BOX_ROW)

ChargeProfile *charge_profiles_get (ChargeProfile **profiles, guchar start, guchar end)
{
  for (int i = 0; profiles[i] != NULL; i++)
    if (profiles[i]->start == start && profiles[i]->end == end)
      return profiles[i];
  return NULL;
}

ChargeProfile **charge_profiles_from_variant (GVariant *variant)
{
  GVariantIter iter;
  GVariant *dict;
  ChargeProfile **profiles;
  int i = 0;
  gchar *id, *title, *description;

  profiles = g_malloc ((g_variant_n_children (variant) + 1) * sizeof (ChargeProfile*));

  g_variant_iter_init (&iter, variant);
  while ((dict = g_variant_iter_next_value (&iter)))
    {
      ChargeProfile *profile = g_malloc(sizeof (ChargeProfile));

      gboolean success = TRUE;
      success = success && g_variant_lookup (dict, "id", "s", &id);
      success = success && g_variant_lookup (dict, "title", "s", &title);
      success = success && g_variant_lookup (dict, "description", "s", &description);
      success = success && g_variant_lookup (dict, "start", "y", &profile->start);
      success = success && g_variant_lookup (dict, "end", "y", &profile->end);

      if (success)
        {
          profile->id = g_strdup(id);
          profile->title = g_strdup(title);
          profile->description = g_strdup(description);
          profiles[i++] = profile;
        }
      else
        {
          g_warning ("Failed to parse charge profile");
          g_free (profile);
        }
    }
  profiles[i] = NULL;

  return profiles;
}

void charge_profiles_free (ChargeProfile **profiles)
{
  for (int i = 0; profiles[i] != NULL; i++)
    {
      g_free (profiles[i]->id);
      g_free (profiles[i]->title);
      g_free (profiles[i]->description);
      g_free (profiles[i]);
    }
  g_free (profiles);
}

static void
cc_charge_threshold_row_class_init (CcChargeThresholdRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/power/cc-charge-threshold-row.ui");
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdRow, radio);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdRow, title_label);
  gtk_widget_class_bind_template_child (widget_class, CcChargeThresholdRow, description_label);
}

CcChargeThresholdRow*
cc_charge_threshold_row_new (ChargeProfile *profile)
{
  CcChargeThresholdRow *row;

  row = g_object_new(CC_TYPE_CHARGE_THRESHOLD_ROW, NULL);
  row->profile = profile;
  gtk_label_set_text (row->title_label, profile->title);
  gtk_label_set_text (row->description_label, profile->description);

  return row;
}

static void
cc_charge_threshold_row_init (CcChargeThresholdRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkRadioButton*
cc_charge_threshold_row_get_radio (CcChargeThresholdRow *self)
{
  return self->radio;
}

ChargeProfile*
cc_charge_threshold_row_get_profile (CcChargeThresholdRow *self)
{
  return self->profile;
}
