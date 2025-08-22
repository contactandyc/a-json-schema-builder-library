// SPDX-FileCopyrightText: 2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-License-Identifier: Apache-2.0

#ifndef A_JSON_SCHEMA_BUILDER_H
#define A_JSON_SCHEMA_BUILDER_H

#include "a-json-library/ajson.h"
#include "a-memory-library/aml_pool.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Primitives ──────────────────────────────────────────────────────────── */
ajson_t *ajsb_object  (aml_pool_t *p);                        /* { "type": "object" } */
ajson_t *ajsb_array   (aml_pool_t *p, ajson_t *items_schema); /* { "type":"array","items":<schema> } */
ajson_t *ajsb_string  (aml_pool_t *p);                        /* { "type": "string" } */
ajson_t *ajsb_number  (aml_pool_t *p);                        /* { "type": "number" } */
ajson_t *ajsb_integer (aml_pool_t *p);                        /* { "type": "integer" } */
ajson_t *ajsb_boolean (aml_pool_t *p);                        /* { "type": "boolean" } */
ajson_t *ajsb_null    (aml_pool_t *p);                        /* { "type": "null" } */
ajson_t *ajsb_ref     (aml_pool_t *p, const char *ref);       /* { "$ref": "<ref>" } */

/* ── Object helpers ──────────────────────────────────────────────────────── */
void ajsb_prop(aml_pool_t *p, ajson_t *obj, const char *name, ajson_t *schema);
void ajsb_required(aml_pool_t *p, ajson_t *obj, size_t n, const char *const *names);
void ajsb_additional_properties(aml_pool_t *p, ajson_t *obj, bool allowed);

/* $defs helpers */
void ajsb_defs_add(aml_pool_t *p, ajson_t *root_obj, const char *name, ajson_t *schema);

/* ── String helpers ─────────────────────────────────────────────────────── */
void ajsb_string_format (aml_pool_t *p, ajson_t *str_schema, const char *format); /* "email","date","time",… */
void ajsb_string_pattern(aml_pool_t *p, ajson_t *str_schema, const char *regex);
void ajsb_string_enum   (aml_pool_t *p, ajson_t *str_schema, size_t n, const char *const *values);

/* ── Number / Integer helpers ───────────────────────────────────────────── */
void ajsb_number_min(aml_pool_t *p, ajson_t *num_schema, double min, bool exclusive);
void ajsb_number_max(aml_pool_t *p, ajson_t *num_schema, double max, bool exclusive);

/* ── Array helpers ──────────────────────────────────────────────────────── */
void ajsb_array_min_items (aml_pool_t *p, ajson_t *arr_schema, int min_items);
void ajsb_array_max_items (aml_pool_t *p, ajson_t *arr_schema, int max_items);
void ajsb_array_unique    (aml_pool_t *p, ajson_t *arr_schema, bool on);

/* ── Combinators ────────────────────────────────────────────────────────── */
ajson_t *ajsb_anyOf(aml_pool_t *p, size_t n, ajson_t *const *schemas);
ajson_t *ajsb_oneOf(aml_pool_t *p, size_t n, ajson_t *const *schemas);
ajson_t *ajsb_allOf(aml_pool_t *p, size_t n, ajson_t *const *schemas);

/* ── Refs / IDs helpers (optional conveniences) ─────────────────────────── */
/* Ensure root_obj has a $defs object and return it (create if missing). */
ajson_t *ajsb_defs_ensure(aml_pool_t *p, ajson_t *root_obj);

/* Replace-or-add a definition at $defs[name] = schema. */
void ajsb_defs_set(aml_pool_t *p, ajson_t *root_obj, const char *name, ajson_t *schema);

/* Identification & anchors */
void ajsb_set_id          (aml_pool_t *p, ajson_t *schema, const char *uri);   /* sets "$id" */
void ajsb_set_schema      (aml_pool_t *p, ajson_t *schema, const char *uri);   /* sets "$schema" */
void ajsb_anchor          (aml_pool_t *p, ajson_t *schema, const char *name);  /* sets "$anchor" */
void ajsb_dynamic_anchor  (aml_pool_t *p, ajson_t *schema, const char *name);  /* sets "$dynamicAnchor" */

/* Refs */
ajson_t *ajsb_ref         (aml_pool_t *p, const char *ref);            /* { "$ref": "<ref>" } */
ajson_t *ajsb_dynamic_ref (aml_pool_t *p, const char *ref);            /* { "$dynamicRef": "<ref>" } */


/* ── Utility ────────────────────────────────────────────────────────────── */
static inline const char *ajsb_stringify(aml_pool_t *p, ajson_t *schema) {
  return ajson_stringify(p, schema);
}

#ifdef __cplusplus
}
#endif
#endif /* A_JSON_SCHEMA_BUILDER_H */
