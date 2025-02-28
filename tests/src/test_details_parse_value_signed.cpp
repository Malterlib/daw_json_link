// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_json_link
//

#include "defines.h"

#include <daw/json/daw_json_link.h>
#include <daw/json/impl/daw_json_parse_common.h>

#include <daw/daw_benchmark.h>

#include <iostream>
#include <optional>
#include <string_view>

bool test_zero_untrusted( ) {
	using namespace daw::json;
	using namespace daw::json::json_details;

	using my_number = json_number_no_name<signed>;
	DAW_CONSTEXPR std::string_view sv = "0,";
	auto rng = BasicParsePolicy( sv.data( ), sv.data( ) + sv.size( ) );
	auto v = parse_value_signed<my_number, false>( rng );
	return not v;
}

bool test_positive_zero_untrusted( ) {
	using namespace daw::json;
	using namespace daw::json::json_details;

	using my_number = json_number_no_name<signed>;
	DAW_CONSTEXPR std::string_view sv = "+0,";
	auto rng = BasicParsePolicy( sv.data( ), sv.data( ) + sv.size( ) );
	auto v = parse_value_signed<my_number, false>( rng );
	return not v;
}

bool test_negative_zero_untrusted( ) {
	using namespace daw::json;
	using namespace daw::json::json_details;

	using my_number = json_number_no_name<signed>;
	DAW_CONSTEXPR std::string_view sv = "-0,";
	auto rng = BasicParsePolicy( sv.data( ), sv.data( ) + sv.size( ) );
	auto v = parse_value_signed<my_number, false>( rng );
	return not v;
}

bool test_missing_untrusted( ) {
	using namespace daw::json;
	using namespace daw::json::json_details;

	using my_number = json_number_no_name<signed>;
	DAW_CONSTEXPR std::string_view sv = " ,";
	auto rng = BasicParsePolicy( sv.data( ), sv.data( ) + sv.size( ) );
	auto v = parse_value_signed<my_number, false>( rng );
	daw::do_not_optimize( v );
	return false;
}

bool test_real_untrusted( ) {
	using namespace daw::json;
	using namespace daw::json::json_details;

	using my_number = json_number_no_name<signed>;
	DAW_CONSTEXPR std::string_view sv = "1.23,";
	auto rng = BasicParsePolicy( sv.data( ), sv.data( ) + sv.size( ) );
	auto v = parse_value_signed<my_number, false>( rng );
	daw::do_not_optimize( v );
	return false;
}

#define do_test( ... )                                                   \
	try {                                                                  \
		auto result = ( __VA_ARGS__ );                                       \
		daw::expecting_message( result, "" #__VA_ARGS__ );                   \
		daw::do_not_optimize( result );                                      \
	} catch( daw::json::json_exception const &jex ) {                      \
		std::cerr << "Unexpected exception thrown by parser in test '"       \
		          << "" #__VA_ARGS__ << "': " << jex.reason( ) << std::endl; \
	}                                                                      \
	do {                                                                   \
	} while( false )

#define do_fail_test( ... )                                                    \
	do {                                                                         \
		try {                                                                      \
			daw::expecting_message( __VA_ARGS__, "" #__VA_ARGS__ );                  \
		} catch( daw::json::json_exception const & ) { break; }                    \
		std::cerr << "Expected exception, but none thrown in '" << "" #__VA_ARGS__ \
		          << "'\n";                                                        \
	} while( false )

int main( int, char ** )
#ifdef DAW_USE_EXCEPTIONS
  try
#endif
{
	do_test( test_zero_untrusted( ) );
	do_fail_test( test_positive_zero_untrusted( ) );
	do_test( test_negative_zero_untrusted( ) );
	do_fail_test( test_missing_untrusted( ) );
	do_fail_test( test_real_untrusted( ) );
}
#ifdef DAW_USE_EXCEPTIONS
catch( daw::json::json_exception const &jex ) {
	std::cerr << "Exception thrown by parser: " << jex.reason( ) << '\n';
	exit( 1 );
} catch( std::exception const &ex ) {
	std::cerr << "Unknown exception thrown during testing: " << ex.what( )
	          << '\n';
	exit( 1 );
} catch( ... ) {
	std::cerr << "Unknown exception thrown during testing\n";
	throw;
}
#endif