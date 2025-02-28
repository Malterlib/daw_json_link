// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_json_link
//

#pragma once

#include "version.h"

#include "daw_json_default_constuctor.h"
#include "daw_json_enums.h"
#include "daw_json_link_types_aggregate.h"
#include "daw_json_name.h"
#include "daw_json_req_helper.h"
#include <daw/json/concepts/daw_nullable_value.h>
#include <daw/json/daw_json_data_contract.h>

#include <daw/cpp_17.h>
#include <daw/daw_fwd_pack_apply.h>
#include <daw/daw_move.h>
#include <daw/daw_traits.h>

#include <string>
#include <string_view>
#include <type_traits>

namespace daw {
	/// @brief Used to flag that the default will be used.
	struct use_default;
} // namespace daw

/***
 * Customization point traits
 *
 */
namespace daw::json {
	inline namespace DAW_JSON_VER {
		namespace json_details {
			template<template<typename...> typename Trait, typename... Params>
			struct ident_trait {
				using type = Trait<Params...>;
			};

			DAW_JSON_MAKE_REQ_TRAIT( has_op_bool_v,
			                         static_cast<bool>( std::declval<T>( ) ) );

			DAW_JSON_MAKE_REQ_TRAIT( has_op_star_v, *std::declval<T>( ) );

			template<typename Constructor, typename... Args>
			struct constructor_cannot_be_invoked;

			template<typename Constructor, typename... Args>
			struct construction_result
			  : daw::conditional_t<
			      std::is_invocable_v<Constructor, Args...>,
			      std::invoke_result<Constructor, Args...>,
			      daw::traits::identity<
			        constructor_cannot_be_invoked<Constructor, Args...>>> {};
		} // namespace json_details

		namespace json_details {
			template<typename JsonMember>
			using without_name = typename JsonMember::without_name;

			template<typename JsonMember, JSONNAMETYPE NewName, bool Cond>
			using copy_name_when = daw::conditional_t<
			  Cond, typename JsonMember::template with_name<NewName>, JsonMember>;

			template<typename JsonMember, JSONNAMETYPE NewName>
			using copy_name_when_noname =
			  copy_name_when<JsonMember, NewName, is_no_name_v<JsonMember>>;
		} // namespace json_details

		namespace json_details {
			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT(
			  is_json_map_alias_v,
			  json_data_contract_trait_t<T>::i_am_a_json_map_alias );

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT( has_switcher_v, T::switcher );

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT(
			  force_aggregate_construction_test1,
			  json_data_contract<T>::force_aggregate_construction );

			DAW_JSON_MAKE_REQ_TRAIT( force_aggregate_construction_test2,
			                         T::force_aggregate_construction );
		} // namespace json_details
		/***
		 * This trait can be specialized such that when class being returned has
		 * non-move/copyable members the construction can be done with { } instead
		 * of a callable.  This is a blunt object and probably should not be used
		 * add a type alias named force_aggregate_construction to your
		 * json_data_contract specialization
		 * @tparam T type to specialize
		 */
		template<typename T>
		inline constexpr bool force_aggregate_construction_v =
		  json_details::force_aggregate_construction_test1<T> or
		  json_details::force_aggregate_construction_test2<T>;

		namespace json_details {
			template<typename, typename = void>
			struct json_constructor;

			template<typename T>
			struct json_constructor<T, std::void_t<typename T::constructor_t>> {
				using type = typename T::constructor_t;
			};

			template<typename T>
			using json_constructor_t = typename json_constructor<T>::type;

			template<typename, typename = void>
			struct json_result;

			template<typename T>
			struct json_result<T, std::void_t<typename T::parse_to_t>> {
				using type = typename T::parse_to_t;
			};

			template<typename T>
			using json_result_t = typename json_result<T>::type;

			template<typename, typename = void>
			struct json_base_type;

			template<typename T>
			struct json_base_type<T, std::void_t<json_result_t<T>>> {
				using type = json_result_t<T>;
			};

			template<typename T>
			using json_base_type_t = typename json_base_type<T>::type;

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT(
			  is_default_default_constructor_type_v,
			  T::i_am_the_default_default_constructor_type );

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT( has_stateless_allocator_v,
			                                    T::has_stateless_allocator );

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT(
			  has_data_contract_constructor_v, json_data_contract<T>::constructor_t );

			template<typename>
			inline constexpr bool must_be_class_member_v = false;
		} // namespace json_details

		template<typename Constructor, typename T, typename ParseState>
		inline constexpr bool should_construct_explicitly_v =
		  not json_details::has_data_contract_constructor_v<T> and
		  ( force_aggregate_construction_v<T> or
		    json_details::is_default_default_constructor_type_v<Constructor> or
		    not json_details::has_stateless_allocator_v<ParseState> );

		template<typename... Ts>
		inline constexpr bool is_empty_pack_v = sizeof...( Ts ) == 0;

		/// @brief Can use the fast, pseudo random string iterators.  They are
		/// InputIterators with an operator- that allows for O(1) distance
		/// calculations as we often know the length but cannot provide random
		/// access.  For types that only use InputIterator operations and last -
		/// first for distance calc
		///
		template<typename>
		inline constexpr bool can_single_allocation_string_v = false;

		template<typename Char, typename CharTrait, typename Allocator>
		inline constexpr bool can_single_allocation_string_v<
		  std::basic_string<Char, CharTrait, Allocator>> = true;

		template<typename T>
		using can_single_allocation_string =
		  std::bool_constant<can_single_allocation_string_v<T>>;

		namespace json_details {
			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT( is_a_json_type_v,
			                                    T::i_am_a_json_type );

			template<typename... Ts>
			inline constexpr bool are_json_types_v = ( is_a_json_type_v<Ts> and ... );

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT( is_an_ordered_member_v,
			                                    T::i_am_an_ordered_member );

			template<typename T>
			using is_an_ordered_member =
			  std::bool_constant<is_an_ordered_member_v<T>>;

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT( is_a_json_tagged_variant_v,
			                                    T::i_am_a_json_tagged_variant );

			template<typename T>
			using json_class_constructor_t_impl =
			  typename json_data_contract<T>::constructor;

			template<typename T>
			using data_contract_constructor_t =
			  typename json_data_contract<T>::constructor_t;

			template<typename T, typename Default>
			using json_class_constructor_t = daw::detected_or_t<
			  typename daw::conditional_t<
			    std::is_same_v<use_default, Default>,
			    daw::conditional_t<has_data_contract_constructor_v<T>,
			                       ident_trait<data_contract_constructor_t, T>,
			                       ident_trait<default_constructor, T>>,
			    daw::traits::identity<Default>>::type,
			  json_class_constructor_t_impl, T>;

			DAW_JSON_MAKE_REQ_TRAIT( is_string_view_like_v,
			                         ( (void)( std::data( std::declval<T>( ) ) ),
			                           (void)( std::size( std::declval<T>( ) ) ) ) );

			static_assert( is_string_view_like_v<std::string_view> );

		} // namespace json_details

		/***
		 * Trait for passively exploiting the zero termination when the type
		 * guarantees it.
		 */
		template<typename>
		inline constexpr bool is_zero_terminated_string_v = false;

		template<typename CharT, typename Traits, typename Alloc>
		inline constexpr bool
		  is_zero_terminated_string_v<std::basic_string<CharT, Traits, Alloc>> =
		    true;

		template<typename T>
		using is_zero_terminated_string =
		  std::bool_constant<is_zero_terminated_string_v<T>>;

		namespace json_details {
			template<typename ParsePolicy, auto Option>
			using apply_policy_option_t =
			  typename ParsePolicy::template SetPolicyOptions<Option>;

			template<typename ParsePolicy, typename String, auto Option>
			using apply_zstring_policy_option_t = daw::conditional_t<
			  is_zero_terminated_string_v<daw::remove_cvref_t<String>>,
			  apply_policy_option_t<ParsePolicy, Option>, ParsePolicy>;

			template<typename String>
			inline constexpr bool is_mutable_string_v =
			  not std::is_const_v<std::remove_pointer_t<std::remove_reference_t<
			    decltype( std::data( std::declval<String &&>( ) ) )>>>;

			template<typename String>
			constexpr bool is_mutable_string =
			  json_details::is_mutable_string_v<String>;

			template<typename String>
			constexpr bool is_rvalue_string = std::is_rvalue_reference_v<String>;

			template<typename String>
			constexpr bool is_ref_string =
			  not is_rvalue_string<String> and
			  std::is_const_v<std::remove_reference_t<String>>;

			/*
			template<typename ParsePolicy, typename String, auto OptionMutable,
			         auto OptionImmutable>
			using apply_mutable_policy = daw::conditional_t<
			  ParsePolicy::allow_temporarily_mutating_buffer( ),
			  daw::conditional_t<is_mutable_string_v<String>,
			                     apply_policy_option_t<ParsePolicy, OptionMutable>,
			                     apply_policy_option_t<ParsePolicy, OptionImmutable>>,
			  daw::conditional_t<
			    (is_rvalue_string<String> and is_mutable_string_v<String>),
			    apply_policy_option_t<ParsePolicy, OptionMutable>,
			    apply_policy_option_t<ParsePolicy, OptionImmutable>>>;
			    */
		} // namespace json_details

		/***
		 * Ignore unknown members trait allows the parser to skip unknown members
		 * when the default is exact
		 * Set to true when data contract has type alias ignore_unknown_members
		 */
		DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT(
		  ignore_unknown_members_v, json_data_contract<T>::ignore_unknown_members );

		/***
		 * A trait to specify that this class, when parsed, will describe all
		 * members of the JSON object. Anything not mapped is an error.
		 * Either specialize this variable daw::json::is_exact_class_mapping_v, or
		 * have a type in your json_data_contract named exact_class_mapping for your
		 * type
		 */
		DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT(
		  is_exact_class_mapping_v, json_data_contract<T>::exact_class_mapping );

		namespace json_details {
			template<typename T, typename ParseState>
			inline constexpr bool all_json_members_must_exist_v =
			  not ignore_unknown_members_v<T> and
			  ( is_exact_class_mapping_v<T> or
			    ParseState::use_exact_mappings_by_default );

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT( has_element_type, T::element_type );

			template<template<typename...> typename T, typename... Params>
			struct identity_parts {
				using type = T<Params...>;
			};
		} // namespace json_details

		/***
		 * is_pointer_like is used in json_array to ensure that to_json_data returns
		 * a Container/View of the data with the size encoded with it.
		 * The std
		 */
		template<typename T>
		inline constexpr bool is_pointer_like_v =
		  std::is_pointer_v<T> or json_details::has_element_type<T>;

		/// Allow tuple like types to be used in json_tuple
		/// \tparam Tuple tuple like type to
		template<typename Tuple, typename = void>
		struct tuple_elements_pack;

		template<typename... Ts>
		struct tuple_elements_pack<std::tuple<Ts...>> {
			using type = std::tuple<Ts...>;

			static constexpr std::size_t size = sizeof...( Ts );

			template<std::size_t Idx>
			using element_t = std::tuple_element_t<Idx, type>;

			template<std::size_t Idx, typename Tuple>
			static constexpr decltype( auto ) get( Tuple &&tp ) {
				return std::get<Idx>( DAW_FWD( tp ) );
			}
		};

		template<typename... Ts>
		struct tuple_elements_pack<daw::fwd_pack<Ts...>> {
			using type = daw::fwd_pack<Ts...>;

			static constexpr std::size_t size = sizeof...( Ts );

			template<std::size_t Idx>
			using element_t =
			  daw::remove_cvref_t<typename daw::tuple_element<Idx, type>::type>;

			template<std::size_t Idx, typename Tuple>
			static constexpr decltype( auto ) get( Tuple &&tp ) {
				return DAW_FWD( tp ).template get<Idx>( );
			}
		};

		/// @brief Is the type pinned in memory and unable to be copied/moved after
		/// construction(e.g. std::mutex).  These types require using RVO in order
		/// to be used but that can have a penalty as std::current_exceptions( )
		/// must be checked which is quite noticeable on MSVC.
		/// @tparam T type to check
		template<typename T>
		inline constexpr bool is_pinned_type_v = not(
		  (std::is_copy_constructible_v<T> and std::is_copy_assignable_v<T>) or
		  ( std::is_move_constructible_v<T> and std::is_move_assignable_v<T> ) );

		namespace json_details {
			template<typename, typename = void>
			inline constexpr bool is_tuple_v = false;

			template<typename... Ts>
			inline constexpr bool is_tuple_v<std::tuple<Ts...>> = true;

			template<typename T>
			inline constexpr bool
			  is_tuple_v<T, typename tuple_elements_pack<T>::type> = true;

			template<typename T>
			using unwrapped_t = concepts::nullable_value_type_t<T>;

			template<typename T>
			using mapped_type_t = typename T::mapped_type;

			template<typename T>
			using key_type_t = typename T::key_type;

// DAW disabling to fix #357
#if true or defined( DAW_JSON_DISABLE_RANDOM )
			template<bool>
			inline constexpr bool can_be_random_iterator_v = false;
#else
			template<bool IsKnown>
			inline constexpr bool can_be_random_iterator_v = IsKnown;
#endif

			DAW_JSON_MAKE_REQ_TYPE_ALIAS_TRAIT( is_literal_json_type_v,
			                                    T::as_string );

			template<typename JsonMember>
			using literal_json_type_as_string = typename JsonMember::as_string;

			template<typename, typename = void>
			inline constexpr bool is_deduced_empty_class_v = false;
		} // namespace json_details
	} // namespace DAW_JSON_VER
} // namespace daw::json
