#include <slim/common/http/headers.h>
#include <slim/common/http/header.h>
#include <slim/common/http/cookie.h>
#include <slim/common/http/cookie/store.h>
#include <slim/common/utilities.h>

#include <algorithm>

namespace slim::common::http {

namespace {
    using slim::common::utilities::iequals;
    using slim::common::utilities::iiequals;
} // namespace

ErrorStatus Headers::append(std::string_view key, std::string_view value) noexcept {
    if (iequals(key, "set-cookie")) return cookies->set(value);
    if (iequals(key, "cookie"))     return cookies->set_cookies(value);

    if (auto h = get(key)) return h->set_value(value);

    auto new_header = std::make_shared<Header>();

    auto e = new_header->set_name(key);
    if(e != ErrorStatus::OK) return e;

    e = new_header->set_value(value);
    if(e != ErrorStatus::OK) return e;

    headers.push_back(std::move(new_header));
    return ErrorStatus::OK;
}

ErrorStatus Headers::erase(std::string_view key) noexcept {
    auto it = std::remove_if(headers.begin(), headers.end(),
        [&](const std::shared_ptr<Header>& h) {
            return iiequals(h->get_name(), key);
        });

    if (it != headers.end())  headers.erase(it, headers.end());

    return ErrorStatus::OK;
}

std::shared_ptr<Header> Headers::get(std::string_view key) const noexcept {
    for (const auto& h : headers)
        if (iiequals(h->get_name(), key)) return h;

    return nullptr;
}

bool Headers::has(std::string_view key) const noexcept {
    return get(key) != nullptr;
}

ErrorStatus Headers::set(std::string_view key, std::string_view value) noexcept {
    if (iequals(key, "set-cookie")) return cookies->set(value);
    if (iequals(key, "cookie"))     return cookies->set_cookies(value);

    if (auto h = get(key)) return h->replace_value(value);

    auto new_header = std::make_shared<Header>();

    auto e = new_header->set_name(key);
    if(e != ErrorStatus::OK) return e;

    e = new_header->set_value(value);
    if(e != ErrorStatus::OK) return e;

    headers.push_back(std::move(new_header));
    return ErrorStatus::OK;
}

std::string Headers::serialize() const {
    std::string result;

    for (const auto& h : headers)
        if (h) result += h->serialize();

    if (cookies) result += cookies->serialize();

    result.append("\r\n");

    return result;
}

} // namespace slim::common::http
