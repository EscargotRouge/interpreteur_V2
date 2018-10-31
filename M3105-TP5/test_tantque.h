/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   test_tantque.h
 * Author: blanquan
 *
 * Created on Oct 31, 2018, 2:40:48 PM
 */

#ifndef TEST_TANTQUE_H
#define TEST_TANTQUE_H

#include <cppunit/extensions/HelperMacros.h>

class test_tantque : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(test_tantque);

    CPPUNIT_TEST(testMethod);
    CPPUNIT_TEST(testFailedMethod);

    CPPUNIT_TEST_SUITE_END();

public:
    test_tantque();
    virtual ~test_tantque();
    void setUp();
    void tearDown();

private:
    void testMethod();
    void testFailedMethod();
};

#endif /* TEST_TANTQUE_H */

