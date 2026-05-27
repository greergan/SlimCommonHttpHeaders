#ifndef __SLIM__COMMON__HTTP__HEADERS__H
#define __SLIM__COMMON__HTTP__HEADERS__H

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <slim/SlimValue.h>

#define SLIMCOMMONHTTPHEADERS_VERSION "@SLIMCOMMONHTTPHEADERS_VERSION@"
#define SLIMCOMMONHTTPHEADERS_GIT_HASH "@SLIMCOMMONHTTPHEADERS_GIT_HASH@"

namespace slim::common::http {
	struct Headers {
		Headers();
		slim::SlimValue append(std::string_view _string, std::string_view _value);
		std::unordered_map<std::string, std::string>& entries() const;
		slim::SlimValue erase(std::string_view _string);
		slim::SlimValue get(std::string_view _string) const;
		slim::SlimValue has(std::string_view _string) const;
		slim::SlimValue set(std::string_view _key, std::string_view _value);

		private:
			std::unordered_map<std::string, std::string> __headers;
	};
};

#endif