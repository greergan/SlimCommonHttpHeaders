#include <catch2/catch_test_macros.hpp>
#include <slim/common/http/headers.h>

TEST_CASE("Headers::entries", "[headers]") {
    SECTION("empty on construction") {
        slim::common::http::Headers headers;
        REQUIRE(headers.entries().empty());
    }

    SECTION("reflects all set keys") {
        slim::common::http::Headers headers;
        headers.set("Content-Type", "application/json");
        headers.set("X-Request-Id", "abc123");
        auto& entries = headers.entries();
        REQUIRE(entries.size() == 2);
        REQUIRE(entries.at("content-type") == "application/json");
        REQUIRE(entries.at("x-request-id") == "abc123");
    }

    SECTION("overwrite does not grow entry count") {
        slim::common::http::Headers headers;
        headers.set("Content-Type", "text/plain");
        headers.set("Content-Type", "application/json");
        REQUIRE(headers.entries().size() == 1);
    }

    SECTION("returns a live reference") {
        slim::common::http::Headers headers;
        auto& e = headers.entries();
        REQUIRE(e.empty());
        headers.set("X-Foo", "bar");
        REQUIRE(e.size() == 1);
    }
}