/*-*- tab-width:4;indent-tabs-mode:nil;c-file-style:"stroustrup";c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +)) -*-*/
/*******************************************************************************
 FILE         :   unittest_cmdline.cpp
 COPYRIGHT    :   Yakov Markovitch, 2015. All rights reserved.
                  See LICENSE for information on usage/redistribution.

 DESCRIPTION  :   pcomn_cmdline tests

 CREATION DATE:   7 Apr 2015
*******************************************************************************/
#include <pcomn_cmdline/cmdext.h>
#include <pcomn_unittest.h>

#include <stdlib.h>
#include <stdio.h>

using namespace pcomn ;

/*******************************************************************************
                            class CmdExtTests
*******************************************************************************/
class CmdExtTests : public CppUnit::TestFixture {

    void Test_Scalar_Args() ;
    void Test_List_Args() ;
    void Test_Pair_Args() ;

    CPPUNIT_TEST_SUITE(CmdExtTests) ;

    CPPUNIT_TEST(Test_Scalar_Args) ;
    CPPUNIT_TEST(Test_List_Args) ;
    CPPUNIT_TEST(Test_Pair_Args) ;

    CPPUNIT_TEST_SUITE_END() ;
} ;

void CmdExtTests::Test_Scalar_Args()
{
    using namespace cmdl ;

    Arg<int> aint ('i', "int", "INTEGER", "") ;
    CmdLine cmd01 ("cmd01") ;

    cmd01.append(aint).flags(CmdLine::NO_ABORT) ;
    CPPUNIT_LOG_EQ(cmd01.parse(CmdStrTokIter("--int=20")), 0) ;
    CPPUNIT_LOG_EQUAL(aint.value(), 20) ;
    // Check for autoreset
    CPPUNIT_LOG_EQ(cmd01.parse(CmdStrTokIter("--int=155")), 0) ;
    CPPUNIT_LOG_EXPRESSION(aint) ;
    CPPUNIT_LOG_EQUAL(aint.value(), 155) ;
    CPPUNIT_LOG_EQ(cmd01.parse(CmdStrTokIter("")), 0) ;
    CPPUNIT_LOG_EXPRESSION(aint) ;
    CPPUNIT_LOG_EQUAL(aint.value(), 0) ;

    CPPUNIT_LOG_EQ(cmd01.parse(CmdStrTokIter("--int=15a --int=25")), CmdLine::BAD_VALUE) ;
    CPPUNIT_LOG_EQ(aint.value(), 0) ;
}

void CmdExtTests::Test_List_Args()
{
    using namespace cmdl ;
    typedef std::vector<int> int_vector ;

    Arg<int> aint ('i', "int", "INTEGER", "") ;
    Arg<std::vector<std::string> > astrvec ("STRING", "") ;
    Arg<int_vector> aintvec ("INT", "") ;

    CmdLine cmd01 ("cmd01") ;

    cmd01.append(astrvec).flags(CmdLine::NO_ABORT) ;
    CPPUNIT_LOG_EQ(cmd01.parse(CmdStrTokIter("ab cd ef")), 0) ;
    CPPUNIT_LOG_EXPRESSION(astrvec) ;
    CPPUNIT_LOG_EQ(astrvec.value(), (string_vector{"ab", "cd", "ef"})) ;

    CmdLine cmd02 ("cmd02") ;
    cmd02.append(aintvec).flags(CmdLine::NO_ABORT) ;
    CPPUNIT_LOG_ASSERT(aintvec.syntax() & CmdArg::isREQ) ;
    CPPUNIT_LOG_EQ(cmd02.parse(CmdStrTokIter("")), CmdLine::ARG_MISSING) ;
    CPPUNIT_LOG_EQ(cmd02.parse(CmdStrTokIter("ab cd ef")), CmdLine::ARG_MISSING | CmdLine::BAD_VALUE) ;
    CPPUNIT_LOG_EXPRESSION(aintvec) ;
    CPPUNIT_LOG_EQ(cmd02.parse(CmdStrTokIter("3  4 9")), 0) ;
    CPPUNIT_LOG_EQ(aintvec.value(), (int_vector{3, 4, 9})) ;
    CPPUNIT_LOG_EQ(cmd02.parse(CmdStrTokIter("3  4a 9")), CmdLine::BAD_VALUE) ;

    CPPUNIT_LOG(std::endl) ;
    Arg<int_vector> optintvec {'\0', "oi", "INT", ""} ;
    CmdLine cmd03 ("cmd03") ;
    cmd03.append(optintvec).flags(CmdLine::NO_ABORT) ;
    CPPUNIT_LOG_EQ(cmd03.parse(CmdStrTokIter("--oi=3")), 0) ;
    CPPUNIT_LOG_EQ(optintvec.value(), (int_vector{3})) ;
    CPPUNIT_LOG_EQ(cmd03.parse(CmdStrTokIter("--oi=4 --oi=3")), 0) ;
    CPPUNIT_LOG_EQ(optintvec.value(), (int_vector{4, 3})) ;

    CPPUNIT_LOG_EQ(cmd03.parse(CmdStrTokIter("--oi=4,3")), CmdLine::BAD_VALUE) ;

    Arg<int_vector> commavec {'\0', "ci", "INT", "", separator(',')} ;
    cmd03.append(commavec) ;
    CPPUNIT_LOG_EQ(cmd03.parse(CmdStrTokIter("--ci=8,5 --ci=9")), 0) ;
    CPPUNIT_LOG_EQ(commavec.value(), (int_vector{8, 5, 9})) ;

    Arg<int_vector> arrowvec {'\0', "ca", "INT", "", separator("->")} ;
    cmd03.append(arrowvec) ;
    CPPUNIT_LOG_EQ(cmd03.parse(CmdStrTokIter("--ca=1,15,2")), CmdLine::BAD_VALUE) ;
    CPPUNIT_LOG_EQ(cmd03.parse(CmdStrTokIter("--ca=1")), 0) ;
    CPPUNIT_LOG_EQ(arrowvec.value(), (int_vector{1})) ;
    CPPUNIT_LOG_EQ(cmd03.parse(CmdStrTokIter("--ca=1->15->2")), 0) ;
    CPPUNIT_LOG_EQ(arrowvec.value(), (int_vector{1, 15, 2})) ;
}

void CmdExtTests::Test_Pair_Args()
{
    using namespace cmdl ;
    typedef unipair<int> int_pair ;
    typedef unipair<std::string> string_pair ;
    typedef std::vector<interval<int> > intpair_vector ;

    Arg<interval<int> > intopt ('i', "int", "INTEGER", "") ;
    Arg<interval<int> > intarg ("INT", "", CmdArg::isVALOPT) ;

    Arg<interval<std::string> > stropt ('s', "str", "STRING", "", separator(':')) ;

    CmdLine cmd01 ("cmd01") ;

    cmd01
        .append(intopt)
        .append(intarg)
        .append(stropt)
        .flags(CmdLine::NO_ABORT) ;

    CPPUNIT_LOG_EQ(cmd01.parse(CmdStrTokIter("1..2")), 0) ;
    CPPUNIT_LOG_EXPRESSION(intarg) ;
    CPPUNIT_LOG_EQUAL(intarg.value(), int_pair(1, 2)) ;

    CPPUNIT_LOG_EQ(cmd01.parse(CmdStrTokIter("3")), 0) ;
    CPPUNIT_LOG_EXPRESSION(intarg) ;
    CPPUNIT_LOG_EQUAL(intarg.value(), int_pair(3, 3)) ;

    Arg<intpair_vector> optintvec {'\0', "oi", "INT", "", separator(',')} ;
    cmd01.append(optintvec) ;
    CPPUNIT_LOG_EQ(cmd01.parse(CmdStrTokIter("--oi=1..2,10")), 0) ;
    CPPUNIT_LOG_EXPRESSION(optintvec) ;
    CPPUNIT_LOG_EXPRESSION(PCOMN_TYPENAME(optintvec.value())) ;
    CPPUNIT_LOG_EQUAL(optintvec.value(), (std::vector<unipair<int> >{{1, 2}, {10, 10}})) ;
}

int main(int argc, char *argv[])
{
    pcomn::unit::TestRunner runner ;
    runner.addTest(CmdExtTests::suite()) ;

    return
        pcomn::unit::run_tests(runner, argc, argv, "unittest.trace.ini",
                               "Testing pcommon cmdline library.") ;
}
