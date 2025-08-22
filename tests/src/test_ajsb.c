// SPDX-FileCopyrightText: 2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-License-Identifier: Apache-2.0

#include "a-json-schema-builder-library/ajsb.h"
#include "a-json-library/ajson.h"
#include "a-memory-library/aml_pool.h"
#include "the-macro-library/macro_test.h"

#include <string.h>
#include <stdbool.h>

/* Small helpers */
#define HAS(json, piece) MACRO_ASSERT_TRUE(strstr((json), (piece)) != NULL)
#define J(p, node)       ajson_stringify((p), (node))

/* ---------- 1) weather_single_object ---------- */
MACRO_TEST(ajsb_weather_single_object) {
  aml_pool_t *p = aml_pool_init(1024);

  ajson_t *root = ajsb_object(p);
  ajsb_prop(p, root, "city",       ajsb_string(p));
  ajsb_prop(p, root, "tempC",      ajsb_number(p));
  ajsb_prop(p, root, "conditions", ajsb_string(p));
  const char *req[] = {"city","tempC","conditions"};
  ajsb_required(p, root, 3, req);
  ajsb_additional_properties(p, root, false);

  const char *got = J(p, root);
  const char *expect =
    "{\"type\":\"object\",\"properties\":{"
      "\"city\":{\"type\":\"string\"},"
      "\"tempC\":{\"type\":\"number\"},"
      "\"conditions\":{\"type\":\"string\"}"
    "},\"required\":[\"city\",\"tempC\",\"conditions\"],\"additionalProperties\":false}";
  MACRO_ASSERT_STREQ(got, expect);

  aml_pool_destroy(p);
}

/* ---------- 2) buildings_array ---------- */
MACRO_TEST(ajsb_buildings_array) {
  aml_pool_t *p = aml_pool_init(2048);

  ajson_t *item = ajsb_object(p);
  ajsb_prop(p, item, "name",     ajsb_string(p));
  ajsb_prop(p, item, "height_m", ajsb_integer(p));
  ajsb_prop(p, item, "city",     ajsb_string(p));
  const char *req_item[] = {"name","height_m","city"};
  ajsb_required(p, item, 3, req_item);
  ajsb_additional_properties(p, item, false);

  ajson_t *arr = ajsb_array(p, item);
  ajsb_array_min_items(p, arr, 5);
  ajsb_array_max_items(p, arr, 5);

  ajson_t *root = ajsb_object(p);
  ajsb_prop(p, root, "buildings", arr);
  const char *req_root[] = {"buildings"};
  ajsb_required(p, root, 1, req_root);
  ajsb_additional_properties(p, root, false);

  const char *j = J(p, root);
  HAS(j, "\"buildings\":{");
  HAS(j, "\"type\":\"array\"");
  HAS(j, "\"items\":{\"type\":\"object\"");
  HAS(j, "\"name\":{\"type\":\"string\"}");
  HAS(j, "\"height_m\":{\"type\":\"integer\"}");
  HAS(j, "\"city\":{\"type\":\"string\"}");
  HAS(j, "\"minItems\":5");
  HAS(j, "\"maxItems\":5");
  HAS(j, "\"required\":[\"buildings\"]");
  HAS(j, "\"additionalProperties\":false");

  aml_pool_destroy(p);
}

/* ---------- 3) project_nested ---------- */
MACRO_TEST(ajsb_project_nested) {
  aml_pool_t *p = aml_pool_init(4096);

  ajson_t *task = ajsb_object(p);
  ajsb_prop(p, task, "task",  ajsb_string(p));
  ajsb_prop(p, task, "owner", ajsb_string(p));
  const char *req_task[] = {"task","owner"};
  ajsb_required(p, task, 2, req_task);
  ajsb_additional_properties(p, task, false);

  ajson_t *tasks = ajsb_array(p, task);
  ajson_t *phase = ajsb_object(p);
  ajsb_prop(p, phase, "name",  ajsb_string(p));
  ajsb_prop(p, phase, "tasks", tasks);
  const char *req_phase[] = {"name","tasks"};
  ajsb_required(p, phase, 2, req_phase);
  ajsb_additional_properties(p, phase, false);

  ajson_t *phases = ajsb_array(p, phase);
  ajson_t *root   = ajsb_object(p);
  ajsb_prop(p, root, "phases", phases);
  const char *req_root[] = {"phases"};
  ajsb_required(p, root, 1, req_root);
  ajsb_additional_properties(p, root, false);

  const char *j = J(p, root);
  HAS(j, "\"phases\":{\"type\":\"array\"");
  HAS(j, "\"tasks\":{\"type\":\"array\"");
  HAS(j, "\"task\":{\"type\":\"string\"}");
  HAS(j, "\"owner\":{\"type\":\"string\"}");
  HAS(j, "\"required\":[\"phases\"]");
  HAS(j, "\"additionalProperties\":false");

  aml_pool_destroy(p);
}

/* ---------- 4) search_anyof ---------- */
MACRO_TEST(ajsb_search_anyof) {
  aml_pool_t *p = aml_pool_init(2048);

  ajson_t *price = ajsb_object(p);
  ajsb_prop(p, price, "price_min", ajsb_number(p));
  ajsb_prop(p, price, "price_max", ajsb_number(p));
  const char *req_price[] = {"price_min","price_max"};
  ajsb_required(p, price, 2, req_price);
  ajsb_additional_properties(p, price, false);

  ajson_t *categories = ajsb_object(p);
  ajsb_prop(p, categories, "categories", ajsb_array(p, ajsb_string(p)));
  const char *req_cat[] = {"categories"};
  ajsb_required(p, categories, 1, req_cat);
  ajsb_additional_properties(p, categories, false);

  ajson_t *alts[2] = { price, categories };
  ajson_t *filters = ajsb_anyOf(p, 2, alts);

  ajson_t *root = ajsb_object(p);
  ajsb_prop(p, root, "term",    ajsb_string(p));
  ajsb_prop(p, root, "filters", filters);
  const char *req_root[] = {"term","filters"};
  ajsb_required(p, root, 2, req_root);
  ajsb_additional_properties(p, root, false);

  const char *j = J(p, root);
  HAS(j, "\"term\":{\"type\":\"string\"}");
  HAS(j, "\"filters\":{\"anyOf\":[");
  HAS(j, "\"price_min\":{\"type\":\"number\"}");
  HAS(j, "\"price_max\":{\"type\":\"number\"}");
  HAS(j, "\"categories\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}}");
  HAS(j, "\"required\":[\"term\",\"filters\"]");
  HAS(j, "\"additionalProperties\":false");

  aml_pool_destroy(p);
}

/* ---------- 5) options_with_null ---------- */
MACRO_TEST(ajsb_options_with_null) {
  aml_pool_t *p = aml_pool_init(1024);

  ajson_t *sort_enum = ajsb_string(p);
  const char *vals[] = {"asc","desc"};
  ajsb_string_enum(p, sort_enum, 2, vals);

  ajson_t *null_t = ajsb_null(p);
  ajson_t *alts[2] = { sort_enum, null_t };
  ajson_t *sort_any = ajsb_anyOf(p, 2, alts);

  ajson_t *root = ajsb_object(p);
  ajsb_prop(p, root, "term", ajsb_string(p));
  ajsb_prop(p, root, "sort", sort_any);
  const char *req_root[] = {"term","sort"};
  ajsb_required(p, root, 2, req_root);
  ajsb_additional_properties(p, root, false);

  const char *j = J(p, root);
  HAS(j, "\"term\":{\"type\":\"string\"}");
  HAS(j, "\"sort\":{\"anyOf\":[");
  HAS(j, "\"enum\":[\"asc\",\"desc\"]");
  HAS(j, "{\"type\":\"null\"}");
  HAS(j, "\"required\":[\"term\",\"sort\"]");
  HAS(j, "\"additionalProperties\":false");

  aml_pool_destroy(p);
}

/* ---------- 6) ui_tree_recursive ---------- */
MACRO_TEST(ajsb_ui_tree_recursive) {
  aml_pool_t *p = aml_pool_init(8192);

  ajson_t *node = ajsb_object(p);

  ajson_t *type_str = ajsb_string(p);
  const char *types[] = {"div","button","header","section","field","form"};
  ajsb_string_enum(p, type_str, 6, types);

  ajson_t *attr = ajsb_object(p);
  ajsb_prop(p, attr, "name",  ajsb_string(p));
  ajsb_prop(p, attr, "value", ajsb_string(p));
  const char *req_attr[] = {"name","value"};
  ajsb_required(p, attr, 2, req_attr);
  ajsb_additional_properties(p, attr, false);

  ajson_t *child_ref = ajsb_ref(p, "#/$defs/node");
  ajson_t *children  = ajsb_array(p, child_ref);

  ajsb_prop(p, node, "type",       type_str);
  ajsb_prop(p, node, "label",      ajsb_string(p));
  ajsb_prop(p, node, "children",   children);
  ajsb_prop(p, node, "attributes", ajsb_array(p, attr));
  const char *req_node[] = {"type","label","children","attributes"};
  ajsb_required(p, node, 4, req_node);
  ajsb_additional_properties(p, node, false);

  ajson_t *root = ajsb_object(p);
  ajsb_defs_add(p, root, "node", node);
  ajsb_prop(p, root, "root", ajsb_ref(p, "#/$defs/node"));
  const char *req_root[] = {"root"};
  ajsb_required(p, root, 1, req_root);
  ajsb_additional_properties(p, root, false);

  const char *j = J(p, root);
  HAS(j, "\"$defs\":{");
  HAS(j, "\"node\":{");
  HAS(j, "\"enum\":[\"div\",\"button\",\"header\",\"section\",\"field\",\"form\"]");
  HAS(j, "\"children\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/$defs/node\"}}");
  HAS(j, "\"attributes\":{\"type\":\"array\"");
  HAS(j, "\"name\":{\"type\":\"string\"}");
  HAS(j, "\"value\":{\"type\":\"string\"}");
  HAS(j, "\"root\":{\"$ref\":\"#/$defs/node\"}");
  HAS(j, "\"required\":[\"root\"]");
  HAS(j, "\"additionalProperties\":false");

  aml_pool_destroy(p);
}

/* ---------- 7) format_block_weather ---------- */
MACRO_TEST(ajsb_format_block_weather) {
  aml_pool_t *p = aml_pool_init(2048);

  ajson_t *schema = ajsb_object(p);
  ajsb_prop(p, schema, "city",       ajsb_string(p));
  ajsb_prop(p, schema, "tempC",      ajsb_number(p));
  ajsb_prop(p, schema, "conditions", ajsb_string(p));
  const char *req[] = {"city","tempC","conditions"};
  ajsb_required(p, schema, 3, req);
  ajsb_additional_properties(p, schema, false);

  ajson_t *fmt = ajsono(p);
  ajsono_set(fmt, "type",   ajson_str(p, "json_schema"), false);
  ajsono_set(fmt, "name",   ajson_str(p, "weather"),     false);
  ajsono_set(fmt, "schema", schema,                      false);
  ajsono_set(fmt, "strict", ajson_true(p),               false);

  const char *js = J(p, schema);
  HAS(js, "\"type\":\"object\"");
  HAS(js, "\"required\":[\"city\",\"tempC\",\"conditions\"]");
  HAS(js, "\"additionalProperties\":false");

  const char *jf = J(p, fmt);
  HAS(jf, "\"type\":\"json_schema\"");
  HAS(jf, "\"name\":\"weather\"");
  HAS(jf, "\"strict\":true");
  HAS(jf, "\"schema\":{\"type\":\"object\"");

  aml_pool_destroy(p);
}

/* ---------- 8) validation_showcase ---------- */
MACRO_TEST(ajsb_validation_showcase) {
  aml_pool_t *p = aml_pool_init(1024);

  ajson_t *array_root = ajsb_array(p, ajsb_string(p));
  const char *ja = J(p, array_root);
  MACRO_ASSERT_STREQ(ja, "{\"type\":\"array\",\"items\":{\"type\":\"string\"}}");

  ajson_t *obj_root = ajsb_object(p);
  ajsb_prop(p, obj_root, "x", ajsb_integer(p));
  const char *req[] = {"x"};
  ajsb_required(p, obj_root, 1, req);
  ajsb_additional_properties(p, obj_root, false);
  const char *jb = J(p, obj_root);
  HAS(jb, "\"x\":{\"type\":\"integer\"}");
  HAS(jb, "\"required\":[\"x\"]");
  HAS(jb, "\"additionalProperties\":false");

  aml_pool_destroy(p);
}

/* ---------- 9) demo_dynamic_refs ---------- */
MACRO_TEST(ajsb_demo_dynamic_refs) {
  aml_pool_t *p = aml_pool_init(4096);

  ajson_t *root = ajsb_object(p);
  ajsb_set_schema(p, root, "https://json-schema.org/draft/2020-12/schema");
  ajsb_set_id(p, root, "https://example.com/schemas/ui.json");

  ajson_t *node = ajsb_object(p);
  ajsb_dynamic_anchor(p, node, "Node");
  ajsb_prop(p, node, "label", ajsb_string(p));
  const char *req[] = {"label"};
  ajsb_required(p, node, 1, req);
  ajsb_additional_properties(p, node, false);

  ajsb_prop(p, node, "children", ajsb_array(p, ajsb_dynamic_ref(p, "#Node")));

  ajsb_defs_set(p, root, "node", node);
  ajsb_prop(p, root, "root", ajsb_ref(p, "#/$defs/node"));
  const char *req_root[] = {"root"};
  ajsb_required(p, root, 1, req_root);

  const char *j = J(p, root);
  HAS(j, "\"$schema\":\"https://json-schema.org/draft/2020-12/schema\"");
  HAS(j, "\"$id\":\"https://example.com/schemas/ui.json\"");
  HAS(j, "\"$dynamicAnchor\":\"Node\"");
  HAS(j, "\"children\":{\"type\":\"array\",\"items\":{\"$dynamicRef\":\"#Node\"}}");
  HAS(j, "\"$defs\":{");
  HAS(j, "\"root\":{\"$ref\":\"#/$defs/node\"}");
  HAS(j, "\"required\":[\"root\"]");

  aml_pool_destroy(p);
}

/* ---------- Runner ---------- */
int main(void) {
  macro_test_case tests[64];
  size_t test_count = 0;

  MACRO_ADD(tests, ajsb_weather_single_object);
  MACRO_ADD(tests, ajsb_buildings_array);
  MACRO_ADD(tests, ajsb_project_nested);
  MACRO_ADD(tests, ajsb_search_anyof);
  MACRO_ADD(tests, ajsb_options_with_null);
  MACRO_ADD(tests, ajsb_ui_tree_recursive);
  MACRO_ADD(tests, ajsb_format_block_weather);
  MACRO_ADD(tests, ajsb_validation_showcase);
  MACRO_ADD(tests, ajsb_demo_dynamic_refs);

  macro_run_all("a-json-schema-builder/ajsb_examples", tests, test_count);
  return 0;
}
