#include <catch2/catch_test_macros.hpp>
#include <slim/common/http/headers.h>

TEST_CASE("Headers::set and Headers::get set overwrites existing key", "[headers]") {
    slim::common::http::Headers headers;
    headers.set("Content-Type", "text/plain");
    REQUIRE(headers.has("Content-Type"));
    REQUIRE(headers.get("content-type") == "text/plain");
    headers.set("Content-Type", "application/json");
    REQUIRE(headers.get("Content-Type") == "application/json");

}

TEST_CASE("Headers::has and Headers::get get returns falsy for missing key", "[headers]") {
    slim::common::http::Headers headers;
    REQUIRE_FALSE(headers.has("X-Missing"));
    REQUIRE_FALSE(headers.get("X-Missing"));
}

TEST_CASE("Headers::set and Headers::get set returns truthy on valid key", "[headers]") {
    slim::common::http::Headers headers;
    REQUIRE(headers.set("X-Custom", "value"));
    REQUIRE(headers.get("X-Custom") == "value");
}

TEST_CASE("Headers::append and Headers::get", "[headers]") {
    slim::common::http::Headers headers;
    REQUIRE(headers.append("X-Custom", "value"));
    REQUIRE(headers.get("X-Custom") == "value");
    REQUIRE(headers.append("X-Custom", "value"));
    REQUIRE(headers.get("X-Custom") == "value, value");
}