// SPDX-FileCopyrightText: 2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-License-Identifier: Apache-2.0

#include "a-json-schema-builder-library/ajsb.h"

/* replace-if-exists, else append. Used internally for hardcoded schema keys. */
static inline void kv_set(ajson_t *obj, const char *k, ajson_t *v) {
  ajsono_set(obj, k, v, /*copy_key=*/false);
}

/* ── Primitives ─────────────────────────────────────────────────────────── */

ajson_t *ajsb_object(aml_pool_t *p) {
  ajson_t *o = ajsono(p);
  kv_set(o, "type", ajson_str(p, "object"));
  return o;
}

ajson_t *ajsb_array(aml_pool_t *p, ajson_t *items_schema) {
  ajson_t *o = ajsono(p);
  kv_set(o, "type",  ajson_str(p, "array"));
  if (items_schema) kv_set(o, "items", items_schema);
  return o;
}

ajson_t *ajsb_string (aml_pool_t *p){ ajson_t *o=ajsono(p); kv_set(o,"type", ajson_str(p,"string"));  return o; }
ajson_t *ajsb_number (aml_pool_t *p){ ajson_t *o=ajsono(p); kv_set(o,"type", ajson_str(p,"number"));  return o; }
ajson_t *ajsb_integer(aml_pool_t *p){ ajson_t *o=ajsono(p); kv_set(o,"type", ajson_str(p,"integer")); return o; }
ajson_t *ajsb_boolean(aml_pool_t *p){ ajson_t *o=ajsono(p); kv_set(o,"type", ajson_str(p,"boolean")); return o; }
ajson_t *ajsb_null   (aml_pool_t *p){ ajson_t *o=ajsono(p); kv_set(o,"type", ajson_str(p,"null"));    return o; }

ajson_t *ajsb_ref(aml_pool_t *p, const char *ref) {
  ajson_t *o = ajsono(p);
  if (ref && *ref) kv_set(o, "$ref", ajson_str(p, ref));
  return o;
}

/* ── Object helpers ─────────────────────────────────────────────────────── */

void ajsb_prop(aml_pool_t *p, ajson_t *obj, const char *name, ajson_t *schema) {
  if (!p || !obj || !name || !*name || !schema) return;
  ajson_t *props = ajsono_scan(obj, "properties");
  if (!props || !ajson_is_object(props)) {
    props = ajsono(p);
    kv_set(obj, "properties", props);
  }
  // FIX: User-provided 'name' MUST be copied into the pool.
  ajsono_set(props, name, schema, true);
}

void ajsb_prop_required(aml_pool_t *p, ajson_t *obj, const char *name, ajson_t *schema) {
  if (!p || !obj || !name || !*name || !schema) return;
  ajsb_prop(p, obj, name, schema);

  // Auto-append to "required" array
  ajson_t *req = ajsono_scan(obj, "required");
  if (!req || !ajson_is_array(req)) {
      req = ajsona(p);
      kv_set(obj, "required", req);
  }
  ajsona_append(req, ajson_str(p, name));
}

void ajsb_required(aml_pool_t *p, ajson_t *obj, size_t n, const char *const *names) {
  if (!p || !obj) return;
  ajson_t *arr = ajsona(p);
  for (size_t i = 0; i < n; ++i) {
    if (names && names[i] && *names[i]) ajsona_append(arr, ajson_str(p, names[i]));
  }
  kv_set(obj, "required", arr);
}

void ajsb_additional_properties(aml_pool_t *p, ajson_t *obj, bool allowed) {
  if (!p || !obj) return;
  kv_set(obj, "additionalProperties", allowed ? ajson_true(p) : ajson_false(p));
}

void ajsb_defs_add(aml_pool_t *p, ajson_t *root_obj, const char *name, ajson_t *schema) {
  if (!p || !root_obj || !name || !*name || !schema) return;
  ajson_t *defs = ajsono_scan(root_obj, "$defs");
  if (!defs || !ajson_is_object(defs)) {
    defs = ajsono(p);
    kv_set(root_obj, "$defs", defs);
  }
  // FIX: User-provided 'name' MUST be copied into the pool.
  ajsono_set(defs, name, schema, true);
}

/* ── Metadata helpers ───────────────────────────────────────────────────── */
void ajsb_title(aml_pool_t *p, ajson_t *schema, const char *title) {
  if (schema && title) kv_set(schema, "title", ajson_str(p, title));
}

void ajsb_description(aml_pool_t *p, ajson_t *schema, const char *description) {
  if (schema && description) kv_set(schema, "description", ajson_str(p, description));
}

void ajsb_default_str(aml_pool_t *p, ajson_t *schema, const char *def_val) {
  if (schema && def_val) kv_set(schema, "default", ajson_str(p, def_val));
}

/* ── String helpers ─────────────────────────────────────────────────────── */

void ajsb_string_format(aml_pool_t *p, ajson_t *str_schema, const char *format) {
  if (!p || !str_schema || !format || !*format) return;
  kv_set(str_schema, "format", ajson_str(p, format));
}

void ajsb_string_pattern(aml_pool_t *p, ajson_t *str_schema, const char *regex) {
  if (!p || !str_schema || !regex || !*regex) return;
  kv_set(str_schema, "pattern", ajson_str(p, regex));
}

void ajsb_string_enum(aml_pool_t *p, ajson_t *str_schema, size_t n, const char *const *values) {
  if (!p || !str_schema) return;
  ajson_t *arr = ajsona(p);
  for (size_t i = 0; i < n; ++i)
    if (values && values[i] && *values[i]) ajsona_append(arr, ajson_str(p, values[i]));
  kv_set(str_schema, "enum", arr);
}

/* ── Number / Integer helpers ───────────────────────────────────────────── */

void ajsb_number_min(aml_pool_t *p, ajson_t *num_schema, double min, bool exclusive) {
  if (!p || !num_schema) return;
  kv_set(num_schema, exclusive ? "exclusiveMinimum" : "minimum",
         ajson_decimal_stringf(p, "%g", min));
}

void ajsb_number_max(aml_pool_t *p, ajson_t *num_schema, double max, bool exclusive) {
  if (!p || !num_schema) return;
  kv_set(num_schema, exclusive ? "exclusiveMaximum" : "maximum",
         ajson_decimal_stringf(p, "%g", max));
}

/* ── Array helpers ──────────────────────────────────────────────────────── */

void ajsb_array_min_items(aml_pool_t *p, ajson_t *arr_schema, int min_items) {
  if (!p || !arr_schema || min_items < 0) return;
  kv_set(arr_schema, "minItems", ajson_number(p, min_items));
}

void ajsb_array_max_items(aml_pool_t *p, ajson_t *arr_schema, int max_items) {
  if (!p || !arr_schema || max_items < 0) return;
  kv_set(arr_schema, "maxItems", ajson_number(p, max_items));
}

void ajsb_array_unique(aml_pool_t *p, ajson_t *arr_schema, bool on) {
  if (!p || !arr_schema) return;
  kv_set(arr_schema, "uniqueItems", on ? ajson_true(p) : ajson_false(p));
}

/* ── Combinators ────────────────────────────────────────────────────────── */

static ajson_t *combine(aml_pool_t *p, const char *kw, size_t n, ajson_t *const *schemas) {
  ajson_t *o   = ajsono(p);
  ajson_t *arr = ajsona(p);
  for (size_t i = 0; i < n; ++i)
    if (schemas && schemas[i]) ajsona_append(arr, schemas[i]);
  kv_set(o, kw, arr);
  return o;
}

ajson_t *ajsb_anyOf(aml_pool_t *p, size_t n, ajson_t *const *schemas) { return combine(p, "anyOf", n, schemas); }
ajson_t *ajsb_oneOf(aml_pool_t *p, size_t n, ajson_t *const *schemas) { return combine(p, "oneOf", n, schemas); }
ajson_t *ajsb_allOf(aml_pool_t *p, size_t n, ajson_t *const *schemas) { return combine(p, "allOf", n, schemas); }

/* ── Refs / IDs helpers (implementations) ───────────────────────────────── */

ajson_t *ajsb_defs_ensure(aml_pool_t *p, ajson_t *root_obj) {
  if (!p || !root_obj) return NULL;
  ajson_t *defs = ajsono_scan(root_obj, "$defs");
  if (!defs || !ajson_is_object(defs)) {
    defs = ajsono(p);
    kv_set(root_obj, "$defs", defs);
  }
  return defs;
}

void ajsb_defs_set(aml_pool_t *p, ajson_t *root_obj, const char *name, ajson_t *schema) {
  if (!p || !root_obj || !name || !*name || !schema) return;
  ajson_t *defs = ajsb_defs_ensure(p, root_obj);
  // FIX: User-provided 'name' MUST be copied into the pool.
  ajsono_set(defs, name, schema, /*copy_key=*/true);  /* replace-or-add */
}

void ajsb_set_id(aml_pool_t *p, ajson_t *schema, const char *uri) {
  if (!p || !schema || !uri || !*uri) return;
  kv_set(schema, "$id", ajson_str(p, uri));
}

void ajsb_set_schema(aml_pool_t *p, ajson_t *schema, const char *uri) {
  if (!p || !schema || !uri || !*uri) return;
  kv_set(schema, "$schema", ajson_str(p, uri));
}

void ajsb_anchor(aml_pool_t *p, ajson_t *schema, const char *name) {
  if (!p || !schema || !name || !*name) return;
  kv_set(schema, "$anchor", ajson_str(p, name));
}

void ajsb_dynamic_anchor(aml_pool_t *p, ajson_t *schema, const char *name) {
  if (!p || !schema || !name || !*name) return;
  kv_set(schema, "$dynamicAnchor", ajson_str(p, name));
}

ajson_t *ajsb_dynamic_ref(aml_pool_t *p, const char *ref) {
  ajson_t *o = ajsono(p);
  if (ref && *ref) kv_set(o, "$dynamicRef", ajson_str(p, ref));
  return o;
}
