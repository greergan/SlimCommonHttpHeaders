#include <catch2/catch_test_macros.hpp>

#include <slim/common/http/header.h>
#include <slim/common/http/headers.h>
#include <slim/common/http/cookie.h>
#include <slim/common/http/cookie/store.h>

using namespace slim::common::http;

TEST_CASE("Headers::append adds a new header when the key does not exist", "[headers][append]") {
    Headers h;

    SECTION("simple header") {
        REQUIRE(h.append("Accept", "text/html") == HeaderStatus::OK);
        REQUIRE(h.entries().size() == 1);

        auto entry = h.get("Accept");
        REQUIRE(entry != nullptr);
        CHECK(entry->get_name() == "Accept");
        REQUIRE(entry->get_value().size() == 1);
        CHECK(entry->get_value()[0] == "text/html");
    }

    SECTION("invalid name does not create an entry") {
        REQUIRE(h.append("", "text/html") == HeaderStatus::NameEmpty);
        CHECK(h.entries().empty());
    }

    SECTION("invalid value does not create an entry") {
        REQUIRE(h.append("Accept", "") == HeaderStatus::ValueEmpty);
        CHECK(h.entries().empty());
        CHECK_FALSE(h.has("Accept"));
    }
}

TEST_CASE("Headers::append on an existing key appends a value rather than replacing it", "[headers][append]") {
    Headers h;
    REQUIRE(h.append("Accept", "text/html") == HeaderStatus::OK);
    REQUIRE(h.append("Accept", "application/json") == HeaderStatus::OK);

    SECTION("no duplicate entry is created") { CHECK(h.entries().size() == 1); }

    SECTION("both values are retained, in order") {
        auto entry = h.get("Accept");
        REQUIRE(entry != nullptr);
        REQUIRE(entry->get_value().size() == 2);
        CHECK(entry->get_value()[0] == "text/html");
        CHECK(entry->get_value()[1] == "application/json");
    }
}

TEST_CASE("Headers::get and Headers::has are case-insensitive on the header name", "[headers][get][has]") {
    Headers h;
    REQUIRE(h.append("Content-Type", "application/json") == HeaderStatus::OK);

    CHECK(h.has("content-type"));
    CHECK(h.has("CONTENT-TYPE"));
    CHECK(h.get("cOnTeNt-TyPe") != nullptr);
    CHECK(h.get("cOnTeNt-TyPe")->get_name() == "Content-Type");

    CHECK_FALSE(h.has("X-Missing"));
    CHECK(h.get("X-Missing") == nullptr);
}

TEST_CASE("Headers::erase removes a header by name, case-insensitively", "[headers][erase]") {
    Headers h;
    REQUIRE(h.append("Accept", "text/html") == HeaderStatus::OK);
    REQUIRE(h.append("Vary", "Accept-Encoding") == HeaderStatus::OK);

    SECTION("erase removes the matching header only") {
        REQUIRE(h.erase("ACCEPT") == HeaderStatus::OK);
        CHECK_FALSE(h.has("Accept"));
        CHECK(h.has("Vary"));
        CHECK(h.entries().size() == 1);
    }

    SECTION("erasing a key that does not exist is a no-op and reports OK") {
        REQUIRE(h.erase("X-Missing") == HeaderStatus::OK);
        CHECK(h.entries().size() == 2);
    }
}

TEST_CASE("Headers::set creates a header when the key does not exist", "[headers][set]") {
    Headers h;
    REQUIRE(h.set("Accept-Language", "en") == HeaderStatus::OK);

    auto entry = h.get("Accept-Language");
    REQUIRE(entry != nullptr);
    REQUIRE(entry->get_value().size() == 1);
    CHECK(entry->get_value()[0] == "en");
}

TEST_CASE("Headers::set replaces all existing values on the matching header", "[headers][set]") {
    Headers h;
    REQUIRE(h.append("Accept-Language", "en") == HeaderStatus::OK);
    REQUIRE(h.append("Accept-Language", "fr") == HeaderStatus::OK);
    REQUIRE(h.get("Accept-Language")->get_value().size() == 2);

    REQUIRE(h.set("accept-language", "de") == HeaderStatus::OK);

    SECTION("only the new value remains") {
        auto entry = h.get("Accept-Language");
        REQUIRE(entry != nullptr);
        REQUIRE(entry->get_value().size() == 1);
        CHECK(entry->get_value()[0] == "de");
    }

    SECTION("no duplicate header entry is created") { CHECK(h.entries().size() == 1); }
}

TEST_CASE("Headers preserves insertion order in entries()", "[headers][entries]") {
    Headers h;
    REQUIRE(h.append("Accept", "text/html") == HeaderStatus::OK);
    REQUIRE(h.append("User-Agent", "test-agent") == HeaderStatus::OK);
    REQUIRE(h.append("Vary", "Accept-Encoding") == HeaderStatus::OK);

    const auto& entries = h.entries();
    REQUIRE(entries.size() == 3);
    CHECK(entries[0]->get_name() == "Accept");
    CHECK(entries[1]->get_name() == "User-Agent");
    CHECK(entries[2]->get_name() == "Vary");
}

TEST_CASE("Headers::get_cookies is never null", "[headers][cookies]") {
    Headers h;
    REQUIRE(h.get_cookies() != nullptr);
    CHECK(h.get_cookies()->entries().empty());
}

TEST_CASE("Headers::append('Set-Cookie', ...) populates the cookie store and the raw header", "[headers][cookies][set-cookie]") {
    Headers h;
    REQUIRE(h.append("Set-Cookie", "id=123; Path=/; Secure") == HeaderStatus::OK);

    SECTION("the cookie store gains an entry") {
        auto& cookies = h.get_cookies()->entries();
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0]->get_name() == "id");
        CHECK(cookies[0]->get_value() == "123");
        REQUIRE(cookies[0]->get_path().has_value());
        CHECK(*cookies[0]->get_path() == "/");
        CHECK(cookies[0]->get_secure());
    }

    SECTION("the raw header is also recorded") {
        auto entry = h.get("Set-Cookie");
        REQUIRE(entry != nullptr);
        REQUIRE(entry->get_value().size() == 1);
        CHECK(entry->get_value()[0] == "id=123; Path=/; Secure");
    }
}

TEST_CASE("Headers::append('Set-Cookie', ...) called twice records two cookies but one header entry with two values",
          "[headers][cookies][set-cookie]") {
    Headers h;
    REQUIRE(h.append("Set-Cookie", "id=123") == HeaderStatus::OK);
    REQUIRE(h.append("Set-Cookie", "session=abc") == HeaderStatus::OK);

    CHECK(h.entries().size() == 1);
    REQUIRE(h.get("Set-Cookie")->get_value().size() == 2);

    auto& cookies = h.get_cookies()->entries();
    REQUIRE(cookies.size() == 2);
    CHECK(cookies[0]->get_name() == "id");
    CHECK(cookies[1]->get_name() == "session");
}

TEST_CASE("Headers::append('Set-Cookie', ...) with a malformed cookie string fails and adds no header", "[headers][cookies][set-cookie]") {
    Headers h;
    REQUIRE(h.append("Set-Cookie", "this-is-not-a-cookie") == HeaderStatus::InvalidCookie);

    CHECK(h.entries().empty());
    CHECK(h.get_cookies()->entries().empty());
}

TEST_CASE("Headers::append('Cookie', ...) parses each pair into the cookie store and keeps the raw header", "[headers][cookies][cookie]") {
    Headers h;
    REQUIRE(h.append("Cookie", "a=1; b=2") == HeaderStatus::OK);

    SECTION("both cookies land in the store") {
        auto& cookies = h.get_cookies()->entries();
        REQUIRE(cookies.size() == 2);
        CHECK(cookies[0]->get_name() == "a");
        CHECK(cookies[0]->get_value() == "1");
        CHECK(cookies[1]->get_name() == "b");
        CHECK(cookies[1]->get_value() == "2");
    }

    SECTION("the raw header is preserved verbatim") {
        auto entry = h.get("Cookie");
        REQUIRE(entry != nullptr);
        REQUIRE(entry->get_value().size() == 1);
        CHECK(entry->get_value()[0] == "a=1; b=2");
    }
}

TEST_CASE("Headers::append('Cookie', ...) is case-insensitive on the header key", "[headers][cookies][cookie]") {
    Headers h;
    REQUIRE(h.append("cOOkie", "a=1") == HeaderStatus::OK);
    CHECK(h.get_cookies()->entries().size() == 1);
}

TEST_CASE("Headers::append('Cookie', ...) with a malformed pair fails and may leave earlier pairs already applied",
          "[headers][cookies][cookie]") {
    Headers h;

    // First pair is well-formed and gets applied to the store before the second, malformed, pair is reached.
    REQUIRE(h.append("Cookie", "a=1; not-a-pair") == HeaderStatus::InvalidCookie);

    CHECK(h.get_cookies()->entries().size() == 1);
    CHECK(h.get_cookies()->entries()[0]->get_name() == "a");

    // Because append returned early on failure, no "Cookie" header entry was recorded.
    CHECK_FALSE(h.has("Cookie"));
}

TEST_CASE("Headers::set('Set-Cookie', ...) also feeds the cookie store and replaces the header value", "[headers][cookies][set]") {
    Headers h;
    REQUIRE(h.set("Set-Cookie", "id=123") == HeaderStatus::OK);
    REQUIRE(h.set("Set-Cookie", "id=456") == HeaderStatus::OK);

    SECTION("the header has only the latest raw value") {
        auto entry = h.get("Set-Cookie");
        REQUIRE(entry != nullptr);
        REQUIRE(entry->get_value().size() == 1);
        CHECK(entry->get_value()[0] == "id=456");
    }

    SECTION("the cookie store reflects the latest cookie for that name (same name/domain/path replaces in place)") {
        auto& cookies = h.get_cookies()->entries();
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0]->get_value() == "456");
    }
}

TEST_CASE("Headers::erase does not affect the cookie store", "[headers][erase][cookies]") {
    Headers h;
    REQUIRE(h.append("Set-Cookie", "id=123") == HeaderStatus::OK);
    REQUIRE(h.erase("Set-Cookie") == HeaderStatus::OK);

    CHECK_FALSE(h.has("Set-Cookie"));
    CHECK(h.get_cookies()->entries().size() == 1);
}
