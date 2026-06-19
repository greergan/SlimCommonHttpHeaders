#include <slim/common/http/headers.h>
#include <slim/common/http/header.h>
#include <slim/common/http/cookie.h>
#include <slim/common/http/cookie/store.h>


#include <algorithm>
#include <cctype>

namespace slim::common::http {

namespace {
    struct AsciiTables {
        std::array<char, 256> to_lower{};

        constexpr AsciiTables() noexcept {
            for (size_t i = 0; i < 256; ++i) {
                to_lower[i] = (i >= 'A' && i <= 'Z') ? static_cast<char>(i + 32) : static_cast<char>(i);
            }
        }
    };

    constexpr AsciiTables ascii{};

    constexpr bool iequals(std::string_view a, std::string_view b) noexcept {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (ascii.to_lower[static_cast<unsigned char>(a[i])] != static_cast<unsigned char>(b[i])) return false;
        return true;
    }

    constexpr bool iiequals(std::string_view a, std::string_view b) noexcept {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (ascii.to_lower[static_cast<unsigned char>(a[i])]
                != ascii.to_lower[static_cast<unsigned char>(b[i])]) return false;
        return true;
    }
} // namespace

HeaderStatus Headers::append(std::string_view key, std::string_view value) noexcept {
    if (iequals(key, "set-cookie")) {
        auto e = cookies->set(value);
        if(e != CookieStatus::OK) return HeaderStatus::InvalidCookie;
        return HeaderStatus::OK;
    }
    if (iequals(key, "cookie")) {
        auto e = cookies->set_cookies(value);
        if(e != CookieStatus::OK) return HeaderStatus::InvalidCookie;
        return HeaderStatus::OK;
    }

    if (auto h = get(key)) return h->set_value(value);

    auto new_header = std::make_shared<Header>();

    auto e = new_header->set_name(key);
    if(e != HeaderStatus::OK) return e;

    e = new_header->set_value(value);
    if(e != HeaderStatus::OK) return e;

    headers.push_back(std::move(new_header));
    return HeaderStatus::OK;
}

HeaderStatus Headers::erase(std::string_view key) noexcept {
    auto it = std::remove_if(headers.begin(), headers.end(),
        [&](const std::shared_ptr<Header>& h) {
            return iiequals(h->get_name(), key);
        });

    if (it != headers.end())  headers.erase(it, headers.end());

    return HeaderStatus::OK;
}

std::shared_ptr<Header> Headers::get(std::string_view key) const noexcept {
    for (const auto& h : headers)
        if (iiequals(h->get_name(), key)) return h;

    return nullptr;
}

bool Headers::has(std::string_view key) const noexcept {
    return get(key) != nullptr;
}

HeaderStatus Headers::set(std::string_view key, std::string_view value) noexcept {
    if (iequals(key, "set-cookie")) {
        auto e = cookies->set(value);
        if(e != CookieStatus::OK) return HeaderStatus::InvalidCookie;
        return HeaderStatus::OK;
    }
    if (iequals(key, "cookie")) {
        auto e = cookies->set_cookies(value);
        if(e != CookieStatus::OK) return HeaderStatus::InvalidCookie;
        return HeaderStatus::OK;
    }

    if (auto h = get(key)) return h->replace_value(value);

    auto new_header = std::make_shared<Header>();

    auto e = new_header->set_name(key);
    if(e != HeaderStatus::OK) return e;

    e = new_header->set_value(value);
    if(e != HeaderStatus::OK) return e;

    headers.push_back(std::move(new_header));
    return HeaderStatus::OK;
}

} // namespace slim::common::http
