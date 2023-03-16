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
	/// ���ڷ� ���޹��� Enum���� ���ڿ��� ��ȯ�մϴ�.
	template< typename EnumType >
	std::string ToString( const EnumType enumValue ) noexcept;

	/// ���ڿ� ���� Enum���� ��ȯ�մϴ�. ���ڿ��� �ش��ϴ� Enum���� ���� ���, 2��° ���ڷ� ���޵� ���� �����մϴ�.
	template< typename EnumType >
	constexpr EnumType ToEnum( std::string_view stringValue ) noexcept;

	/// ���ڷ� ���޹��� Enum Type�� ���Ͽ�, �ش� Enum�� ��� ������ Array���·� ��ȯ�մϴ�.
	template< typename EnumType >
	const std::vector< EnumType >& GetEnumValues() noexcept;

	/// ���ø� ���ڷ� ������ EnumType�� ��� Enum���鿡 ���ڷ� ���޵� �Լ��� �����ŵ�ϴ�.
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