#pragma once

#include "base/base_libraries.h"

namespace mylib
{
	template <class T, size_t N>
	using array_t = std::array<T, N>;

	template <typename T, typename _Alloc = std::allocator<T>>
	using vector_t = std::vector<T, _Alloc>;

	template <typename T, typename _Alloc = std::allocator<T>>
	using list_t = std::list<T, _Alloc>;

	template <class T,
		class _Pr = std::less<T>,
		class _Alloc = std::allocator<T>>
		using multiset_t = std::multiset<T, _Pr, _Alloc>;

	template <class K, class V,
		class Hash = std::hash<K>,
		class Eq = std::equal_to<K>,
		class Allocator = std::allocator<std::pair<const K, V>>>
		using hash_map_t = std::unordered_map<K, V, Hash, Eq, Allocator>;

	template <class T, 
		class _Container = std::vector<T>, 
		class _Pr = std::less<typename _Container::value_type>>
		using pr_queue = std::priority_queue<T,_Container,_Pr>;

	using str_t = std::string;
	using strview_t = std::string_view;

	template <std::output_iterator<const char&> _OutputIt>
	inline _OutputIt format_args_to(_OutputIt _Out, std::string_view _Fmt, std::format_args _Args) {
		std::_Fmt_iterator_buffer<_OutputIt, char> _Buf(_STD move(_Out));
		_STD vformat_to(std::_Fmt_it{ _Buf }, _Fmt, _Args);
		return _Buf._Out();
	}
}