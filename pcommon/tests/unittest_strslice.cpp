/*-*- tab-width:3; indent-tabs-mode:nil; c-file-style:"ellemtel"; c-file-offsets:((innamespace . 0)(inclass . ++)) -*-*/
/*******************************************************************************
 FILE         :   unittest_strslice.cpp
 COPYRIGHT    :   Yakov Markovitch, 2011-2014. All rights reserved.
                  See LICENSE for information on usage/redistribution.

 DESCRIPTION  :   Unittest for basic_strslice<>

 PROGRAMMED BY:   Yakov Markovitch
 CREATION DATE:   27 Jan 2011
*******************************************************************************/
#include <pcomn_strslice.h>
#include <pcomn_unittest.h>
#include <pcomn_meta.h>

#include <memory>

/*******************************************************************************
                     class StrSliceTests
*******************************************************************************/
class StrSliceTests : public CppUnit::TestFixture {

      void Test_Strslice_Construct() ;
      void Test_Strslice_Compare() ;
      void Test_Strslice_Compare_CaseInsensitive() ;
      void Test_String_Split() ;
      void Test_Strslice_Strip() ;
      void Test_Strslice_Strnew() ;

      CPPUNIT_TEST_SUITE(StrSliceTests) ;

      CPPUNIT_TEST(Test_Strslice_Construct) ;
      CPPUNIT_TEST(Test_Strslice_Compare) ;
      CPPUNIT_TEST(Test_Strslice_Compare_CaseInsensitive) ;
      CPPUNIT_TEST(Test_String_Split) ;
      CPPUNIT_TEST(Test_Strslice_Strip) ;
      CPPUNIT_TEST(Test_Strslice_Strnew) ;

      CPPUNIT_TEST_SUITE_END() ;
} ;


/*******************************************************************************
 StrSliceTests
*******************************************************************************/
void StrSliceTests::Test_Strslice_Construct()
{
   using namespace pcomn ;

   CPPUNIT_LOG_ASSERT(strslice().empty()) ;
   CPPUNIT_LOG_IS_FALSE(strslice()) ;
   CPPUNIT_LOG_ASSERT(strslice().is_null()) ;
   CPPUNIT_LOG_ASSERT(strslice("").empty()) ;
   CPPUNIT_LOG_IS_FALSE(strslice("")) ;
   CPPUNIT_LOG_IS_FALSE(strslice("").is_null()) ;

   CPPUNIT_LOG_IS_TRUE((std::has_trivial_copy_assign<strslice>::value)) ;
   CPPUNIT_LOG_IS_TRUE((std::is_trivially_copyable<strslice>::value)) ;
}

void StrSliceTests::Test_Strslice_Compare()
{
   using namespace pcomn ;
   CPPUNIT_LOG_ASSERT(strslice("abc") == "abc") ;
   CPPUNIT_LOG_IS_FALSE(strslice("abc") == "bc") ;
   CPPUNIT_LOG_ASSERT(strslice("abc") != "bc") ;

   CPPUNIT_LOG_ASSERT("abc" == strslice("abc")) ;
   CPPUNIT_LOG_IS_FALSE("bc" == strslice("abc")) ;
   CPPUNIT_LOG_ASSERT("bc" != strslice("abc")) ;

   CPPUNIT_LOG(std::endl) ;
   CPPUNIT_LOG_ASSERT(strslice("abc") == std::string("abc")) ;
   CPPUNIT_LOG_IS_FALSE(strslice("abc") == std::string("bc")) ;
   CPPUNIT_LOG_ASSERT(strslice("abc") != std::string("bc")) ;

   CPPUNIT_LOG_ASSERT(std::string("abc") == strslice("abc")) ;
   CPPUNIT_LOG_IS_FALSE(std::string("bc") == strslice("abc")) ;
   CPPUNIT_LOG_ASSERT(std::string("bc") != strslice("abc")) ;

   CPPUNIT_LOG(std::endl) ;
   CPPUNIT_LOG_ASSERT(strslice("abc") < "abcd") ;
   CPPUNIT_LOG_ASSERT("abc" < strslice("abcd")) ;
   CPPUNIT_LOG_ASSERT(strslice("abcd") > "abc") ;
   CPPUNIT_LOG_ASSERT("abcd" > strslice("abc")) ;

   CPPUNIT_LOG_IS_FALSE(strslice("bcd") < "abcd") ;
   CPPUNIT_LOG_ASSERT(strslice("bcd") > "abcd") ;
   CPPUNIT_LOG_ASSERT(strslice("bcd") >= "abcd") ;
   CPPUNIT_LOG_IS_FALSE(strslice("bcd") <= "abcd") ;
   CPPUNIT_LOG_IS_FALSE("bcd" <= strslice("abcd")) ;
   CPPUNIT_LOG_ASSERT(strslice("bcd") <= "bcd") ;
   CPPUNIT_LOG_ASSERT("bcd" <= strslice("bcd")) ;

   // Ensure the comparison is unsigned
   CPPUNIT_LOG(std::endl) ;
   CPPUNIT_LOG_ASSERT("abc" < strslice("\x85\x86\x87")) ;
   CPPUNIT_LOG_ASSERT("abc" < strslice("\x85")) ;
   CPPUNIT_LOG_ASSERT(strslice("abc").compare("\x85\x86\x87") < 0) ;
   CPPUNIT_LOG_ASSERT(strslice("abc").compare("abc") == 0) ;
   CPPUNIT_LOG_ASSERT(strslice("abcd").compare("abc") > 0) ;
   CPPUNIT_LOG_ASSERT(strslice("b").compare("abc") > 0) ;
}

void StrSliceTests::Test_Strslice_Compare_CaseInsensitive()
{
   using namespace pcomn ;

   const strslice abc ("abc") ;
   const strslice ABC ("ABC") ;
   const strslice Abc ("Abc") ;

   CPPUNIT_LOG_ASSERT(eqi(strslice("abc"), strslice("abc"))) ;
   CPPUNIT_LOG_ASSERT(eqi(abc, abc)) ;
   CPPUNIT_LOG_IS_FALSE(eqi(strslice("abc"), strslice("bc"))) ;
   CPPUNIT_LOG_ASSERT(eqi(abc, ABC)) ;
   CPPUNIT_LOG_ASSERT(eqi(ABC, ABC)) ;
   CPPUNIT_LOG_ASSERT(eqi(ABC, abc)) ;
   CPPUNIT_LOG_ASSERT(eqi(ABC, Abc)) ;
   CPPUNIT_LOG_ASSERT(eqi(strslice(), strslice())) ;
   CPPUNIT_LOG_ASSERT(eqi("bcd", "BCD")) ;
   CPPUNIT_LOG_IS_FALSE(eqi("cdb", "BCD")) ;

   CPPUNIT_LOG(std::endl) ;
   CPPUNIT_LOG_IS_FALSE(lti(strslice(), strslice())) ;
   CPPUNIT_LOG_IS_FALSE(lti(strslice("abc"), strslice("abc"))) ;
   CPPUNIT_LOG_IS_FALSE(lti(strslice(abc), strslice("abc"))) ;
   CPPUNIT_LOG_IS_FALSE(lti(strslice(abc), strslice(abc))) ;
   CPPUNIT_LOG_IS_FALSE(lti(strslice(Abc), strslice(abc))) ;
   CPPUNIT_LOG_IS_FALSE(lti(strslice(abc), strslice(Abc))) ;

   CPPUNIT_LOG_ASSERT(lti(abc, "b")) ;
   CPPUNIT_LOG_ASSERT(lti(abc, "B")) ;
   CPPUNIT_LOG_ASSERT(lti("bcd", "CD")) ;
   CPPUNIT_LOG_ASSERT(lti("BCD", "CD")) ;
   CPPUNIT_LOG_ASSERT(lti("BCD", "cd")) ;
}

void StrSliceTests::Test_String_Split()
{
   using namespace pcomn ;

   // split
   CPPUNIT_LOG_EQUAL(strsplit(strslice("abcdcfc"), 'c'), strslice_pair("ab", "dcfc")) ;
   CPPUNIT_LOG_EQUAL(strsplit(strslice("abcdcfc"), strslice("c")), strslice_pair("ab", "dcfc")) ;
   CPPUNIT_LOG_EQUAL(strsplit(strslice("abcdcfc"), strslice("")), strslice_pair("", "abcdcfc")) ;
   CPPUNIT_LOG_IS_NULL(strsplit(strslice("abcdcfc"), strslice("")).first.begin()) ;

   CPPUNIT_LOG_EQUAL(strsplit(strslice("abcdcfc"), 'A'), strslice_pair("abcdcfc", "")) ;
   CPPUNIT_LOG_IS_NULL(strsplit(strslice("abcdcfc"), 'A').second.begin()) ;
   CPPUNIT_LOG_EQUAL(strsplit(strslice("abcdcfcA"), 'A'), strslice_pair("abcdcfc", "")) ;
   CPPUNIT_LOG_ASSERT(strsplit(strslice("abcdcfcA"), 'A').second.begin()) ;

   // rsplit
   CPPUNIT_LOG(std::endl) ;
   CPPUNIT_LOG_EQUAL(strrsplit(strslice("abcdcf"), 'c'), strslice_pair("abcd", "f")) ;
   CPPUNIT_LOG_EQUAL(strrsplit(strslice("abcdcf"), strslice("c")), strslice_pair("abcd", "f")) ;
   CPPUNIT_LOG_EQUAL(strrsplit(strslice("abcdcf"), strslice("")), strslice_pair("abcdcf", "")) ;
   CPPUNIT_LOG_IS_NULL(strrsplit(strslice("abcdcf"), strslice("")).second.begin()) ;

   CPPUNIT_LOG_EQUAL(strrsplit(strslice("abcdcf"), 'A'), strslice_pair("", "abcdcf")) ;
   CPPUNIT_LOG_IS_NULL(strrsplit(strslice("abcdcf"), 'A').first.begin()) ;
   CPPUNIT_LOG_EQUAL(strrsplit(strslice("Aabcdcf"), 'A'), strslice_pair("", "abcdcf")) ;
   CPPUNIT_LOG_ASSERT(strrsplit(strslice("Aabcdcf"), 'A').first.begin()) ;

   // string arguments instead of strslice
   CPPUNIT_LOG(std::endl) ;
   CPPUNIT_LOG_EQUAL(strsplit("abcdcfc", 'c'), strslice_pair("ab", "dcfc")) ;
   CPPUNIT_LOG_EQUAL(strsplit("abcdcfc", "c"), strslice_pair("ab", "dcfc")) ;
   CPPUNIT_LOG_EQUAL(strsplit("abcdcfc", strslice("c")), strslice_pair("ab", "dcfc")) ;
   CPPUNIT_LOG_EQUAL(strsplit(strslice("abcdcfc"), "c"), strslice_pair("ab", "dcfc")) ;
   CPPUNIT_LOG_EQUAL(strsplit("abcdcfc", std::string("c")), strslice_pair("ab", "dcfc")) ;
   CPPUNIT_LOG_EQUAL(strsplit(std::string("abcdcfc"), "c"), strslice_pair("ab", "dcfc")) ;
   CPPUNIT_LOG_EQUAL(strsplit(std::string("abcdcfc"), 'c'), strslice_pair("ab", "dcfc")) ;

   CPPUNIT_LOG_EQUAL(strrsplit("abcdcf", 'c'), strslice_pair("abcd", "f")) ;
   CPPUNIT_LOG_EQUAL(strrsplit("abcdcf", "c"), strslice_pair("abcd", "f")) ;
   CPPUNIT_LOG_EQUAL(strrsplit("abcdcf", strslice("c")), strslice_pair("abcd", "f")) ;
   CPPUNIT_LOG_EQUAL(strrsplit(strslice("abcdcf"), "c"), strslice_pair("abcd", "f")) ;
   CPPUNIT_LOG_EQUAL(strrsplit("abcdcf", std::string("c")), strslice_pair("abcd", "f")) ;
   CPPUNIT_LOG_EQUAL(strrsplit(std::string("abcdcf"), "c"), strslice_pair("abcd", "f")) ;
   CPPUNIT_LOG_EQUAL(strrsplit(std::string("abcdcf"), 'c'), strslice_pair("abcd", "f")) ;
}

void StrSliceTests::Test_Strslice_Strip()
{
}

void StrSliceTests::Test_Strslice_Strnew()
{
   char sbuf1[] = "Hello" ;
   const char sbuf2[] = "world!" ;
   std::unique_ptr<char[]> str ;

   CPPUNIT_LOG_RUN(str.reset(pcomn::str::strnew(sbuf1))) ;
   CPPUNIT_LOG_EQUAL(strlen(str.get()), strlen(sbuf1)) ;
   CPPUNIT_LOG_RUN(str.reset(pcomn::str::strnew(pcomn::strslice(sbuf1)))) ;
   CPPUNIT_LOG_EQUAL(strlen(str.get()), strlen(sbuf1)) ;
   CPPUNIT_LOG_RUN(str.reset(pcomn::str::strnew(sbuf1 + 0))) ;
   CPPUNIT_LOG_EQUAL(strlen(str.get()), strlen(sbuf1)) ;

   CPPUNIT_LOG_RUN(str.reset(pcomn::str::strnew(pcomn::strslice()))) ;
   CPPUNIT_LOG_EQUAL(strlen(str.get()), size_t()) ;

   CPPUNIT_LOG_RUN(str.reset(pcomn::str::strnew(sbuf2))) ;
   CPPUNIT_LOG_EQUAL(strlen(str.get()), strlen(sbuf2)) ;
}

int main(int argc, char *argv[])
{
   pcomn::unit::TestRunner runner ;
   runner.addTest(StrSliceTests::suite()) ;

   return
      pcomn::unit::run_tests(runner, argc, argv, "unittest.diag.ini",
                             "basic_strslice<> tests") ;
}
