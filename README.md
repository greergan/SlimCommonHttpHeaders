# SlimCommonHttpHeaders

A lightweight C++ HTTP headers container for the `Slim` ecosystem. Provides case-insensitive key storage, value appending, and [`SlimValue`](https://github.com/greergan/SlimValue) based error reporting.

---

## Features

* **Case-Insensitive Keys** — keys are normalized to lowercase on every operation
* **[`SlimValue`](https://github.com/greergan/SlimValue) Return Types** — operations return a value that is truthy on success and carries an error message on failure
* **Live Reference to Entries** — `entries()` returns a `const` reference that reflects mutations immediately
* **Append Semantics** — `append()` joins multiple values for the same key with `", "`, matching the HTTP spec

---

## API

```cpp
namespace slim::common::http {
    struct Headers {
        Headers();
        // Set or overwrite a header value.
        slim::SlimValue set(std::string_view key, std::string_view value);
        // Append a value to an existing header, or create it if absent.
        // Multiple values are joined with ", ".
        slim::SlimValue append(std::string_view key, std::string_view value);
        // Retrieve a header value. Returns falsy if the key is not present.
        slim::SlimValue get(std::string_view key) const;
        // Returns true if the key exists (case-insensitive).
        bool has(std::string_view key) const;
        // Remove a header. Returns falsy if the key did not exist.
        slim::SlimValue erase(std::string_view key);
        // Direct read-only access to the underlying map (lowercase keys).
        const std::unordered_map<std::string, std::string>& entries() const;
    };
}
```

### Error Conditions

| Operation | Error |
|-----------|-------|
| Any operation with an empty key | `"header key must not be empty"` |
| `set` / `append` with an empty value | `"header value must not be empty"` |

---

## Usage

```cpp
#include <slim/common/http/headers.h>

slim::common::http::Headers headers;

// Set and retrieve
headers.set("Content-Type", "application/json");
auto header = headers.get("content-type"); // case-insensitive
if (header == "application/json") {
    std::cout << ct.to_string() << "\n"; // "application/json"
}

// Append multiple values
headers.append("Accept", "text/html");
headers.append("Accept", "application/json");
// headers.get("accept") == "text/html, application/json"

// Check existence
if (headers.has("Content-Type")) { /* ... */ }

// Iterate all headers
for (const auto& [key, value] : headers.entries()) {
    std::cout << key << ": " << value << "\n";
}

// Error handling
auto result = headers.set("", "value");
if (!result) {
    std::cerr << result.error() << "\n"; // "header key must not be empty"
}
```

---

## Build & Install

This project is built and tested using [SlimLibraryPackager](https://github.com/greergan/SlimLibraryPackager). Refer to that project for build and test instructions.

### Test Files

| File | Coverage |
|------|----------|
| `tests/set_get.cpp` | `set`, `get`, `has`, `append` — basic get/set, overwrites, missing keys |
| `tests/entries.cpp` | `entries` — empty state, live reference, key count after mutations |

---

## Notes

* Keys are always stored and looked up in **lowercase**. `"Content-Type"` and `"content-type"` refer to the same entry.
* `entries()` returns a live `const` reference to the internal map. Mutations through `set`, `append`, or `erase` are immediately visible through a held reference.
* `append` on a key that does not yet exist behaves identically to `set`.
