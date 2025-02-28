// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_json_link
//

#pragma once

#include "impl/version.h"

#include "daw_from_json_fwd.h"
#include "impl/daw_json_parse_policy.h"
#include "impl/daw_json_value.h"

#include <daw/daw_move.h>
#include <daw/daw_string_view.h>

#include <cstddef>
#include <daw/stdinc/declval.h>
#include <daw/stdinc/move_fwd_exch.h>
#include <optional>
#include <utility>
#include <vector>

namespace daw::json {
	inline namespace DAW_JSON_VER {
		enum json_parse_handler_result {
			/// Continue parsing with next element/member
			Continue,
			/// Skip the rest of this class or array
			SkipClassArray,
			/// We are completed and do not wish to see any more
			Complete
		};

		namespace json_details {
			struct handler_result_holder {
				json_parse_handler_result value = json_parse_handler_result::Continue;

				handler_result_holder( ) = default;

				constexpr handler_result_holder( bool b )
				  : value( b ? Continue : Complete ) {}

				constexpr handler_result_holder( json_parse_handler_result r )
				  : value( r ) {}

				constexpr explicit operator bool( ) const {
					return value == json_parse_handler_result::Continue;
				}
			};

			namespace hnd_checks {
				// On Next Value
				DAW_JSON_MAKE_REQ_TRAIT2(
				  has_on_value_handler_impl,
				  std::declval<T>( ).handle_on_value( std::declval<U>( ) ) );

				template<typename Handler, json_options_t, typename JPair>
				inline constexpr bool has_on_value_handler_v =
				  has_on_value_handler_impl<Handler, JPair>;

				// On Array Start
				DAW_JSON_MAKE_REQ_TRAIT2(
				  has_on_array_start_handler_impl,
				  std::declval<T>( ).handle_on_array_start( std::declval<U>( ) ) );

				template<typename Handler, json_options_t /*O*/, typename JValue>
				inline constexpr bool has_on_array_start_handler_v =
				  has_on_array_start_handler_impl<Handler, JValue>;

				// On Array End
				DAW_JSON_MAKE_REQ_TRAIT( has_on_array_end_handler_v,
				                         std::declval<T>( ).handle_on_array_end( ) );

				// On Class Start
				DAW_JSON_MAKE_REQ_TRAIT2(
				  has_on_class_start_handler_impl,
				  std::declval<T>( ).handle_on_class_start( std::declval<U>( ) ) );

				template<typename Handler, json_options_t /*Opt*/, typename JValue>
				inline constexpr bool has_on_class_start_handler_v =
				  has_on_class_start_handler_impl<Handler, JValue>;

				// On Class End
				DAW_JSON_MAKE_REQ_TRAIT( has_on_class_end_handler_v,
				                         std::declval<T>( ).handle_on_class_end( ) );

				// On Number
				DAW_JSON_MAKE_REQ_TRAIT2(
				  has_on_number_handler_jv_impl,
				  std::declval<T>( ).handle_on_number( std::declval<U>( ) ) );

				template<typename Handler, json_options_t P, typename A>
				inline constexpr bool has_on_number_handler_jv_v =
				  has_on_number_handler_jv_impl<Handler, basic_json_value<P, A>>;

				DAW_JSON_MAKE_REQ_TRAIT( has_on_number_handler_dbl_v,
				                         std::declval<T>( ).handle_on_number( 0.0 ) );

				// On Bool // T = Handler, U = JValue
				DAW_JSON_MAKE_REQ_TRAIT2(
				  has_on_bool_handler_jv_impl,
				  std::declval<T>( ).handle_on_bool( std::declval<U>( ) ) );

				template<typename Handler, json_options_t P, typename A>
				inline constexpr bool has_on_bool_handler_jv_v =
				  has_on_bool_handler_jv_impl<Handler, basic_json_value<P, A>>;

				DAW_JSON_MAKE_REQ_TRAIT( has_on_bool_handler_bl_v,
				                         std::declval<T>( ).handle_on_bool( true ) );

				// On String // T = Handler, U = JValue
				DAW_JSON_MAKE_REQ_TRAIT2(
				  has_on_string_handler_impl,
				  std::declval<T>( ).handle_on_string( std::declval<U>( ) ) );

				template<typename Handler, json_options_t P, typename A>
				inline constexpr bool has_on_string_handler_jv_v =
				  has_on_string_handler_impl<Handler, basic_json_value<P, A>>;

				DAW_JSON_MAKE_REQ_TRAIT(
				  has_on_string_handler_str_v,
				  std::declval<T>( ).handle_on_string( std::declval<std::string>( ) ) );

				// On Null, T = Handler, U = JValue
				DAW_JSON_MAKE_REQ_TRAIT2(
				  has_on_null_handler_impl,
				  std::declval<T>( ).handle_on_null( std::declval<U>( ) ) );

				template<typename Handler, json_options_t P, typename A>
				inline constexpr bool has_on_null_handler_jv_v =
				  has_on_null_handler_impl<Handler, basic_json_value<P, A>>;

				DAW_JSON_MAKE_REQ_TRAIT( has_on_null_handler_v,
				                         std::declval<T>( ).handle_on_null( ) );

				// On Error
				DAW_JSON_MAKE_REQ_TRAIT2(
				  has_on_error_handler_impl,
				  std::declval<T>( ).handle_on_error( std::declval<U>( ) ) );

				template<typename Handler, json_options_t P, typename A>
				inline constexpr bool has_on_error_handler_v =
				  has_on_error_handler_impl<Handler, basic_json_value<P, A>>;
			} // namespace hnd_checks

			template<typename T>
			constexpr daw::remove_cvref_t<T> as_copy( T &&value ) {
				return value;
			}

			template<typename Handler, json_options_t P, typename A>
			inline constexpr handler_result_holder
			handle_on_value( Handler &&handler, basic_json_pair<P, A> p ) {
				if constexpr( hnd_checks::has_on_value_handler_v<Handler, P, A> ) {
					return handler.handle_on_value( std::move( p ) );
				} else {
					(void)p;
					return handler_result_holder{ };
				}
			}

			template<typename Handler, json_options_t P, typename A>
			inline constexpr handler_result_holder
			handle_on_array_start( Handler &&handler, basic_json_value<P, A> jv ) {
				if constexpr( hnd_checks::has_on_array_start_handler_v<Handler, P,
				                                                       A> ) {
					return handler.handle_on_array_start( std::move( jv ) );
				} else {
					(void)jv;
					return handler_result_holder{ };
				}
			}

			template<typename Handler>
			inline constexpr handler_result_holder
			handle_on_array_end( Handler &&handler ) {
				if constexpr( hnd_checks::has_on_array_end_handler_v<Handler> ) {
					return handler.handle_on_array_end( );
				} else {
					return handler_result_holder{ };
				}
			}

			template<typename Handler, json_options_t P, typename A>
			inline constexpr handler_result_holder
			handle_on_class_start( Handler &&handler, basic_json_value<P, A> jv ) {
				if constexpr( hnd_checks::has_on_class_start_handler_v<Handler, P,
				                                                       A> ) {
					return handler.handle_on_class_start( std::move( jv ) );
				} else {
					(void)jv;
					return handler_result_holder{ };
				}
			}

			template<typename Handler>
			inline constexpr handler_result_holder
			handle_on_class_end( Handler &&handler ) {
				if constexpr( hnd_checks::has_on_class_end_handler_v<Handler> ) {
					return handler.handle_on_class_end( );
				} else {
					return handler_result_holder{ };
				}
			}

			template<typename Handler, json_options_t P, typename A>
			inline constexpr handler_result_holder
			handle_on_number( Handler &&handler, basic_json_value<P, A> &jv ) {
				if constexpr( hnd_checks::has_on_number_handler_jv_v<Handler, P, A> ) {
					return handler.handle_on_number( as_copy( jv ) );
				} else if constexpr( hnd_checks::has_on_number_handler_dbl_v<
				                       Handler> ) {
					return handler.handle_on_number( from_json<double>( jv ) );
				} else {
					(void)jv;
					return handler_result_holder{ };
				}
			}

			template<typename Handler, json_options_t P, typename A>
			inline constexpr handler_result_holder
			handle_on_bool( Handler &&handler, basic_json_value<P, A> jv ) {
				if constexpr( hnd_checks::has_on_bool_handler_jv_v<Handler, P, A> ) {
					return handler.handle_on_bool( as_copy( jv ) );
				} else if constexpr( hnd_checks::has_on_bool_handler_bl_v<Handler> ) {
					return handler.handle_on_bool( from_json<bool>( jv ) );
				} else {
					(void)jv;
					return handler_result_holder{ };
				}
			}

			template<typename Handler, json_options_t P, typename A>
			inline constexpr handler_result_holder
			handle_on_string( Handler &&handler, basic_json_value<P, A> &jv ) {
				if constexpr( hnd_checks::has_on_string_handler_jv_v<Handler, P, A> ) {
					return handler.handle_on_string( as_copy( jv ) );
				} else if constexpr( hnd_checks::has_on_string_handler_str_v<
				                       Handler> ) {
					return handler.handle_on_string( jv.get_string( ) );
				} else {
					(void)jv;
					return handler_result_holder{ };
				}
			}

			template<typename Handler, json_options_t P, typename A>
			inline constexpr handler_result_holder
			handle_on_null( Handler &&handler, basic_json_value<P, A> &jv ) {
				if constexpr( hnd_checks::has_on_null_handler_jv_v<Handler, P, A> ) {
					return handler.handle_on_null( as_copy( jv ) );
				} else if constexpr( hnd_checks::has_on_null_handler_v<Handler> ) {
					return handler.handle_on_null( );
				} else {
					return handler_result_holder{ };
				}
			}

			template<typename Handler, json_options_t P, typename A>
			inline constexpr handler_result_holder
			handle_on_error( Handler &&handler, basic_json_value<P, A> jv ) {
				if constexpr( hnd_checks::has_on_error_handler_v<Handler, P, A> ) {
					return handler.handle_on_error( std::move( jv ) );
				} else {
					(void)jv;
					return handler_result_holder{ };
				}
			}

		} // namespace json_details

		enum class StackParseStateType { Class, Array };

		template<json_options_t P, typename A>
		struct JsonEventParserStackValue {
			using iterator = basic_json_value_iterator<P, A>;
			StackParseStateType type;
			std::pair<iterator, iterator> value;
		};

		template<typename StackValue>
		class DefaultJsonEventParserStackPolicy {
			std::vector<StackValue> m_stack{ };

		public:
			using value_type = StackValue;
			using reference = StackValue &;
			using size_type = std::size_t;
			using difference_type = std::ptrdiff_t;

			DefaultJsonEventParserStackPolicy( ) = default;

			CPP20CONSTEXPR void push_back( value_type &&v ) {
				m_stack.push_back( std::move( v ) );
			}

			[[nodiscard]] CPP20CONSTEXPR reference back( ) {
				return m_stack.back( );
			}

			CPP20CONSTEXPR void clear( ) {
				m_stack.clear( );
			}

			CPP20CONSTEXPR void pop_back( ) {
				m_stack.pop_back( );
			}

			[[nodiscard]] CPP20CONSTEXPR bool empty( ) const {
				return m_stack.empty( );
			}
		};

		template<json_options_t P, typename A,
		         typename StackContainerPolicy = use_default, typename Handler,
		         auto... ParseFlags>
		inline constexpr void
		json_event_parser( basic_json_value<P, A> bjv, Handler &&handler,
		                   options::parse_flags_t<ParseFlags...> ) {

			using ParseState = TryDefaultParsePolicy<typename BasicParsePolicy<
			  P, A>::template SetPolicyOptions<ParseFlags...>>;

			using iterator =
			  basic_json_value_iterator<ParseState::policy_flags( ), A>;
			using json_value_t = typename iterator::json_pair;
			using stack_value_t =
			  JsonEventParserStackValue<ParseState::policy_flags( ), A>;
			auto jvalue = basic_json_value( bjv );

			auto parent_stack = [] {
				if constexpr( std::is_same_v<StackContainerPolicy, use_default> ) {
					return DefaultJsonEventParserStackPolicy<stack_value_t>{ };
				} else {
					return StackContainerPolicy{ };
				}
			}( );
			long long class_depth = 0;
			long long array_depth = 0;

			auto const move_to_last = [&]( ) {
				parent_stack.back( ).value.first = parent_stack.back( ).value.second;
			};

			auto const process_value = [&]( json_value_t p ) {
				{
					auto result = json_details::handle_on_value( handler, p );
					switch( result.value ) {
					case json_parse_handler_result::Complete:
						parent_stack.clear( );
						return;
					case json_parse_handler_result::SkipClassArray:
						move_to_last( );
						return;
					case json_parse_handler_result::Continue:
						break;
					}
				}

				auto &jv = p.value;
				switch( jv.type( ) ) {
				case JsonBaseParseTypes::Array: {
					++array_depth;
					auto result = json_details::handle_on_array_start( handler, jv );
					switch( result.value ) {
					case json_parse_handler_result::Complete:
						parent_stack.clear( );
						return;
					case json_parse_handler_result::SkipClassArray:
						move_to_last( );
						return;
					case json_parse_handler_result::Continue:
						break;
					}
					parent_stack.push_back(
					  { StackParseStateType::Array,
					    std::pair<iterator, iterator>( jv.begin( ), jv.end( ) ) } );
				} break;
				case JsonBaseParseTypes::Class: {
					++class_depth;
					auto result = json_details::handle_on_class_start( handler, jv );
					switch( result.value ) {
					case json_parse_handler_result::Complete:
						parent_stack.clear( );
						return;
					case json_parse_handler_result::SkipClassArray:
						move_to_last( );
						return;
					case json_parse_handler_result::Continue:
						break;
					}
					parent_stack.push_back(
					  { StackParseStateType::Class,
					    std::pair<iterator, iterator>( jv.begin( ), jv.end( ) ) } );
				} break;
				case JsonBaseParseTypes::Number: {
					auto result = json_details::handle_on_number( handler, jv );
					switch( result.value ) {
					case json_parse_handler_result::Complete:
						parent_stack.clear( );
						return;
					case json_parse_handler_result::SkipClassArray:
						move_to_last( );
						return;
					case json_parse_handler_result::Continue:
						break;
					}
				} break;
				case JsonBaseParseTypes::Bool: {
					auto result = json_details::handle_on_bool( handler, jv );
					switch( result.value ) {
					case json_parse_handler_result::Complete:
						parent_stack.clear( );
						return;
					case json_parse_handler_result::SkipClassArray:
						move_to_last( );
						return;
					case json_parse_handler_result::Continue:
						break;
					}
				} break;
				case JsonBaseParseTypes::String: {
					auto result = json_details::handle_on_string( handler, jv );
					switch( result.value ) {
					case json_parse_handler_result::Complete:
						parent_stack.clear( );
						return;
					case json_parse_handler_result::SkipClassArray:
						move_to_last( );
						return;
					case json_parse_handler_result::Continue:
						break;
					}
				} break;
				case JsonBaseParseTypes::Null: {
					auto result = json_details::handle_on_null( handler, jv );
					switch( result.value ) {
					case json_parse_handler_result::Complete:
						parent_stack.clear( );
						return;
					case json_parse_handler_result::SkipClassArray:
						move_to_last( );
						return;
					case json_parse_handler_result::Continue:
						break;
					}
				} break;
				case JsonBaseParseTypes::None:
				default: {
					auto result = json_details::handle_on_error( handler, jv );
					switch( result.value ) {
					case json_parse_handler_result::Complete:
						parent_stack.clear( );
						return;
					case json_parse_handler_result::SkipClassArray:
						move_to_last( );
						return;
					case json_parse_handler_result::Continue:
						break;
					}
				} break;
				}
			};

			auto const process_range = [&]( stack_value_t v ) {
				if( v.value.first != v.value.second ) {
					auto jv = *v.value.first;
					++v.value.first;
					parent_stack.push_back( std::move( v ) );
					process_value( std::move( jv ) );
				} else {
					switch( v.type ) {
					case StackParseStateType::Class: {
						daw_json_assert_weak(
						  ( class_depth > 0 ) &
						    ( v.value.first.get_raw_state( ).has_more( ) and
						      v.value.first.get_raw_state( ).front( ) == '}' ),
						  ErrorReason::InvalidEndOfValue );
						--class_depth;
						auto result = json_details::handle_on_class_end( handler );
						switch( result.value ) {
						case json_parse_handler_result::Complete:
							parent_stack.clear( );
							return;
						case json_parse_handler_result::SkipClassArray:
						case json_parse_handler_result::Continue:
							break;
						}
					} break;
					case StackParseStateType::Array: {
						daw_json_assert_weak(
						  ( array_depth > 0 ) &
						    ( v.value.first.get_raw_state( ).has_more( ) and
						      v.value.first.get_raw_state( ).front( ) == ']' ),
						  ErrorReason::InvalidEndOfValue );
						--array_depth;
						auto result = json_details::handle_on_array_end( handler );
						switch( result.value ) {
						case json_parse_handler_result::Complete:
							parent_stack.clear( );
							return;
						case json_parse_handler_result::SkipClassArray:
						case json_parse_handler_result::Continue:
							break;
						}
					} break;
					}
				}
			};

			process_value( json_value_t{ std::nullopt, std::move( jvalue ) } );

			while( not parent_stack.empty( ) ) {
				auto v = std::move( parent_stack.back( ) );
				parent_stack.pop_back( );
				process_range( v );
			}
			daw_json_ensure( class_depth == 0 and array_depth == 0,
			                 ErrorReason::InvalidEndOfValue );
		}

		template<json_options_t P, typename A,
		         typename StackContainerPolicy = use_default, typename Handler>
		inline constexpr void json_event_parser( basic_json_value<P, A> bjv,
		                                         Handler &&handler ) {
			json_event_parser( std::move( bjv ), DAW_FWD( handler ),
			                   options::parse_flags<> );
		}

		template<typename Handler, auto... ParseFlags>
		inline void
		json_event_parser( daw::string_view json_document, Handler &&handler,
		                   options::parse_flags_t<ParseFlags...> pflags ) {

			return json_event_parser( basic_json_value( json_document ),
			                          DAW_FWD2( Handler, handler ), pflags );
		}

		template<typename Handler>
		inline void json_event_parser( daw::string_view json_document,
		                               Handler &&handler ) {

			return json_event_parser( basic_json_value( json_document ),
			                          DAW_FWD2( Handler, handler ),
			                          options::parse_flags<> );
		}

	} // namespace DAW_JSON_VER
} // namespace daw::json
