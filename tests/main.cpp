#include <catch2/catch_test_macros.hpp>

#include <slim/common/http/header.h>
#include <slim/common/http/headers.h>
#include <slim/common/http/cookie.h>
#include <slim/common/http/cookie/store.h>

using namespace slim::common::http;

TEST_CASE("Headers::append adds a new header when the key does not exist", "[headers][append]") {
    Headers h;

    SECTION("simple header") {
        REQUIRE(h.append("Accept", "text/html") == ErrorStatus::OK);
        REQUIRE(h.entries().size() == 1);

        auto entry = h.get("Accept");
        REQUIRE(entry != nullptr);
        CHECK(entry->get_name() == "Accept");
        REQUIRE(entry->get_value().size() == 1);
        CHECK(entry->get_value()[0] == "text/html");
    }

    SECTION("invalid name does not create an entry") {
        REQUIRE(h.append("", "text/html") == ErrorStatus::HeaderNameEmpty);
        CHECK(h.entries().empty());
    }

    SECTION("invalid value does not create an entry") {
        REQUIRE(h.append("Accept", "") == ErrorStatus::HeaderValueEmpty);
        CHECK(h.entries().empty());
        CHECK_FALSE(h.has("Accept"));
    }
}

TEST_CASE("Headers::append on an existing key appends a value rather than replacing it", "[headers][append]") {
    Headers h;
    REQUIRE(h.append("Accept", "text/html") == ErrorStatus::OK);
    REQUIRE(h.append("Accept", "application/json") == ErrorStatus::OK);

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
    REQUIRE(h.append("Content-Type", "application/json") == ErrorStatus::OK);

    CHECK(h.has("content-type"));
    CHECK(h.has("CONTENT-TYPE"));
    CHECK(h.get("cOnTeNt-TyPe") != nullptr);
    CHECK(h.get("cOnTeNt-TyPe")->get_name() == "Content-Type");

    CHECK_FALSE(h.has("X-Missing"));
    CHECK(h.get("X-Missing") == nullptr);
}

TEST_CASE("Headers::erase removes a header by name, case-insensitively", "[headers][erase]") {
    Headers h;
    REQUIRE(h.append("Accept", "text/html") == ErrorStatus::OK);
    REQUIRE(h.append("Vary", "Accept-Encoding") == ErrorStatus::OK);

    SECTION("erase removes the matching header only") {
        REQUIRE(h.erase("ACCEPT") == ErrorStatus::OK);
        CHECK_FALSE(h.has("Accept"));
        CHECK(h.has("Vary"));
        CHECK(h.entries().size() == 1);
    }

    SECTION("erasing a key that does not exist is a no-op and reports OK") {
        REQUIRE(h.erase("X-Missing") == ErrorStatus::OK);
        CHECK(h.entries().size() == 2);
    }
}

TEST_CASE("Headers::set creates a header when the key does not exist", "[headers][set]") {
    Headers h;
    REQUIRE(h.set("Accept-Language", "en") == ErrorStatus::OK);

    auto entry = h.get("Accept-Language");
    REQUIRE(entry != nullptr);
    REQUIRE(entry->get_value().size() == 1);
    CHECK(entry->get_value()[0] == "en");
}

TEST_CASE("Headers::set replaces all existing values on the matching header", "[headers][set]") {
    Headers h;
    REQUIRE(h.append("Accept-Language", "en") == ErrorStatus::OK);
    REQUIRE(h.append("Accept-Language", "fr") == ErrorStatus::OK);
    REQUIRE(h.get("Accept-Language")->get_value().size() == 2);

    REQUIRE(h.set("accept-language", "de") == ErrorStatus::OK);

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
    REQUIRE(h.append("Accept", "text/html") == ErrorStatus::OK);
    REQUIRE(h.append("User-Agent", "test-agent") == ErrorStatus::OK);
    REQUIRE(h.append("Vary", "Accept-Encoding") == ErrorStatus::OK);

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

TEST_CASE("Headers::append('Set-Cookie', ...) populates the cookie store but is not added as a header",
          "[headers][cookies][set-cookie]") {
    Headers h;
    REQUIRE(h.append("Set-Cookie", "id=123; Path=/; Secure") == ErrorStatus::OK);

    SECTION("the cookie store gains an entry") {
        auto& cookies = h.get_cookies()->entries();
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0]->get_name() == "id");
        CHECK(cookies[0]->get_value() == "123");
        REQUIRE(cookies[0]->get_path().has_value());
        CHECK(*cookies[0]->get_path() == "/");
        CHECK(cookies[0]->get_secure());
    }

    SECTION("no raw header is recorded") {
        CHECK_FALSE(h.has("Set-Cookie"));
        CHECK(h.get("Set-Cookie") == nullptr);
        CHECK(h.entries().empty());
    }
}

TEST_CASE("Headers::append('Set-Cookie', ...) called twice records two cookies and adds no header entry",
          "[headers][cookies][set-cookie]") {
    Headers h;
    REQUIRE(h.append("Set-Cookie", "id=123") == ErrorStatus::OK);
    REQUIRE(h.append("Set-Cookie", "session=abc") == ErrorStatus::OK);

    CHECK(h.entries().empty());
    CHECK_FALSE(h.has("Set-Cookie"));

    auto& cookies = h.get_cookies()->entries();
    REQUIRE(cookies.size() == 2);
    CHECK(cookies[0]->get_name() == "id");
    CHECK(cookies[1]->get_name() == "session");
}

TEST_CASE("Headers::append('Set-Cookie', ...) with a malformed cookie string fails and adds no header", "[headers][cookies][set-cookie]") {
    Headers h;
    REQUIRE(h.append("Set-Cookie", "this-is-not-a-cookie") == ErrorStatus::CookieMalformedMissingEquals);

    CHECK(h.entries().empty());
    CHECK(h.get_cookies()->entries().empty());
}

TEST_CASE("Headers::append('Cookie', ...) parses each pair into the cookie store and adds no header",
          "[headers][cookies][cookie]") {
    Headers h;
    REQUIRE(h.append("Cookie", "a=1; b=2") == ErrorStatus::OK);

    SECTION("both cookies land in the store") {
        auto& cookies = h.get_cookies()->entries();
        REQUIRE(cookies.size() == 2);
        CHECK(cookies[0]->get_name() == "a");
        CHECK(cookies[0]->get_value() == "1");
        CHECK(cookies[1]->get_name() == "b");
        CHECK(cookies[1]->get_value() == "2");
    }

    SECTION("no raw header is recorded") {
        CHECK_FALSE(h.has("Cookie"));
        CHECK(h.get("Cookie") == nullptr);
        CHECK(h.entries().empty());
    }
}

TEST_CASE("Headers::append('Cookie', ...) is case-insensitive on the header key and still adds no header",
          "[headers][cookies][cookie]") {
    Headers h;
    REQUIRE(h.append("cOOkie", "a=1") == ErrorStatus::OK);

    CHECK(h.get_cookies()->entries().size() == 1);
    CHECK(h.entries().empty());
    CHECK_FALSE(h.has("Cookie"));
}

TEST_CASE("Headers::append('Cookie', ...) with a malformed pair fails and may leave earlier pairs already applied",
          "[headers][cookies][cookie]") {
    Headers h;

    // First pair is well-formed and gets applied to the store before the second, malformed, pair is reached.
    REQUIRE(h.append("Cookie", "a=1; not-a-pair") == ErrorStatus::CookieMalformedPairMissingEquals);

    CHECK(h.get_cookies()->entries().size() == 1);
    CHECK(h.get_cookies()->entries()[0]->get_name() == "a");

    // No "Cookie" header entry was recorded, whether append failed or succeeded.
    CHECK_FALSE(h.has("Cookie"));
}

TEST_CASE("Headers::set('Set-Cookie', ...) also feeds the cookie store but never appears as a header",
          "[headers][cookies][set]") {
    Headers h;
    REQUIRE(h.set("Set-Cookie", "id=123") == ErrorStatus::OK);
    REQUIRE(h.set("Set-Cookie", "id=456") == ErrorStatus::OK);

    SECTION("no header entry exists for Set-Cookie") {
        CHECK_FALSE(h.has("Set-Cookie"));
        CHECK(h.get("Set-Cookie") == nullptr);
        CHECK(h.entries().empty());
    }

    SECTION("the cookie store reflects the latest cookie for that name (same name/domain/path replaces in place)") {
        auto& cookies = h.get_cookies()->entries();
        REQUIRE(cookies.size() == 1);
        CHECK(cookies[0]->get_value() == "456");
    }
}

TEST_CASE("Headers::erase has no effect on the cookie store, since cookie headers are never stored as ordinary headers",
          "[headers][erase][cookies]") {
    Headers h;
    REQUIRE(h.append("Set-Cookie", "id=123") == ErrorStatus::OK);

    // Set-Cookie was never added as a header entry, so erasing it is a no-op.
    REQUIRE(h.erase("Set-Cookie") == ErrorStatus::OK);

    CHECK_FALSE(h.has("Set-Cookie"));
    CHECK(h.get_cookies()->entries().size() == 1);
}

TEST_CASE("Mixing ordinary headers with cookie headers keeps only the ordinary ones in entries()",
          "[headers][cookies][entries]") {
    Headers h;
    REQUIRE(h.append("Accept", "text/html") == ErrorStatus::OK);
    REQUIRE(h.append("Set-Cookie", "id=123") == ErrorStatus::OK);
    REQUIRE(h.append("Cookie", "a=1") == ErrorStatus::OK);
    REQUIRE(h.append("Vary", "Accept-Encoding") == ErrorStatus::OK);

    const auto& entries = h.entries();
    REQUIRE(entries.size() == 2);
    CHECK(entries[0]->get_name() == "Accept");
    CHECK(entries[1]->get_name() == "Vary");

    CHECK(h.get_cookies()->entries().size() == 2);
}

TEST_CASE("Headers::serialize renders entries as wire-format header lines", "[headers][serialize]") {
    Headers h;
    REQUIRE(h.append("Accept", "text/html") == ErrorStatus::OK);
    REQUIRE(h.append("Accept", "application/json") == ErrorStatus::OK);
    REQUIRE(h.append("Vary", "Accept-Encoding") == ErrorStatus::OK);

    const std::string expects = "Accept: text/html, application/json\r\n"
                                 "Vary: Accept-Encoding\r\n"
                                 "\r\n";

    CHECK(h.serialize() == expects);
}
