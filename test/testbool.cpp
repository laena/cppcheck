/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjam�ki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "tokenize.h"
#include "checkbool.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestBool : public TestFixture {
public:
    TestBool() : TestFixture("TestBool")
    { }

private:


    void run() {
        TEST_CASE(bitwiseOnBoolean);      // if (bool & bool)
        TEST_CASE(incrementBoolean);
        TEST_CASE(assignBoolToPointer);

        TEST_CASE(comparisonOfBoolExpressionWithInt1);
        TEST_CASE(comparisonOfBoolExpressionWithInt2);
        TEST_CASE(comparisonOfBoolExpressionWithInt3);

        TEST_CASE(comparisonOfBoolWithInt1);
        TEST_CASE(comparisonOfBoolWithInt2);
        TEST_CASE(comparisonOfBoolWithInt3);
        TEST_CASE(comparisonOfBoolWithInt4);
        TEST_CASE(comparisonOfBoolWithInt5);
        TEST_CASE(comparisonOfBoolWithInt6); // #4224 - integer is casted to bool

        TEST_CASE(checkComparisonOfFuncReturningBool1);
        TEST_CASE(checkComparisonOfFuncReturningBool2);
        TEST_CASE(checkComparisonOfFuncReturningBool3);
        TEST_CASE(checkComparisonOfFuncReturningBool4);
        TEST_CASE(checkComparisonOfFuncReturningBool5);
        TEST_CASE(checkComparisonOfFuncReturningBool6);
    }

    void check(const char code[], bool experimental = false) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.addEnabled("warning");
        settings.inconclusive = true;
        settings.experimental = experimental;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check...
        CheckBool checkBool(&tokenizer, &settings, this);
        checkBool.runChecks(&tokenizer, &settings, this);

        // Simplify token list..
        const std::string str1(tokenizer.tokens()->stringifyList(0,true));
        tokenizer.simplifyTokenList();
        const std::string str2(tokenizer.tokens()->stringifyList(0,true));
        if (str1 != str2)
            warn(("Unsimplified code in test case. It looks like this test "
                  "should either be cleaned up or moved to TestTokenizer or "
                  "TestSimplifyTokens instead.\nstr1="+str1+"\nstr2="+str2).c_str());


        checkBool.runSimplifiedChecks(&tokenizer, &settings, this);
    }


    void assignBoolToPointer() {
        check("void foo(bool *p) {\n"
              "    p = false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Boolean value assigned to pointer.\n", errout.str());
    }

    void comparisonOfBoolExpressionWithInt1() {
        check("void f(int x) {\n"
              "    if ((x && 0x0f)==6)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x && 0x0f)==0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x || 0x0f)==6)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((x || 0x0f)==0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x & 0x0f)==6)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((x | 0x0f)==6)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());


        check("void f(int x) {\n"
              "    if ((5 && x)==3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((5 && x)==3 || (8 && x)==9)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((5 && x)!=3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());


        check("void f(int x) {\n"
              "    if ((5 && x) > 3)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((5 && x) > 0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((5 && x) < 0)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if ((5 && x) < 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if ((5 && x) > 1)\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());


        check("void f(int x) {\n"
              "    if (0 < (5 && x))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (0 > (5 && x))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (1 > (5 && x))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (1 < (5 && x))\n"
              "        a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(bool x ) {\n"
              "  if ( x > false )\n"
              "      a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=).\n", errout.str());

        check("void f(bool x ) {\n"
              "  if ( false < x )\n"
              "      a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=).\n", errout.str());

        check("void f(bool x ) {\n"
              "  if ( x < false )\n"
              "      a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=).\n", errout.str());

        check("void f(bool x ) {\n"
              "  if ( false > x )\n"
              "      a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=).\n", errout.str());

        check("void f(bool x ) {\n"
              "  if ( x >= false )\n"
              "      a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=).\n", errout.str());

        check("void f(bool x ) {\n"
              "  if ( false >= x )\n"
              "      a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=).\n", errout.str());

        check("void f(bool x ) {\n"
              "  if ( x <= false )\n"
              "      a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=).\n", errout.str());

        check("void f(bool x ) {\n"
              "  if ( false <= x )\n"
              "      a++;\n"
              "}\n"
             );
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean value using relational operator (<, >, <= or >=).\n", errout.str());

        check("typedef int (*func)(bool invert);\n"
              "void x(int, func f);\n"
              "void foo(int error) {\n"
              "  if (error == ABC) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolExpressionWithInt2() {
        check("void f(int x) {\n"
              "    if (!x == 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (!x != 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (x != 10) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (10 == !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x) {\n"
              "    if (10 != !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer other than 0 or 1.\n", errout.str());

        check("void f(int x, int y) {\n"
              "    if (y != !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean expression with an integer.\n", errout.str());

        check("void f(int x, bool y) {\n"
              "    if (y != !x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int x) {\n"
              "    if (10 != x) {\n"
              "        printf(\"x not equal to 10\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolExpressionWithInt3() {
        check("int f(int x) {\n"
              "    return t<0>() && x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkComparisonOfFuncReturningBool1() {
        check("void f(){\n"
              "     int temp = 4;\n"
              "     if(compare1(temp) > compare2(temp)){\n"
              "         printf(\"foo\");\n"
              "     }\n"
              "}\n"
              "bool compare1(int temp){\n"
              "     if(temp==4){\n"
              "         return true;\n"
              "     }\n"
              "     else\n"
              "         return false;\n"
              "}\n"
              "bool compare2(int temp){\n"
              "     if(temp==4){\n"
              "         return false;\n"
              "     }\n"
              "     else\n"
              "         return true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator.\n", errout.str());
    }

    void checkComparisonOfFuncReturningBool2() {
        check("void f(){\n"
              " int temp = 4;\n"
              " bool a = true;\n"
              " if(compare(temp) > a){\n"
              "     printf(\"foo\");\n"
              " }\n"
              "}\n"
              "bool compare(int temp){\n"
              "  if(temp==4){\n"
              "     return true;\n"
              "  }\n"
              "    else\n"
              "     return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator.\n", errout.str());
    }

    void checkComparisonOfFuncReturningBool3() {
        check("void f(){\n"
              " int temp = 4;\n"
              " if(compare(temp) > temp){\n"
              "         printf(\"foo\");\n"
              "   }\n"
              "}\n"
              "bool compare(int temp){\n"
              "   if(temp==4){\n"
              "     return true;\n"
              "   }\n"
              " else\n"
              "     return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator.\n", errout.str());
    }

    void checkComparisonOfFuncReturningBool4() {
        check("void f(){\n"
              "   int temp = 4;\n"
              " bool b = compare2(6);\n"
              " if(compare1(temp)> b){\n"
              "         printf(\"foo\");\n"
              " }\n"
              "}\n"
              "bool compare1(int temp){\n"
              " if(temp==4){\n"
              "     return true;\n"
              "     }\n"
              " else\n"
              "     return false;\n"
              "}\n"
              "bool compare2(int temp){\n"
              " if(temp == 5){\n"
              "     return true;\n"
              " }\n"
              " else\n"
              "     return false;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator.\n", errout.str());
    }

    void checkComparisonOfFuncReturningBool5() {
        check("void f(){\n"
              "     int temp = 4;\n"
              "     if(compare1(temp) > !compare2(temp)){\n"
              "         printf(\"foo\");\n"
              "     }\n"
              "}\n"
              "bool compare1(int temp){\n"
              "     if(temp==4){\n"
              "         return true;\n"
              "     }\n"
              "     else\n"
              "         return false;\n"
              "}\n"
              "bool compare2(int temp){\n"
              "     if(temp==4){\n"
              "         return false;\n"
              "     }\n"
              "     else\n"
              "         return true;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Comparison of two functions returning boolean value using relational (<, >, <= or >=) operator.\n", errout.str());
    }

    void checkComparisonOfFuncReturningBool6() {
        check("int compare1(int temp);\n"
              "namespace Foo {\n"
              "    bool compare1(int temp);\n"
              "}\n"
              "void f(){\n"
              "    int temp = 4;\n"
              "    if(compare1(temp) > compare2(temp)){\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("namespace Foo {\n"
              "    bool compare1(int temp);\n"
              "}\n"
              "int compare1(int temp);\n"
              "void f(){\n"
              "    int temp = 4;\n"
              "    if(compare1(temp) > compare2(temp)){\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int compare1(int temp);\n"
              "namespace Foo {\n"
              "    bool compare1(int temp);\n"
              "    void f(){\n"
              "        int temp = 4;\n"
              "        if(compare1(temp) > compare2(temp)){\n"
              "            printf(\"foo\");\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Comparison of a function returning boolean value using relational (<, >, <= or >=) operator.\n", errout.str());

        check("int compare1(int temp);\n"
              "namespace Foo {\n"
              "    bool compare1(int temp);\n"
              "    void f(){\n"
              "        int temp = 4;\n"
              "        if(::compare1(temp) > compare2(temp)){\n"
              "            printf(\"foo\");\n"
              "        }\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool compare1(int temp);\n"
              "void f(){\n"
              "    int temp = 4;\n"
              "    if(foo.compare1(temp) > compare2(temp)){\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkComparisonOfBoolWithBool() {
        const char code[] = "void f(){\n"
                            "    int temp = 4;\n"
                            "    bool b = compare2(6);\n"
                            "    bool a = compare1(4);\n"
                            "    if(b > a){\n"
                            "        printf(\"foo\");\n"
                            "    }\n"
                            "}\n"
                            "bool compare1(int temp){\n"
                            "    if(temp==4){\n"
                            "        return true;\n"
                            "    }\n"
                            "    else\n"
                            "        return false;\n"
                            "}\n"
                            "bool compare2(int temp){\n"
                            "    if(temp == 5){\n"
                            "        return true;\n"
                            "    }\n"
                            "    else\n"
                            "        return false;\n"
                            "}\n";
        check(code, true);
        ASSERT_EQUALS("[test.cpp:5]: (style) Comparison of a variable having boolean value using relational (<, >, <= or >=) operator.\n", errout.str());
        check(code, false);
        ASSERT_EQUALS("", errout.str());
    }

    void bitwiseOnBoolean() { // 3062
        check("void f(_Bool a, _Bool b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '&&'?\n", errout.str());

        check("void f(_Bool a, _Bool b) {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '||'?\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "    if(a & !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '&&'?\n", errout.str());

        check("void f(bool a, bool b) {\n"
              "    if(a | !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '||'?\n", errout.str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '&&'?\n", errout.str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a & !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '&&'?\n", errout.str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a | b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '||'?\n", errout.str());

        check("bool a, b;\n"
              "void f() {\n"
              "    if(a | !b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '||'?\n", errout.str());

        check("void f(bool a, int b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Boolean variable 'a' is used in bitwise operation. Did you mean '&&'?\n", errout.str());

        check("void f(int a, bool b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style, inconclusive) Boolean variable 'b' is used in bitwise operation. Did you mean '&&'?\n", errout.str());

        check("void f(int a, int b) {\n"
              "    if(a & b) {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool b) {\n"
              "    foo(bar, &b);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void incrementBoolean() {
        check("bool bValue = true;\n"
              "void f() { bValue++; }");
        ASSERT_EQUALS("[test.cpp:2]: (style) Incrementing a variable of type 'bool' with postfix operator++ is deprecated by the C++ Standard. You should assign it the value 'true' instead.\n", errout.str());

        check("void f(bool test){\n"
              "    test++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Incrementing a variable of type 'bool' with postfix operator++ is deprecated by the C++ Standard. You should assign it the value 'true' instead.\n", errout.str());

        check("void f(int test){\n"
              "    test++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt1() {
        check("void f(bool x) {\n"
              "    if (x < 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer.\n", errout.str());

        check("void f(bool x) {\n"
              "    if (10 >= x) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer.\n", errout.str());

        check("void f(bool x) {\n"
              "    if (x != 0) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool x) {\n"  // #3356
              "    if (x == 1) {\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool x) {\n"
              "    if (x != 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer that is neither 1 nor 0.\n", errout.str());

        check("void f(bool x) {\n"
              "    if (x == 10) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer that is neither 1 nor 0.\n", errout.str());

        check("void f(bool x) {\n"
              "    if (x == 0) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("DensePropertyMap<int, true> visited;"); // #4075
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt2() {
        check("void f(bool x, int y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer.\n", errout.str());

        check("void f(int x, bool y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer.\n", errout.str());

        check("void f(bool x, bool y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool x, fooClass y) {\n"
              "    if (x == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt3() {
        check("void f(int y) {\n"
              "    if (y > false) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer.\n", errout.str());

        check("void f(int y) {\n"
              "    if (true == y) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer.\n", errout.str());

        check("void f(bool y) {\n"
              "    if (y == true) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(bool y) {\n"
              "    if (false < 5) {\n"
              "        printf(\"foo\");\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Comparison of a boolean with an integer.\n", errout.str());
    }

    void comparisonOfBoolWithInt4() {
        check("void f(int x) {\n"
              "    if (!x == 1) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt5() {
        check("void SetVisible(int index, bool visible) {\n"
              "    bool (SciTEBase::*ischarforsel)(char ch);\n"
              "    if (visible != GetVisible(index)) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void comparisonOfBoolWithInt6() { // #4224 - integer is casted to bool
        check("void SetVisible(bool b, int i) {\n"
              "    if (b == (bool)i) { }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestBool)
