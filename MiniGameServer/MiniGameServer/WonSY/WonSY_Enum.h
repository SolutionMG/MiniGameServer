/*
	Copyright 2022, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#pragma once

#ifndef WONSY_ENUM
#define WONSY_ENUM

#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 16

#include "Opensource/magic_enum.hpp"

//#include <WonSY_Attributes.hh>
#define WONSY_LIKELY   [[likely]]
#define WONSY_UNLIKELY [[unlikely]]

#include <string>
#include <vector>
#include <array>

#define EXPEND_ENUM_RANGE( enumName )                                   \
template<>                                                              \
struct magic_enum::customize::enum_range< enumName >                    \
{                                                                       \
	static constexpr int min = 0;                                       \
	static constexpr int max = static_cast< int >( enumName::Max ) + 1; \
};                                                                      \

#pragma region [ Dec ]
namespace WonSY::Enum
{
	/// 인자로 전달받은 Enum값을 문자열로 변환합니다.
	template< typename EnumType >
	std::string ToString( const EnumType enumValue ) noexcept;

	/// 문자열 값을 Enum으로 변환합니다. 문자열에 해당하는 Enum값이 없을 경우, 2번째 인자로 전달된 값을 리턴합니다.
	template< typename EnumType >
	constexpr EnumType ToEnum( std::string_view stringValue ) noexcept;

	/// 인자로 전달받은 Enum Type에 대하여, 해당 Enum의 모든 값들을 Array형태로 반환합니다.
	template< typename EnumType >
	const std::vector< EnumType >& GetEnumValues() noexcept;

	/// 템플릿 인자로 제공된 EnumType의 모든 Enum값들에 인자로 전달된 함수를 실행시킵니다.
	template< typename EnumType, typename CallableType >
	void ForEach( const CallableType& );
}
#pragma endregion

#pragma region [ Def ]
template< typename EnumType >
std::string WonSY::Enum::ToString( const EnumType enumValue ) noexcept
{
	static_assert( std::is_enum< EnumType >::value, "not Enum Type!" );
	static_assert( static_cast< int >( EnumType::Max ) < magic_enum::customize::enum_range< EnumType >::max, "Please Use 'EXPEND_ENUM_RANGE' Macro!" );

	return magic_enum::enum_name< EnumType >( enumValue ).data();
}

template< typename EnumType >
constexpr EnumType WonSY::Enum::ToEnum( std::string_view stringValue ) noexcept
{
	static_assert( std::is_enum< EnumType >::value, "not Enum Type!" );
	static_assert( static_cast< int >( EnumType::Max ) != 0, "Need Max Element And Not Void Enum!" );
	static_assert( static_cast< int >( EnumType::Max ) < magic_enum::customize::enum_range< EnumType >::max, "Please Use 'EXPEND_ENUM_RANGE' Macro!" );

	const std::optional< EnumType > retOptional{ magic_enum::enum_cast< EnumType >( stringValue ) };

	if (retOptional.has_value())
		return retOptional.value();
	else WONSY_UNLIKELY
		return EnumType::Max;
}

template< typename EnumType >
const std::vector< EnumType >& WonSY::Enum::GetEnumValues() noexcept
{
	static_assert( std::is_enum< EnumType >::value, "not Enum Type!" );
	static_assert( static_cast< int >( EnumType::Max ) != 0, "Need Max Element And Not Void Enum!" );
	static_assert( static_cast< int >( EnumType::Max ) < magic_enum::customize::enum_range< EnumType >::max, "Please Use 'EXPEND_ENUM_RANGE' Macro!" );

	static const std::vector< EnumType > enumArray{
		[]() constexpr noexcept 
		{
			constexpr auto ele = magic_enum::enum_values< EnumType >();
			return std::vector< EnumType >( ele.begin(), ele.end() - 1 );
		}() };

	return enumArray;
}

template< typename EnumType, typename CallableType >
void WonSY::Enum::ForEach( const CallableType& func )
{
	// 'static_cast's In WonSY::Enum::GetEnumValues;

	const auto& enumCont{ GetEnumValues< EnumType >() };
	for ( const auto enumValue : enumCont )
	{
		func( enumValue );
	}
}

#pragma endregion

namespace WsyEnum = WonSY::Enum;

#endif