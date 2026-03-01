// ======================================================================
//
// hash_compat.h
// Compatibility layer for x64 builds: maps STLport hash_map/hash_set
// to C++11 unordered_map/unordered_set from the MSVC standard library.
//
// ======================================================================

#ifndef INCLUDED_hash_compat_H
#define INCLUDED_hash_compat_H

#ifdef _M_X64

#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace std
{
	template <class _Key,
	          class _Tp,
	          class _HashFcn = std::hash<_Key>,
	          class _EqualKey = std::equal_to<_Key>,
	          class _Alloc = std::allocator<std::pair<const _Key, _Tp> > >
	class hash_map : public std::unordered_map<_Key, _Tp, _HashFcn, _EqualKey, _Alloc>
	{
	public:
		typedef std::unordered_map<_Key, _Tp, _HashFcn, _EqualKey, _Alloc> base_type;
		using base_type::base_type;
	};

	template <class _Key,
	          class _HashFcn = std::hash<_Key>,
	          class _EqualKey = std::equal_to<_Key>,
	          class _Alloc = std::allocator<_Key> >
	class hash_set : public std::unordered_set<_Key, _HashFcn, _EqualKey, _Alloc>
	{
	public:
		typedef std::unordered_set<_Key, _HashFcn, _EqualKey, _Alloc> base_type;
		using base_type::base_type;
	};
}

#endif // _M_X64

#endif // INCLUDED_hash_compat_H
