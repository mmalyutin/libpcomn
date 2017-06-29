/*-*- mode: c++; tab-width: 3; indent-tabs-mode: nil; c-file-style: "ellemtel"; c-file-offsets:((innamespace . 0)(inclass . ++)) -*-*/
#ifndef __PCOMN_HASH_H
#define __PCOMN_HASH_H
/*******************************************************************************
 FILE         :   pcomn_hash.h
 COPYRIGHT    :   Yakov Markovitch, 2000-2016. All rights reserved.
                  See LICENSE for information on usage/redistribution.

 DESCRIPTION  :   Hash functions & functors.
                  MD5 hash.
                  CRC32 function(s)

 CREATION DATE:   10 Jan 2000
*******************************************************************************/
/** @file
 Hash functions and functors, MD5 hash, CRC32 calculations.
*******************************************************************************/
#include <pcommon.h>
#include <pcomn_assert.h>

#include "t1ha/t1ha.h"

#include <wchar.h>

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

_PCOMNEXP uint32_t calc_crc32(uint32_t crc, const void *buf, size_t len) ;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <pcomn_meta.h>

#include <string>
#include <iosfwd>
#include <functional>
#include <utility>
#include <tuple>

namespace pcomn {

/*******************************************************************************
 Overloaded generic hash algorithms
*******************************************************************************/
inline uint32_t calc_crc32(uint32_t prev_crc, const char *str)
{
   return ::calc_crc32(prev_crc, (const uint8_t *)str, strlen(str)) ;
}

inline uint32_t calc_crc32(const char *str)
{
   return calc_crc32(0, str) ;
}

inline uint32_t calc_crc32(uint32_t prev_crc, const void *buf, size_t sz)
{
   return ::calc_crc32(prev_crc, buf, sz) ;
}

/*******************************************************************************/
/** Fowler/Noll/Vo (FNV) hash.
*******************************************************************************/
inline uint32_t hashFNV32(const void *data, size_t size, uint32_t init)
{
	uint32_t result = init ;
	for (const uint8_t *p = static_cast<const uint8_t *>(data), *end = p + size ; p != end ; ++p)
   {
      result ^= uint32_t(*p) ;
      result *= uint32_t(16777619UL) ;
   }
	return result ;
}

inline uint32_t hashFNV32(const void *data, size_t size)
{
   return hashFNV32(data, size, 2166136261UL) ;
}

inline uint64_t hashFNV64(const void *data, size_t size, uint64_t init)
{
	uint64_t result = init ;
	for (const uint8_t *p = static_cast<const uint8_t *>(data), *end = p + size ; p != end ; ++p)
   {
      result ^= uint64_t(*p) ;
      result *= uint64_t(1099511628211ULL) ;
   }
	return result ;
}

inline uint64_t hashFNV64(const void *data, size_t size)
{
   return hashFNV64(data, size, 14695981039346656037ULL) ;
}

inline uint32_t hashFNV(const void *data, size_t size, std::integral_constant<size_t, 4>)
{
   return hashFNV32(data, size) ;
}

inline uint32_t hashFNV(const void *data, size_t size, uint32_t init, std::integral_constant<size_t, 4>)
{
   return hashFNV32(data, size, init) ;
}

inline uint64_t hashFNV(const void *data, size_t size, std::integral_constant<size_t, 8>)
{
   return hashFNV64(data, size) ;
}

inline uint64_t hashFNV(const void *data, size_t size, uint64_t init, std::integral_constant<size_t, 8>)
{
   return hashFNV64(data, size, init) ;
}

inline size_t hashFNV(const void *data, size_t size)
{
   return hashFNV(data, size, std::integral_constant<size_t, sizeof(size_t)>()) ;
}

inline size_t hashFNV(const void *data, size_t size, size_t init)
{
   return hashFNV(data, size, init, std::integral_constant<size_t, sizeof(size_t)>()) ;
}

/******************************************************************************/
/** Robert Jenkins' hash function for hashing a 32-bit integer to a 32-bit integer
*******************************************************************************/
inline uint32_t jenkins_hash32to32(uint32_t x)
{
   x = (x+0x7ed55d16) + (x<<12) ;
   x = (x^0xc761c23c) ^ (x>>19) ;
   x = (x+0x165667b1) + (x<<5) ;
   x = (x+0xd3a2646c) ^ (x<<9) ;
   x = (x+0xfd7046c5) + (x<<3) ;
   x = (x^0xb55a4f09) ^ (x>>16) ;
   return x ;
}

/******************************************************************************/
/** Thomas Wang's hash function for hashing a 64-bit integer to a 64-bit integer
*******************************************************************************/
inline uint64_t wang_hash64to64(uint64_t x)
{
  x = ~x + (x << 21) ; // x = (x << 21) - x - 1
  x = x ^ (x >> 24) ;
  x = x + (x << 3) + (x << 8) ; // x * 265
  x = x ^ (x >> 14) ;
  x = x + (x << 2) + (x << 4) ; // x * 21
  x = x ^ (x >> 28) ;
  x = x + (x << 31) ;
  return x ;
}

/******************************************************************************/
/** Thomas Wang's hash function for hashing a 64-bit integer to a 32-bit integer
*******************************************************************************/
inline uint32_t wang_hash64to32(uint64_t x)
{
  x = ~x + (x << 18) ; // x = (x << 18) - x - 1
  x = x ^ (x >> 31) ;
  x = x + (x << 2) + (x << 4) ; // x * 21
  x = x ^ (x >> 11) ;
  x = x + (x << 6) ;
  x = x ^ (x >> 22) ;
  return (uint32_t)x ;
}

inline uint32_t hash_64(uint64_t x, std::integral_constant<size_t, 4>)
{
   return wang_hash64to32(x) ;
}

inline uint64_t hash_64(uint64_t x, std::integral_constant<size_t, 8>)
{
   return wang_hash64to64(x) ;
}

inline uint32_t hash_32(uint32_t x, std::integral_constant<size_t, 4>)
{
   return jenkins_hash32to32(x) ;
}

inline uint64_t hash_32(uint32_t x, std::integral_constant<size_t, 8>)
{
   return wang_hash64to64(x) ;
}

/***************************************************************************//**
 Hashing 32-bit integer to size_t
*******************************************************************************/
inline size_t hash_32(uint32_t x)
{
   return hash_32(x, std::integral_constant<size_t, sizeof(size_t)>()) ;
}

/***************************************************************************//**
 Hashing 64-bit integer to size_t
*******************************************************************************/
inline size_t hash_64(uint64_t x)
{
   return hash_64(x, std::integral_constant<size_t, sizeof(size_t)>()) ;
}

/***************************************************************************//**
 Hashing byte sequence to size_t.
 @note Always returns 0 for zero length.
*******************************************************************************/
inline size_t hash_bytes(const void *data, size_t length)
{
   return length ? t1ha0(data, length, 0) : 0 ;
}

/*******************************************************************************
 Hasher functors
*******************************************************************************/
/** Functor returning its parameter unchanged (but converted to size_t).
*******************************************************************************/
template<typename T>
struct hash_identity : public std::unary_function<T, size_t> {
      size_t operator() (const T &val) const { return static_cast<size_t>(val) ; }
} ;

/*******************************************************************************
 Forward declarations of hash functor templates
*******************************************************************************/
template<typename> struct hash_fn ;

// "Unwrap" type being hashed from const, volatile, and reference wrappers.
template<typename T> struct hash_fn<const T> : hash_fn<T> {} ;
template<typename T> struct hash_fn<volatile T> : hash_fn<T> {} ;
template<typename T> struct hash_fn<T &> : hash_fn<T> {} ;
template<typename T> struct hash_fn<T &&> : hash_fn<T> {} ;
template<typename T> struct hash_fn<const T *> : hash_fn<T *> {} ;
template<typename T>
struct hash_fn<std::reference_wrapper<T>> : hash_fn<T> {} ;

/*******************************************************************************
 Universal hashing function
*******************************************************************************/
template<typename T>
inline decltype(hash_fn<std::decay_t<T>>()(std::declval<T>())) hasher(T &&data)
{
   return hash_fn<std::decay_t<T>>()(std::forward<T>(data)) ;
}

/******************************************************************************/
/** Hash accumulator: "reduces" a sequence of hash value to single hash value.
 Useful for hashing structures and sequences (i.e. complex objects).
 The algorith is from Python's tuplehash.
*******************************************************************************/
struct tuple_hash {
      constexpr tuple_hash() = default ;

      explicit constexpr tuple_hash(uint64_t initial_hash) :
         _accumulator((uint64_t)0x31E9D059168ULL ^ initial_hash),
         _count(1)
      {}

      template<typename InputIterator>
      tuple_hash(const InputIterator &b, const InputIterator &e) : tuple_hash()
      {
         append_data(b, e) ;
      }

      constexpr uint64_t value() const { return _accumulator ^ _count ; }
      constexpr operator uint64_t() const { return value() ; }

      tuple_hash &append(uint64_t hash)
      {
         _accumulator = (1000003 * _accumulator) ^ hash ;
         ++_count ;
         return *this ;
      }

      template<typename T>
      tuple_hash &append_data(T &&data) { return append(hasher(std::forward<T>(data))) ; }

      template<typename InputIterator>
      tuple_hash &append_data(InputIterator b, InputIterator e)
      {
         for (; b != e ; *this += *b, ++b) ;
         return *this ;
      }

      template<typename T>
      tuple_hash &operator+=(T &&data) { return append_data(std::forward<T>(data)) ; }

      template<typename...T>
      tuple_hash &append_as_tuple(const std::tuple<T...> &data)
      {
         append_item<sizeof...(T)>::append(*this, data) ;
         return this ;
      }

   private:
      uint64_t _accumulator = 0x345678L ;
      uint64_t _count       = 0 ;

      template<unsigned tuplesz, typename=void>
      struct append_item {
            template<typename T>
            static void append(tuple_hash &accumulator, T &&tuple_data)
            {
               append_item<tuplesz-1>::append(accumulator, std::forward<T>(tuple_data)) ;
               accumulator.append(std::get<tuplesz-1>(std::forward<T>(tuple_data))) ;
            }
      } ;
      template<typename D> struct append_item<0, D> {
            template<typename...Args> static void append(Args &&...) {}
      } ;
} ;

/***************************************************************************//**
 Function that hashes its arguments and returns combined hash value.
*******************************************************************************/
inline size_t tuplehasher() { return 0 ; }

template<typename A1>
inline size_t tuplehasher(A1 &&a1)
{
   return tuple_hash(hasher(std::forward<A1>(a1))) ;
}

template<typename A1, typename A2>
inline size_t tuplehasher(A1 &&a1, A2 &&a2)
{
   return tuple_hash(hasher(std::forward<A1>(a1))).append_data(std::forward<A2>(a2)) ;
}

template<typename A1, typename A2, typename A3, typename... Args>
inline size_t tuplehasher(A1 &&a1, A2 &&a2, A3 &&a3, Args &&...args)
{
   tuple_hash result (hasher(std::forward<A1>(a1))) ;
   result.append_data(std::forward<A2>(a2)).append_data(std::forward<A2>(a3)) ;

   const size_t h[] = { hasher(std::forward<Args>(args))...} ;
   for (size_t v: h) result.append(v) ;
   return result ;
}

/******************************************************************************/
/** Cryptographic non-POD hash (has constructors)
*******************************************************************************/
template<typename T>
struct crypthash : T {
      /// Creates a hash filled with zeros
      crypthash() { T::init() ; }
      /// Creates a hash from a hex string representation
      explicit crypthash(const char *hashstr) { T::init(hashstr) ; }
} ;

/******************************************************************************/
/** 128-bit aligned binary big-endian POD data
*******************************************************************************/
struct binary128_t {

      constexpr binary128_t() : _idata() {}

      /// Check helper
      explicit constexpr operator bool() const { return !!(_idata[0] | _idata[1]) ; }

      unsigned char *data() { return reinterpret_cast<unsigned char *>(&_idata) ; }
      constexpr const unsigned char *data() const
      {
         return (const unsigned char *)(const void *)&_idata ;
      }

      static constexpr size_t size() { return sizeof _idata ; }
      static constexpr size_t slen() { return 2*size() ; }

      _PCOMNEXP std::string to_string() const ;
      _PCOMNEXP char *to_strbuf(char *buf) const ;

      friend bool operator==(const binary128_t &l, const binary128_t &r)
      {
         return !((l._idata[0] ^ r._idata[0]) | (l._idata[1] ^ r._idata[1])) ;
      }

      friend bool operator<(const binary128_t &l, const binary128_t &r)
      {
         return
            value_from_big_endian(l._idata[0]) < value_from_big_endian(r._idata[0]) ||
            l._idata[0] == r._idata[0] && value_from_big_endian(l._idata[1]) < value_from_big_endian( r._idata[1]) ;
      }

      constexpr size_t hash() const { return _idata[0] ^ _idata[1] ; }

   protected:
      /// Create a hash filled with zeros.
      void init() { _idata[1] = _idata[0] = 0 ; }
      void init(const char *hexstr)
      {
         init() ;
         uint64_t idata[2] ;
         if (hextob(idata, sizeof idata, hexstr))
         {
            _idata[0] = idata[0] ;
            _idata[1] = idata[1] ;
         }
      }

   private:
      uint64_t _idata[2] ;
} ;

PCOMN_STATIC_CHECK(sizeof(binary128_t) == 16) ;

// Define !=, >, <=, >= for binary128_t
PCOMN_DEFINE_RELOP_FUNCTIONS(, binary128_t) ;

template<typename T>
struct is_literal128 :
   std::bool_constant<sizeof(T) == sizeof(binary128_t) && std::is_literal_type<binary128_t>::value>
{} ;


template<typename T>
inline std::enable_if_t<is_literal128<T>::value, T *> cast128(binary128_t *v)
{
   return reinterpret_cast<T *>(v) ;
}

template<typename T>
inline std::enable_if_t<is_literal128<T>::value, const T *> cast128(const binary128_t *v)
{
   return reinterpret_cast<const T *>(v) ;
}

template<typename T>
inline std::enable_if_t<is_literal128<T>::value, T &> cast128(binary128_t &v)
{
   return *cast128<T>(&v) ;
}

template<typename T>
inline std::enable_if_t<is_literal128<T>::value, const T &> cast128(const binary128_t &v)
{
   return *cast128<T>(&v) ;
}

template<typename T>
inline std::enable_if_t<is_literal128<T>::value, T &&> cast128(binary128_t &&v)
{
   return std::move(*reinterpret_cast<T *>(&v)) ;
}

/******************************************************************************/
/** MD5 hash POD type (no constructors, no destructors)
*******************************************************************************/
struct md5hash_pod_t : binary128_t {} ;

/******************************************************************************/
/** SHA1 hash POD type (no constructors, no destructors)
*******************************************************************************/
struct sha1hash_pod_t {

      constexpr sha1hash_pod_t() : _idata() {}

      /// Check helper.
      explicit constexpr operator bool() const
      {
         return _idata[0] || _idata[1] || _idata[2] || _idata[3] || _idata[4] ;
      }

      unsigned char *data() { return reinterpret_cast<unsigned char *>(_idata) ; }
      const unsigned char *data() const { return reinterpret_cast<const unsigned char *>(_idata) ; }

      static size_t size() { return sizeof _idata ; }

      _PCOMNEXP std::string to_string() const ;

      friend bool operator==(const sha1hash_pod_t &l, const sha1hash_pod_t &r)
      {
         return !memcmp(l._idata, r._idata, sizeof l._idata) ;
      }

      friend bool operator<(const sha1hash_pod_t &l, const sha1hash_pod_t &r)
      {
         return memcmp(l._idata, r._idata, sizeof l._idata) < 0 ;
      }

      size_t hash() const
      {
         return ((uint64_t)_idata[3] << 32) | (uint64_t)_idata[4] ;
      }

   protected:
      void init() { _idata[4] = _idata[3] = _idata[2] = _idata[1] = _idata[0] = 0 ; }
      void init(const char *hexstr)
      {
         init() ;
         uint32_t idata[5] ;
         if (hextob(idata, sizeof idata, hexstr))
         {
            _idata[0] = idata[0] ;
            _idata[1] = idata[1] ;
            _idata[2] = idata[2] ;
            _idata[3] = idata[3] ;
            _idata[4] = idata[4] ;
         }
      }

   private:
      uint32_t _idata[5] ;
} ;

PCOMN_STATIC_CHECK(sizeof(sha1hash_pod_t) == 20) ;

// Define !=, >, <=, >= for sha1hash_pod_t
PCOMN_DEFINE_RELOP_FUNCTIONS(, sha1hash_pod_t) ;

/******************************************************************************/
/** MD5 hash (has constructors)
*******************************************************************************/
typedef crypthash<md5hash_pod_t> md5hash_t ;

/******************************************************************************/
/** SHA1 hash non-POD (has constructors)
*******************************************************************************/
typedef crypthash<sha1hash_pod_t> sha1hash_t ;

/*******************************************************************************

*******************************************************************************/
/// @cond
namespace detail {
template<size_t n>
struct crypthash_state {
      size_t   _size ;
      uint64_t _statebuf[n] ;

      bool is_init() const { return *_statebuf || _size ; }
} ;

}
/// @endcond

/******************************************************************************/
/** MD5 hash accumulator: calculates MD5 incrementally.
*******************************************************************************/
class _PCOMNEXP MD5Hash {
   public:
      MD5Hash() { memset(&_state, 0, sizeof _state) ; }

      md5hash_t value() const ;
      operator md5hash_t() const { return value() ; }

      /// Data size appended so far
      size_t size() const { return _state._size ; }

      MD5Hash &append_data(const void *buf, size_t size) ;
      MD5Hash &append_file(const char *filename) ;
      MD5Hash &append_file(FILE *file) ;

   private:
      typedef detail::crypthash_state<24> state ;
      state _state ;
} ;

/// Create zero MD5.
inline md5hash_t md5hash() { return md5hash_t() ; }

/// Compute MD5 for a buffer.
///
/// @note For convenient overloadings of this function see pcomn_strslice.h (md5 for any
/// string) and pcomn_buffer.h (md5 for any buffer)
///
_PCOMNEXP md5hash_t md5hash(const void *buf, size_t size) ;
/// Compute MD5 for a file specified by a filename.
/// @param filename     The name of file to md5.
/// @param raise_error  Whether to throw pcomn::system_error exception on I/O error
/// (e.g., @a filename does not exist), if DONT_RAISE_ERROR then return zero md5.
/// @throw std::invalid_argument if @a filename is NULL.
_PCOMNEXP md5hash_t md5hash_file(const char *filename, size_t *size, RaiseError raise_error = DONT_RAISE_ERROR) ;
/// @overload
_PCOMNEXP md5hash_t md5hash_file(int fd, size_t *size, RaiseError raise_error = DONT_RAISE_ERROR) ;
/// @overload
inline md5hash_t md5hash_file(const char *filename, RaiseError raise_error = DONT_RAISE_ERROR)
{
   return md5hash_file(filename, NULL, raise_error) ;
}
/// @overload
inline md5hash_t md5hash_file(int fd, RaiseError raise_error = DONT_RAISE_ERROR)
{
   return md5hash_file(fd, NULL, raise_error) ;
}

/// Compute MD5 for a file opened for reading.
/// @note Doesn't rewind file before starting calculation.
inline md5hash_t md5hash_file(FILE *file, size_t *size = NULL)
{
   MD5Hash crypthasher ;
   crypthasher.append_file(file) ;
   if (size)
      *size = crypthasher.size() ;
   return crypthasher.value() ;
}

/******************************************************************************/
/** SHA1 hash accumulator: calculates SHA1 incrementally.
*******************************************************************************/
class _PCOMNEXP SHA1Hash {
   public:
      SHA1Hash() { memset(&_state, 0, sizeof _state) ; }

      sha1hash_t value() const ;
      operator sha1hash_t() const { return value() ; }

      /// Data size appended so far
      size_t size() const { return _state._size ; }

      SHA1Hash &append_data(const void *buf, size_t size) ;
      SHA1Hash &append_file(const char *filename) ;
      SHA1Hash &append_file(FILE *file) ;

   private:
      typedef detail::crypthash_state<24> state ;
      state _state ;
} ;

/// Create all-zero SHA1.
inline sha1hash_t sha1hash() { return sha1hash_t() ; }

/// Compute SHA1 for a buffer.
///
/// @note For convenient overloadings of this function see pcomn_strslice.h (md5 for any
/// string) and pcomn_buffer.h (md5 for any buffer)
///
_PCOMNEXP sha1hash_t sha1hash(const void *buf, size_t size) ;
/// Compute SHA1 for a file specified by a filename.
/// @param filename     The name of file to md5.
/// @param raise_error  Whether to throw pcomn::system_error exception on I/O error
/// (e.g., @a filename does not exist), if DONT_RAISE_ERROR then return zero md5.
/// @throw std::invalid_argument if @a filename is NULL.
_PCOMNEXP sha1hash_t sha1hash_file(const char *filename, size_t *size, RaiseError raise_error = DONT_RAISE_ERROR) ;
/// @overload
_PCOMNEXP sha1hash_t sha1hash_file(int fd, size_t *size, RaiseError raise_error = DONT_RAISE_ERROR) ;
/// @overload
inline sha1hash_t sha1hash_file(const char *filename, RaiseError raise_error = DONT_RAISE_ERROR)
{
   return sha1hash_file(filename, NULL, raise_error) ;
}
/// @overload
inline sha1hash_t sha1hash_file(int fd, RaiseError raise_error = DONT_RAISE_ERROR)
{
   return sha1hash_file(fd, NULL, raise_error) ;
}

/// Compute SHA1 for a file opened for reading.
/// @note Doesn't rewind file before starting calculation.
inline sha1hash_t sha1hash_file(FILE *file, size_t *size = NULL)
{
   SHA1Hash crypthasher ;
   crypthasher.append_file(file) ;
   if (size)
      *size = crypthasher.size() ;
   return crypthasher.value() ;
}

/******************************************************************************/
/** Generic hashing functor.
 Uses pcomn::hasher() function for hashing values.
*******************************************************************************/
template<typename T> struct hash_fn : std::hash<T> {} ;

template<> struct hash_fn<bool> { size_t operator()(bool v) const { return v ; } } ;

template<typename T>
struct hash_fundamental {
      PCOMN_STATIC_CHECK(std::is_fundamental<T>::value) ;

      size_t operator()(T value) const { return hash_64((uint64_t)value) ; }
} ;

template<> struct hash_fn<double> {
      size_t operator()(double value) const
      {
         return hash_64(*reinterpret_cast<uint64_t *>(&value)) ;
      }
} ;

template<> struct hash_fn<void *> {
      size_t operator()(const void *value) const
      {
         return hash_64(*reinterpret_cast<uint64_t *>(&value)) ;
      }
} ;

template<> struct hash_fn<float> : hash_fn<double> {} ;
template<> struct hash_fn<long double> : hash_fn<double> {} ;

#define PCOMN_HASH_INT_FN(type) template<> struct hash_fn<type> : hash_fundamental<type> {}

PCOMN_HASH_INT_FN(char) ;
PCOMN_HASH_INT_FN(unsigned char) ;
PCOMN_HASH_INT_FN(signed char) ;
PCOMN_HASH_INT_FN(short) ;
PCOMN_HASH_INT_FN(unsigned short) ;
PCOMN_HASH_INT_FN(int) ;
PCOMN_HASH_INT_FN(unsigned) ;
PCOMN_HASH_INT_FN(long) ;
PCOMN_HASH_INT_FN(unsigned long) ;
PCOMN_HASH_INT_FN(long long) ;
PCOMN_HASH_INT_FN(unsigned long long) ;

/***************************************************************************//**
 tuple_hash C string, return 0 for NULL and empty string
*******************************************************************************/
template<> struct hash_fn<char *> {
      size_t operator()(const char *s) const
      {
         return hash_bytes(s, s ? strlen(s) : 0) ;
      }
} ;

template<> struct hash_fn<std::string> {
      size_t operator()(const std::string &s) const
      {
         return hash_bytes(s.data(), s.size()) ;
      }
} ;

template<typename T1, typename T2>
struct hash_fn<std::pair<T1, T2>> {
      size_t operator()(const std::pair<T1, T2> &data) const
      {
         return tuplehasher(data.first, data.second) ;
      }
} ;

template<>
struct hash_fn<std::tuple<>> {
      size_t operator()(const std::tuple<> &) const { return 0 ; }
} ;

template<typename T1, typename...T>
struct hash_fn<std::tuple<T1, T...>> {
      size_t operator()(const std::tuple<T1, T...> &data) const
      {
         return tuple_hash().append_as_tuple(data) ;
      }
} ;

/******************************************************************************/
/** Hashing functor for any sequence
*******************************************************************************/
template<typename T, typename H = hash_fn<T>>
struct hash_fn_seq : private H {
      template<typename  S>
      size_t operator()(S &&sequence) const
      {
         tuple_hash accumulator ;
         for (auto &v: sequence)
            accumulator.append(H::operator()(v)) ;
         return accumulator ;
      }

      template<typename InputIterator>
      size_t operator()(InputIterator b, InputIterator e) const
      {
         tuple_hash accumulator ;
         for ( ; b != e ; ++b)
            accumulator.append(H::operator()(*b)) ;
         return accumulator ;
      }
} ;

template<typename T>
struct hash_fn_member : public std::unary_function<T, size_t> {
      size_t operator()(const T &instance) const { return instance.hash() ; }
} ;

/*******************************************************************************
 ostream
*******************************************************************************/
std::ostream &operator<<(std::ostream &, const binary128_t &) ;
std::ostream &operator<<(std::ostream &, const sha1hash_pod_t &) ;

} // end of namespace pcomn

namespace std {
/*******************************************************************************
 std::hash specializations for cryptohashes
*******************************************************************************/
template<> struct hash<pcomn::binary128_t>    : pcomn::hash_fn_member<pcomn::binary128_t> {} ;
template<> struct hash<pcomn::md5hash_pod_t>  : pcomn::hash_fn_member<pcomn::md5hash_pod_t> {} ;
template<> struct hash<pcomn::sha1hash_pod_t> : pcomn::hash_fn_member<pcomn::sha1hash_pod_t> {} ;

template<typename T>
struct hash<pcomn::crypthash<T> > : public hash<T> {} ;
}

#endif /* __cplusplus */

#endif /* __PCOMN_HASH_H */

/*******************************************************************************
 Hashing buffers
 For this to work, include pcomn_buffer.h before pcomn_hash.h
*******************************************************************************/
#if defined(__cplusplus) && defined(__PCOMN_BUFFER_H) && !defined(__PCOMN_BUFFER_HASH_H)
#define __PCOMN_BUFFER_HASH_H

namespace pcomn {

template<typename B>
inline typename enable_if_buffer<B, uint32_t>::type
calc_crc32(uint32_t crc, const B &buffer)
{
   return calc_crc32(crc, buf::cdata(buffer), buf::size(buffer)) ;
}

template<typename B>
inline typename enable_if_buffer<B, uint32_t>::type
calc_crc32(uint32_t crc, size_t ignore_tail, const B &buffer)
{
   const size_t bufsize = buf::size(buffer) ;
   return
      ignore_tail >= bufsize ? crc : calc_crc32(crc, buf::cdata(buffer), bufsize - ignore_tail) ;
}

} // end of namespace pcomn
#endif // __cplusplus && __PCOMN_BUFFER_H
