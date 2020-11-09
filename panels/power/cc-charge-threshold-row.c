#include "cc-charge-threshold-row.h"

struct _CcChargeThresholdRow {
  GtkListBoxRow parent_instance;
  GtkRadioButton *radio;
  char *title;
  char *description;
};

G_DEFINE_TYPE (CcChargeThresholdRow, cc_charge_threshold_row, GTK_TYPE_LIST_BOX_ROW)

enum
{
  PROP_0,
  PROP_TITLE,
  PROP_DESCRIPTION,
  PROP_LAST
};

static void
finalize (GObject *object)
{
  CcChargeThresholdRow *self = CC_CHARGE_THRESHOLD_ROW (object);
  g_free (self->title);
  g_free (self->description);
}

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
  CcChargeThresholdRow *self = CC_CHARGE_THRESHOLD_ROW (object);

  switch (prop_id) {
  case PROP_TITLE:
    self->title = g_value_dup_string (value);
    break;
  case PROP_DESCRIPTION:
    self->description = g_value_dup_string (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  CcChargeThresholdRow *self = CC_CHARGE_THRESHOLD_ROW (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, self->title);
    break;
  case PROP_DESCRIPTION:
    g_value_set_string (value, self->description);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
cc_charge_threshold_row_class_init (CcChargeThresholdRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->finalize = finalize;

  g_object_class_install_property (object_class,
                                   PROP_TITLE,
                                   g_param_spec_string ("title",
                                                        "title",
                                                        "title",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_DESCRIPTION,
                                   g_param_spec_string ("description",
                                                        "description",
                                                        "description",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/power/cc-charge-threshold-row.ui");

  gtk_widget_class_bind_template_child_internal (widget_class, CcChargeThresholdRow, radio);
}

static void
cc_charge_threshold_row_init (CcChargeThresholdRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
