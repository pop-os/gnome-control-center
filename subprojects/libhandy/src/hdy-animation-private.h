/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-animation.h"

G_BEGIN_DECLS

gdouble hdy_lerp (gdouble a, gdouble b, gdouble t);

G_END_DECLS
