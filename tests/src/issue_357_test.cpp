// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_json_link
//

#include <daw/json/daw_json_link.h>

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

struct voltage {
	double num;
};

struct Bus {
	int uid{ };
	std::string name;
	std::optional<voltage> volts;
};

struct buses {
	int id;
};

struct buses_res {
	int id;
};

struct System {
	std::string name;
	// ** here is the problem, optional before vector of classes
	std::optional<std::string> version;
	std::vector<buses> b;
	std::vector<buses_res> br;
};

namespace daw::json {
	template<>
	struct json_data_contract<voltage> {
		static constexpr char const num[] = "num";
		using type = json_member_list<json_number<num>>;

		static constexpr auto to_json_data( voltage const &v ) {
			return std::forward_as_tuple( v.num );
		}
	};
	template<>
	struct json_data_contract<Bus> {
		static constexpr char const uid[] = "uid";
		static constexpr char const name[] = "name";
		static constexpr char const volts[] = "volts";
		using type = json_member_list<json_number<uid, int>, json_string<name>,
		                              json_class_null<volts, voltage>>;

		static constexpr auto to_json_data( Bus const &b ) {
			return std::forward_as_tuple( b.uid, b.name, b.volts );
		}
	};
	template<>
	struct json_data_contract<buses> {
		static constexpr char const id[] = "id";
		using type = json_member_list<json_number<id, int>>;

		static constexpr auto to_json_data( buses const &b ) {
			return std::forward_as_tuple( b.id );
		}
	};
	template<>
	struct json_data_contract<buses_res> {
		static constexpr char const id[] = "id";
		using type = json_member_list<json_number<id, int>>;

		static constexpr auto to_json_data( buses_res const &b ) {
			return std::forward_as_tuple( b.id );
		}
	};
	template<>
	struct json_data_contract<System> {
		static constexpr char const name[] = "name";
		static constexpr char const version[] = "version";
		static constexpr char const b[] = "b";
		static constexpr char const br[] = "br";
		using type =
		  json_member_list<json_string<name>, json_string_null<version>,
		                   json_array<b, buses>, json_array<br, buses_res>>;

		static constexpr auto to_json_data( System const &s ) {
			return std::forward_as_tuple( s.name, s.version, s.b, s.br );
		}
	};
} // namespace daw::json

int main( ) {
	std::string_view json_doc = R"json(
{
	"name": "foo",
	"b": [{
		"id": 1234
	}],
	"br": [{
		"id": 5678
	}]
}
)json";

	auto s = daw::json::from_json<System>( json_doc );
	auto new_doc = daw::json::to_json( s );
	std::cout << new_doc << '\n';
}