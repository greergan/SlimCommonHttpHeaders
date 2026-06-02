#include <string>
#include <string_view>
#include <unordered_map>
#include <slim/common/http/headers.h>

namespace {
	std::string to_lower(std::string_view _string) {
        std::string result(_string);
        for(auto& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return result;
    }
	slim::SlimValue validkey(std::string_view key) {
		slim::SlimValue result = to_lower(key);
		if(!result) {
			result.set_error("header key must not be empty");
		}
		return result;
	}
}

slim::common::http::Headers::Headers() {}

slim::SlimValue slim::common::http::Headers::append(std::string_view key, std::string_view value) {
	slim::SlimValue result = validkey(key);
	if(result) {
		if(value.empty()) {
			result = false;
			result.set_error("header value must not be empty");
			return result;
		}

		auto currentvalue = headers[result.to_string()];
		if(currentvalue.empty()) {
			currentvalue = std::string(value);
		}
		else {
			currentvalue += ", " + std::string(value);
		}
		headers[result.to_string()] = currentvalue;
	}
	return result;
}

const std::unordered_map<std::string, std::string>& slim::common::http::Headers::entries() const {
	return headers;
}

slim::SlimValue slim::common::http::Headers::erase(std::string_view key) {
	slim::SlimValue result = validkey(key);
	if(result) {
		result = headers.erase(result.to_string());
	}
	return result;
}

slim::SlimValue slim::common::http::Headers::get(std::string_view key) const {
	slim::SlimValue result = validkey(key);
	if(result) {
		auto it = headers.find(result.to_string());
		result = it != headers.end() ? result = it->second : result = false;
	}
	return result;
}

bool slim::common::http::Headers::has(std::string_view key) const {
	return headers.contains(to_lower(key));
}

slim::SlimValue slim::common::http::Headers::set(std::string_view key, std::string_view value) {
	slim::SlimValue result = validkey(key);
	if(result) {
		if(value.empty()) {
			result = false;
			result.set_error("header value must not be empty");
			return result;
		}
		headers[result.to_string()] = std::string(value);
	}
	return result;
}
