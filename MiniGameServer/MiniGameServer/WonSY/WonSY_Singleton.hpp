/*
	Copyright 2019, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#pragma once

#define WONSY_SINGLETONE

#include <type_traits>

namespace WonSY 
{
	inline namespace Base
	{
		template< 
			typename T, 
			typename = std::void_t< decltype( std::declval< T >() ) > /* = default ctor */ >
		class TSingleton
		{
		public:
			__forceinline static T& GetInstance()
			{
				static T instance;
				return instance;
			}

		protected:
			TSingleton()  = default;
			~TSingleton() = default;

		public:
			TSingleton( TSingleton const& )            = delete;
			TSingleton& operator=( TSingleton const& ) = delete;

			TSingleton( TSingleton&& )            = delete;
			TSingleton& operator=( TSingleton&& ) = delete;
		};

		//template< typename T >
		//class TManualSingleton
		//{
		//public:
		//	__forceinline static T& GetInstance()
		//	{
		//		return m_instance;
		//	}
		//
		//	template< typename... Parameter >
		//	void MakeInstance( Parameter&&... parameter )
		//	{
		//		m_instance = T( std::forward< Parameter >( parameter ) );
		//	}
		//
		//private:
		//	static T m_instance;
		//
		//	TManualSingleton() = default;
		//	~TManualSingleton() = default;
		//
		//public:
		//	TManualSingleton( TManualSingleton const& ) = delete;
		//	TManualSingleton& operator=( TManualSingleton const& ) = delete;
		//
		//	TManualSingleton( TManualSingleton&& ) = delete;
		//	TManualSingleton& operator=( TManualSingleton&& ) = delete;
		//};
	}
}

template < typename T >
using WsySingleton = WonSY::Base::TSingleton< T >;

//template < typename T >
//using WsyManualSingleton = WonSY::Base::TManualSingleton< T >;
