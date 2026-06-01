#include <catch2/catch_test_macros.hpp>
#include <slim/common/http/headers.h>

TEST_CASE("Headers::set and Headers::get", "[headers]") {
	SECTION("set overwrites existing key") {
        slim::common::http::Headers headers;
        headers.set("Content-Type", "text/plain");
        REQUIRE(headers.has("Content-Type"));
		REQUIRE(headers.get("Content-Type") == "text/plain");
		headers.set("Content-Type", "application/json");
        REQUIRE(headers.get("Content-Type") == "application/json");
    }

    SECTION("get returns falsy for missing key") {
        slim::common::http::Headers headers;
        REQUIRE_FALSE(headers.has("X-Missing"));
        auto result = headers.get("X-Missing");
        REQUIRE(result == "");
    }

    SECTION("set returns truthy on valid key") {
        slim::common::http::Headers headers;
        slim::SlimValue result = headers.set("X-Custom", "value");
        REQUIRE(result);
        REQUIRE(headers.get("X-Custom") == "value");
    }
}