<a href="https://codeberg.org/greergan/SlimTS">
  <img src="https://raw.githubusercontent.com/greergan/SlimTS/master/assets/slimts_logo.png" width="75" alt="SlimTS Logo">
</a>   

# SlimCommonHttpHeaders

Acts as a validating, collection of [SlimCommonHttpHeader](https://codeberg.org/greergan/SlimCommonHttpHeader) instances.  
Encapsulates [SlimCommonHttpCookieStore](https://codeberg.org/greergan/SlimCommonHttpCookieStore).  
Used as the backing store for the [SlimTS](https://codeberg.org/greergan/SlimTS) Javascript Headers object.  
Part of the [SlimCommon](https://codeberg.org/greergan/SlimCommon) library.  
Dependency of the [SlimCommonHttpRequest](https://codeberg.org/greergan/SlimCommonHttpRequest) and [SlimCommonHttpResponse](https://codeberg.org/greergan/SlimCommonHttpResponse) micro-libraries.  
Built using [SlimLibraryPackager](https://codeberg.org/greergan/SlimLibraryPackager).  
CI/CD supplied by unified workflows provided by [SlimLibraryPackager](https://codeberg.org/greergan/SlimLibraryPackager).

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Core API](#core-api)
  - [Headers class](#headers-class)
  - [Constructors and object lifetime](#constructors-and-object-lifetime)
  - [Setters](#setters)
  - [Getters](#getters)
  - [Cookie integration](#cookie-integration)
- [Building](#building)
- [Dependencies](#dependencies)
- [Examples](#examples)

## Overview

This library provides a strict, validation-heavy collection type for managing multiple [`Header`](https://codeberg.org/greergan/SlimCommonHttpHeader) instances, with:

- Case-insensitive header name matching for lookup, replacement, and removal
- Two distinct mutation semantics — `append()` for multi-value accumulation vs `set()` for replace-in-place
- Automatic detection and parsing of `Set-Cookie` and `Cookie` headers into a dedicated [`CookieStore`](https://codeberg.org/greergan/SlimCommonHttpCookieStore)
- Shared ownership of stored headers via `std::shared_ptr<Header>`
- Status reporting via the same `HeaderStatus` enum used by [`SlimCommonHttpHeader`](https://codeberg.org/greergan/SlimCommonHttpHeader)
- Heavy use of `noexcept`

[↑ Top](#table-of-contents)

## Features

| Feature | Description |
|--------|-------------|
| Case-insensitive matching | Header names are looked up, replaced, and erased without regard to case |
| Append semantics | `append()` accumulates a value onto an existing header rather than overwriting it |
| Set semantics | `set()` replaces an existing header's value in place |
| Cookie routing | `Set-Cookie` and `Cookie` headers are transparently parsed into the backing `CookieStore` |
| Existence check | Quickly test whether a header name is present |
| Removal | Erase all non-cookie headers matching a given name |

[↑ Top](#table-of-contents)

## Core API

### Headers class

```cpp
slim::common::http::Headers headers;
```

[↑ Top](#table-of-contents)

### Constructors and object lifetime

| Form | Description |
|------|-------------|
| `Headers()` | Default constructor, produces an empty collection with a fresh, empty `CookieStore` |

[↑ Top](#table-of-contents)

### Setters

| Method | Description |
|--------|-------------|
| `HeaderStatus append(std::string_view key, std::string_view value) noexcept` | If a header named `key` already exists, appends `value` onto it; otherwise creates a new header. If `key` is `Set-Cookie` or `Cookie`, also parses `value` into the backing `CookieStore` |
| `HeaderStatus set(std::string_view key, std::string_view value) noexcept` | If a header named `key` already exists, replaces its value with `value`; otherwise creates a new header. If `key` is `Set-Cookie` or `Cookie`, also parses `value` into the backing `CookieStore` |
| `HeaderStatus erase(std::string_view key) noexcept` | Removes every header matching `key` (case-insensitive) |

[↑ Top](#table-of-contents)

### Getters

| Method | Returns |
|--------|---------|
| `std::shared_ptr<Header> get(std::string_view key) const noexcept` | The first stored header matching `key` (case-insensitive), or `nullptr` if not found |
| `bool has(std::string_view key) const noexcept` | Whether a header matching `key` is present |
| `const std::vector<std::shared_ptr<Header>>& entries() const noexcept` | The full underlying collection of stored headers, in insertion order |

[↑ Top](#table-of-contents)

### Cookie integration

| Method | Returns |
|--------|---------|
| `const std::shared_ptr<CookieStore>& get_cookies() const noexcept` | The backing `CookieStore` populated from every `Set-Cookie` / `Cookie` header seen by `append()` or `set()` |

`Set-Cookie` and `Cookie` are still tracked as ordinary headers and appear in `entries()`; the `CookieStore` is a parallel, structured view maintained alongside them. If cookie parsing fails, `append()`/`set()` return `HeaderStatus::InvalidCookie` and the header is **not** added.

[↑ Top](#table-of-contents)

## Building

This library is built using [SlimLibraryPackager](https://codeberg.org/greergan/SlimLibraryPackager). See that repository for build instructions.

[↑ Top](#table-of-contents)

## Dependencies

External package dependencies for this library are declared in the `required_packages` file at the repository root. This file is read by [SlimLibraryPackager](https://codeberg.org/greergan/SlimLibraryPackager) during the build process to resolve dependencies and install them if not present.

```
SlimCommonHttpHeader
#SlimCommonHttpCookie only required for tests
SlimCommonHttpCookie
SlimCommonHttpCookieStore
```

[↑ Top](#table-of-contents)

## Examples

```cpp
// Setting ordinary headers
slim::common::http::Headers headers;

HeaderStatus e = headers.set("Content-Type", "application/json");
if (e != HeaderStatus::OK) return e;
```

```cpp
// Appending accumulates onto an existing header rather than overwriting it
headers.append("Accept", "text/html");
headers.append("Accept", "application/json");
// -> the "Accept" header now carries both values
```

```cpp
// Set-Cookie headers are routed into the backing CookieStore automatically
HeaderStatus e = headers.append("Set-Cookie", "session=abc123; Path=/; Secure; SameSite=Strict");
if (e != HeaderStatus::OK) return e;

auto cookie = headers.get_cookies()->get("session");
if (cookie) {
    cookie->set_max_age(3600);
}
```

```cpp
// Cookie request headers behave the same way
headers.append("Cookie", "session=abc123; theme=dark; locale=en-US");
// -> headers.get_cookies() now contains "session", "theme", and "locale"
```

```cpp
// Lookup, existence checks, and removal
if (headers.has("content-type")) {
    auto h = headers.get("CONTENT-TYPE"); // case-insensitive match
}

headers.erase("Accept");
```

```cpp
// Iterating the full collection
for (const auto& h : headers.entries()) {
    // h->get_name(), h->get_value()
}
```

[↑ Top](#table-of-contents)
