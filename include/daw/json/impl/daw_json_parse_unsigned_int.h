// The MIT License (MIT)
//
// Copyright (c) 2019 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "daw_iterator_range.h"
#include "daw_json_assert.h"

#include <daw/daw_cxmath.h>

#include <cstddef>
#include <utility>

#ifdef DAW_ALLOW_SSE2
#include <emmintrin.h>
#include <xmmintrin.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif
#endif

namespace daw::json::impl::unsignedint {
	namespace {
		template<typename Unsigned>
		struct unsigned_parser {
			[[nodiscard]] static constexpr std::pair<Unsigned, char const *>
			parse( char const *ptr ) {
				uintmax_t n = 0;
				auto dig = static_cast<unsigned>( *ptr ) - static_cast<unsigned>( '0' );
				while( dig < 10U ) {
					n *= 10U;
					n += static_cast<uintmax_t>( dig );
					++ptr;
					dig = static_cast<unsigned>( *ptr ) - static_cast<unsigned>( '0' );
				}
				return {daw::construct_a<Unsigned>( n ), ptr};
			}

#ifdef DAW_ALLOW_SSE2
			// SSE2 end of number finding.  Using technique from
			// https://github.com/vinniefalco/json/blob/develop/include/boost/json/detail/sse2.hpp
			[[nodiscard]] static constexpr size_t find_len_sse2( char const *ptr ) {
				__m128i const lower_bound = _mm_set1_epi8( '0' );
				__m128i const upper_bound = _mm_set1_epi8( '9' );
				__m128i values1 =
				  _mm_loadu_si128( reinterpret_cast<__m128i const *>( ptr ) );

				values1 = _mm_or_si128( _mm_cmplt_epi8( values1, lower_bound ),
				                        _mm_cmpgt_epi8( values1, upper_bound ) );

				auto const m = _mm_movemask_epi8( values1 );
				if( m == 0 ) {
					return static_cast<size_t>( 16U );
				}
#if defined( __GNUC__ ) or defined( __clang__ )
				return static_cast<size_t>( __builtin_ffs( m ) - 1 );
#else
				// MSVC
				unsigned long index;
				_BitScanForward( &index, m );
				return static_cast<size_t>( index );
#endif
			}

			[[nodiscard]] static constexpr std::pair<Unsigned, char const *>
			parse_sse2( char const *ptr ) {
				auto const digits = [ptr] {
					auto result = find_len_sse2( ptr );
					return result == 16U ? result + find_len_sse2( ptr + 16 ) : result;
				}( );
				uintmax_t result = 0;
				for( size_t n = 0; n < digits; ++n ) {
					result *= 10U;
					auto const dig = static_cast<uintmax_t>( ptr[n] - '0' );
					result += dig;
				}
				ptr += static_cast<ptrdiff_t>( digits );
				return {daw::construct_a<Unsigned>( result ), ptr};
			}
#endif
		};
		static_assert( unsigned_parser<unsigned>::parse( "12345" ).first == 12345 );
	} // namespace
} // namespace daw::json::impl::unsignedint

namespace daw::json::impl {
	namespace {
		template<typename Result, JsonRangeCheck RangeCheck = JsonRangeCheck::Never,
		         typename First, typename Last, bool IsTrustedInput>
		[[nodiscard]] constexpr auto parse_unsigned_integer2(
		  IteratorRange<First, Last, IsTrustedInput> &rng ) noexcept {
			daw_json_assert_untrusted( rng.is_number( ),
			                           "Expecting a digit as first item" );

			using namespace daw::json::impl::unsignedint;
			using iresult_t =
			  std::conditional_t<RangeCheck == JsonRangeCheck::CheckForNarrowing,
			                     uintmax_t, Result>;
			auto [v, new_p] = unsigned_parser<iresult_t>::parse( rng.first );
			uint_fast8_t c = static_cast<uint_fast8_t>( new_p - rng.first );
			rng.first = new_p;

			struct result_t {
				Result value;
				uint_fast8_t count;
			};

			if constexpr( RangeCheck == JsonRangeCheck::CheckForNarrowing ) {
				return result_t{daw::narrow_cast<Result>( v ), c};
			} else {
				return result_t{v, c};
			}
		}

		template<typename Result, JsonRangeCheck RangeCheck, SIMDModes SimdMode,
		         typename First, typename Last, bool IsTrustedInput>
		[[nodiscard]] constexpr Result parse_unsigned_integer(
		  IteratorRange<First, Last, IsTrustedInput> &rng ) noexcept {
			daw_json_assert_untrusted( rng.is_number( ),
			                           "Expecting a digit as first item" );

			using namespace daw::json::impl::unsignedint;
			using result_t =
			  std::conditional_t<RangeCheck == JsonRangeCheck::CheckForNarrowing or
			                       std::is_enum_v<Result>,
			                     uintmax_t, Result>;
			auto [result, ptr] = [&] {
				if constexpr( SimdMode == SIMDModes::SSE2 ) {
					return unsigned_parser<result_t>::parse_sse2( rng.first );
				} else {
					return unsigned_parser<result_t>::parse( rng.first );
				}
			}( );
			rng.first = ptr;

			if constexpr( RangeCheck == JsonRangeCheck::CheckForNarrowing ) {
				return daw::narrow_cast<Result>( result );
			} else {
				return static_cast<Result>( result );
			}
		}
	} // namespace
} // namespace daw::json::impl
