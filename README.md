# A JSON Schema Builder Library

Tiny C helpers for **programmatically building JSON Schema documents** using [`a-json-library`](https://github.com/) values and an `aml_pool_t` memory pool.

* Focus: composing schemas (objects, arrays, strings, numbers, refs, combinators) with a small, null‑safe API.
* Keywords covered include common JSON Schema fields and 2020‑12 style `$defs`, `$anchor`, and `$dynamic*` helpers.

---

## Requirements

* C99 or later
* \[`a-json-library`]\(header includes `a-json-library/ajson.h`)
* \[`a-memory-library`]\(header includes `a-memory-library/aml_pool.h`)

---

## Build

```bash
# Compile the library object
cc -std=c99 -Iinclude -c src/ajsb.c

# Link it with your app plus the ajson/aml libraries (names vary by setup)
cc app.c ajsb.o -o app $(pkg-config --libs ...)   # adjust as needed
```

CMake (minimal):

```cmake
add_library(ajsb src/ajsb.c)
target_include_directories(ajsb PUBLIC include)
target_link_libraries(ajsb PUBLIC ajson aml) # adjust to your lib names
```

---

## Quick start

```c
#include "a-json-schema-builder-library/ajsb.h"

/* Assume you have an aml_pool_t* p from your allocator setup. */

ajson_t *user = ajsb_object(p);
ajsb_set_schema(p, user, "https://json-schema.org/draft/2020-12/schema");

/* properties.email: string, format email */
ajson_t *email = ajsb_string(p);
ajsb_string_format(p, email, "email");
ajsb_prop(p, user, "email", email);

/* properties.age: integer, minimum 0 (inclusive) */
ajson_t *age = ajsb_integer(p);
ajsb_number_min(p, age, 0, /*exclusive=*/false);
ajsb_prop(p, user, "age", age);

/* properties.roles: array of enum strings, minItems=1, uniqueItems=true */
const char *role_vals[] = {"admin","staff","user"};
ajson_t *role = ajsb_string(p);
ajsb_string_enum(p, role, 3, role_vals);

ajson_t *roles = ajsb_array(p, role);
ajsb_array_min_items(p, roles, 1);
ajsb_array_unique(p, roles, true);
ajsb_prop(p, user, "roles", roles);

/* $defs + $ref for a UUID-ish id */
ajson_t *uuid = ajsb_string(p);
ajsb_string_pattern(p, uuid, "^[0-9a-fA-F-]{36}$");
ajsb_defs_add(p, user, "uuid", uuid);
ajsb_prop(p, user, "id", ajsb_ref(p, "#/$defs/uuid"));

/* required + disallow additionalProperties */
const char *req[] = {"email","age"};
ajsb_required(p, user, 2, req);
ajsb_additional_properties(p, user, false);

/* serialize */
puts(ajsb_stringify(p, user));
```

Output (pretty-printed for readability):

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "type": "object",
  "properties": {
    "email": { "type": "string", "format": "email" },
    "age": { "type": "integer", "minimum": 0 },
    "roles": {
      "type": "array",
      "items": { "type": "string", "enum": ["admin","staff","user"] },
      "minItems": 1,
      "uniqueItems": true
    },
    "id": { "$ref": "#/$defs/uuid" }
  },
  "required": ["email","age"],
  "additionalProperties": false,
  "$defs": {
    "uuid": { "type": "string", "pattern": "^[0-9a-fA-F-]{36}$" }
  }
}
```

---

## API (brief)

### Primitives

```c
ajson_t *ajsb_object(aml_pool_t *p);
ajson_t *ajsb_array(aml_pool_t *p, ajson_t *items_schema);
ajson_t *ajsb_string(aml_pool_t *p);
ajson_t *ajsb_number(aml_pool_t *p);
ajson_t *ajsb_integer(aml_pool_t *p);
ajson_t *ajsb_boolean(aml_pool_t *p);
ajson_t *ajsb_null(aml_pool_t *p);
ajson_t *ajsb_ref(aml_pool_t *p, const char *ref);
```

### Object helpers

```c
void ajsb_prop(aml_pool_t *p, ajson_t *obj, const char *name, ajson_t *schema);
void ajsb_required(aml_pool_t *p, ajson_t *obj, size_t n, const char *const *names);
void ajsb_additional_properties(aml_pool_t *p, ajson_t *obj, bool allowed);
void ajsb_defs_add(aml_pool_t *p, ajson_t *root_obj, const char *name, ajson_t *schema);
```

### String helpers

```c
void ajsb_string_format (aml_pool_t *p, ajson_t *str_schema, const char *format);
void ajsb_string_pattern(aml_pool_t *p, ajson_t *str_schema, const char *regex);
void ajsb_string_enum   (aml_pool_t *p, ajson_t *str_schema, size_t n, const char *const *values);
```

### Number / Integer helpers

```c
void ajsb_number_min(aml_pool_t *p, ajson_t *num_schema, double min, bool exclusive);
void ajsb_number_max(aml_pool_t *p, ajson_t *num_schema, double max, bool exclusive);
```

### Array helpers

```c
void ajsb_array_min_items(aml_pool_t *p, ajson_t *arr_schema, int min_items);
void ajsb_array_max_items(aml_pool_t *p, ajson_t *arr_schema, int max_items);
void ajsb_array_unique   (aml_pool_t *p, ajson_t *arr_schema, bool on);
```

### Combinators

```c
ajson_t *ajsb_anyOf(aml_pool_t *p, size_t n, ajson_t *const *schemas);
ajson_t *ajsb_oneOf(aml_pool_t *p, size_t n, ajson_t *const *schemas);
ajson_t *ajsb_allOf(aml_pool_t *p, size_t n, ajson_t *const *schemas);
```

### IDs, anchors, defs

```c
ajson_t *ajsb_defs_ensure(aml_pool_t *p, ajson_t *root_obj);
void ajsb_defs_set(aml_pool_t *p, ajson_t *root_obj, const char *name, ajson_t *schema);
void ajsb_set_id(aml_pool_t *p, ajson_t *schema, const char *uri);
void ajsb_set_schema(aml_pool_t *p, ajson_t *schema, const char *uri);
void ajsb_anchor(aml_pool_t *p, ajson_t *schema, const char *name);
void ajsb_dynamic_anchor(aml_pool_t *p, ajson_t *schema, const char *name);
ajson_t *ajsb_dynamic_ref(aml_pool_t *p, const char *ref);
```

### Utility

```c
const char *ajsb_stringify(aml_pool_t *p, ajson_t *schema); /* wraps ajson_stringify */
```

> **Notes**
>
> * All functions are defensive: null/empty inputs are ignored where sensible.
> * Builders return `ajson_t*` nodes owned by `aml_pool_t`. Free the pool to free everything.

---

## License

Apache-2.0 © 2025 Andy Curtis (`SPDX-License-Identifier: Apache-2.0`).
