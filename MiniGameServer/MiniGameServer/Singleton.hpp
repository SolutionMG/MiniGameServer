#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <type_traits>

namespace Base
{
	//기본생성자를 꼭 만들도록 함
	template<class T, class = std::void_t<decltype(std::declval<T>())>>
	class TSingleton
	{
	public:
		static T& GetInstance( )
		{
			static T instance;
			return instance;
		}
	protected:
		TSingleton( ) = default;
		~TSingleton( ) = default;
	public:
		TSingleton( TSingleton const& ) = delete;
		TSingleton& operator=( TSingleton const& ) = delete;

		TSingleton( TSingleton&& ) = delete;
		TSingleton& operator=( TSingleton&& ) = delete;
	};
}

#endif