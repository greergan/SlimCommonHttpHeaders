#include <string>
#include <string_view>
#include <unordered_map>
#include <slim/common/http/headers.h>

namespace slim::common::http::header {
	slim::SlimValue valid_key(std::string_view _key);
}

slim::SlimValue slim::common::http::header::valid_key(std::string_view _key) {
	slim::SlimValue results = true;
	return results;
}

slim::common::http::Headers::Headers() {}

slim::SlimValue slim::common::http::Headers::append(std::string_view _key, std::string_view _value) {
	slim::SlimValue results = header::valid_key(_key);
	if(results) {
		__headers[std::string(_key)] = std::string(_value);
	}
	return results;
}

std::unordered_map<std::string, std::string>& slim::common::http::Headers::entries() {
	return __headers;
}

slim::SlimValue slim::common::http::Headers::get(std::string_view _key) const {
	slim::SlimValue results = has(_key);
	if(results) {
		results = __headers.at(std::string(_key));
	}
	return results;
}

slim::SlimValue slim::common::http::Headers::has(std::string_view _key) const {
	slim::SlimValue results = false;
	if(__headers.contains(std::string(_key))) {
		results = true;
	}
	return results;
}

slim::SlimValue slim::common::http::Headers::set(std::string_view _key, std::string_view _value) {
	slim::SlimValue results = header::valid_key(_key);
	if(results) {
		__headers[std::string(_key)] = std::string(_value);
	}
	return results;
}
