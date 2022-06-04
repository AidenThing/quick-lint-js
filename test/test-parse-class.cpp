// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <quick-lint-js/array.h>
#include <quick-lint-js/char8.h>
#include <quick-lint-js/cli-location.h>
#include <quick-lint-js/diag-collector.h>
#include <quick-lint-js/diag-matcher.h>
#include <quick-lint-js/diagnostic-types.h>
#include <quick-lint-js/language.h>
#include <quick-lint-js/padded-string.h>
#include <quick-lint-js/parse-support.h>
#include <quick-lint-js/parse.h>
#include <quick-lint-js/spy-visitor.h>
#include <string>
#include <string_view>
#include <vector>

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::UnorderedElementsAre;

namespace quick_lint_js {
namespace {
TEST(test_parse, super_in_class) {
  {
    spy_visitor v = parse_and_visit_statement(
        u8"class C extends Base { constructor() { super(); } }");
    EXPECT_THAT(v.errors, IsEmpty());
  }
}

TEST(test_parse, parse_class_statement) {
  {
    spy_visitor v = parse_and_visit_statement(u8"class C {}"_sv);

    ASSERT_EQ(v.variable_declarations.size(), 1);
    EXPECT_EQ(v.variable_declarations[0].name, u8"C");
    EXPECT_EQ(v.variable_declarations[0].kind, variable_kind::_class);

    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  //
                                      "visit_enter_class_scope",     //
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class Derived extends Base {}"_sv);

    ASSERT_EQ(v.variable_declarations.size(), 1);
    EXPECT_EQ(v.variable_declarations[0].name, u8"Derived");
    EXPECT_EQ(v.variable_declarations[0].kind, variable_kind::_class);

    ASSERT_EQ(v.variable_uses.size(), 1);
    EXPECT_EQ(v.variable_uses[0].name, u8"Base");

    EXPECT_THAT(v.visits, ElementsAre("visit_variable_use",          //
                                      "visit_variable_declaration",  //
                                      "visit_enter_class_scope",     //
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v = parse_and_visit_statement(
        u8"class FileStream extends fs.ReadStream {}");
    ASSERT_EQ(v.variable_uses.size(), 1);
    EXPECT_EQ(v.variable_uses[0].name, u8"fs");
  }

  {
    spy_visitor v;
    padded_string code(u8"class A {} class B {}"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.variable_declarations,
        ElementsAre(
            spy_visitor::visited_variable_declaration{
                u8"A", variable_kind::_class, variable_init_kind::normal},
            spy_visitor::visited_variable_declaration{
                u8"B", variable_kind::_class, variable_init_kind::normal}));
  }
}

TEST(test_parse, class_statement_requires_a_name) {
  {
    spy_visitor v;
    padded_string code(u8"class {}"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",  //
                                      "visit_exit_class_scope"));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_missing_name_in_class_statement,  //
                              class_keyword, 0, u8"class")));
  }
}

TEST(test_parse, class_statement_requires_a_body) {
  {
    spy_visitor v;
    padded_string code(u8"class C "_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration"));  // C
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_missing_body_for_class,  //
                              class_keyword_and_name_and_heritage,
                              strlen(u8"class C"), u8"")));
  }

  {
    spy_visitor v;
    padded_string code(u8"class ;"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, IsEmpty());
    EXPECT_THAT(
        v.errors,
        UnorderedElementsAre(
            DIAG_TYPE_OFFSETS(&code, diag_missing_name_in_class_statement,  //
                              class_keyword, 0, u8"class"),
            DIAG_TYPE_OFFSETS(&code, diag_missing_body_for_class,  //
                              class_keyword_and_name_and_heritage,
                              strlen(u8"class"), u8"")));
  }
}

TEST(test_parse, unclosed_class_statement) {
  {
    spy_visitor v;
    padded_string code(u8"class C { "_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     //
                                      "visit_exit_class_scope"));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_unclosed_class_block,  //
                              block_open, strlen(u8"class C "), u8"{")));
  }

  {
    spy_visitor v;
    padded_string code(u8"class C { method() {} "_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_enter_function_scope_body",  // method
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));         // C
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_unclosed_class_block,  //
                              block_open, strlen(u8"class C "), u8"{")));
  }

  {
    spy_visitor v;
    padded_string code(u8"class C { property "_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  // C
                            "visit_enter_class_scope",     // C
                            "visit_property_declaration",  // property
                            "visit_exit_class_scope"));    // C
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_unclosed_class_block,  //
                              block_open, strlen(u8"class C "), u8"{")));
  }
}

TEST(test_parse, class_statement_with_odd_heritage) {
  {
    // TODO(strager): Should this report errors?
    spy_visitor v = parse_and_visit_statement(u8"class C extends 0 {}"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     //
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C extends null {}"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     //
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C extends (A, B) {}"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_use",          // A
                                      "visit_variable_use",          // B
                                      "visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     //
                                      "visit_exit_class_scope"));
  }
}

TEST(test_parse, class_statement_extending_class_expression) {
  {
    spy_visitor v = parse_and_visit_statement(
        u8"class C extends class B { x() {} } { y() {} }"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",          // B
                                      "visit_variable_declaration",       // B
                                      "visit_property_declaration",       // B.x
                                      "visit_enter_function_scope",       // B.x
                                      "visit_enter_function_scope_body",  // B.x
                                      "visit_exit_function_scope",        // B.x
                                      "visit_exit_class_scope",           // B
                                      "visit_variable_declaration",       // C
                                      "visit_enter_class_scope",          // C
                                      "visit_property_declaration",       // C.y
                                      "visit_enter_function_scope",       // C.y
                                      "visit_enter_function_scope_body",  // C.y
                                      "visit_exit_function_scope",        // C.y
                                      "visit_exit_class_scope"));         // C
  }
}

TEST(test_parse, class_statement_with_methods) {
  {
    spy_visitor v = parse_and_visit_statement(
        u8"class Monster { eatMuffins(muffinCount) { } }");

    ASSERT_EQ(v.variable_declarations.size(), 2);
    EXPECT_EQ(v.variable_declarations[0].name, u8"Monster");
    EXPECT_EQ(v.variable_declarations[1].name, u8"muffinCount");

    ASSERT_EQ(v.property_declarations.size(), 1);
    EXPECT_EQ(v.property_declarations[0].name, u8"eatMuffins");

    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // Monster
                            "visit_enter_class_scope",          //
                            "visit_property_declaration",       // eatMuffins
                            "visit_enter_function_scope",       //
                            "visit_variable_declaration",       // muffinCount
                            "visit_enter_function_scope_body",  //
                            "visit_exit_function_scope",        //
                            "visit_exit_class_scope"));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { static m() { } }"_sv);

    ASSERT_EQ(v.property_declarations.size(), 1);
    EXPECT_EQ(v.property_declarations[0].name, u8"m");

    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",       // C
                                      "visit_enter_class_scope",          //
                                      "visit_property_declaration",       // m
                                      "visit_enter_function_scope",       //
                                      "visit_enter_function_scope_body",  //
                                      "visit_exit_function_scope",        //
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { async m() { } }"_sv);
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"m"}));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { static async m() { } }"_sv);
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"m"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { *m() { } }"_sv);
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"m"}));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { get length() { } }"_sv);
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"length"}));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { set length(value) { } }"_sv);
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"length"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(
        u8"class C {\n"
        u8"  static get length() { }\n"
        u8"  static set length(l) { }\n"
        u8"}");
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"length"},
                    spy_visitor::visited_property_declaration{u8"length"}));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { a(){} b(){} c(){} }"_sv);
    ASSERT_EQ(v.property_declarations.size(), 3);
    EXPECT_EQ(v.property_declarations[0].name, u8"a");
    EXPECT_EQ(v.property_declarations[1].name, u8"b");
    EXPECT_EQ(v.property_declarations[2].name, u8"c");
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { \"stringKey\"() {} }");
    ASSERT_EQ(v.property_declarations.size(), 1);
    EXPECT_EQ(v.property_declarations[0].name, std::nullopt);
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { [x + y]() {} }"_sv);
    ASSERT_EQ(v.variable_uses.size(), 2);
    EXPECT_EQ(v.variable_uses[0].name, u8"x");
    EXPECT_EQ(v.variable_uses[1].name, u8"y");
    ASSERT_EQ(v.property_declarations.size(), 1);
    EXPECT_EQ(v.property_declarations[0].name, std::nullopt);
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { #m() { } }"_sv);
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"#m"}));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { async #m() { } }"_sv);
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"#m"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { *#m() { } }"_sv);
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"#m"}));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { async *#m() { } }"_sv);
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"#m"}));
  }
}

TEST(test_parse, class_statement_methods_with_arrow_operator) {
  {
    spy_visitor v;
    padded_string code(u8"class C { method() => {} }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          //
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       //
                            "visit_enter_function_scope_body",  //
                            "visit_exit_function_scope",        //
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_functions_or_methods_should_not_have_arrow_operator,  //
            arrow_operator, strlen(u8"class C { method() "), u8"=>")));
  }
}

TEST(test_parse, missing_class_method_name_fails) {
  {
    padded_string code(u8"class Monster { (muffinCount) { } }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // Monster
                            "visit_enter_class_scope",          //
                            "visit_property_declaration",       // (unnamed)
                            "visit_enter_function_scope",       //
                            "visit_variable_declaration",       // muffinCount
                            "visit_enter_function_scope_body",  //
                            "visit_exit_function_scope",        //
                            "visit_exit_class_scope"));         // Monster
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code, diag_missing_class_method_name,  //
                    expected_name, strlen(u8"class Monster { "), u8"")));
  }
}

TEST(test_parse, class_statement_with_fields) {
  {
    spy_visitor v =
        parse_and_visit_statement(u8"class FruitBasket { banana; }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  // FruitBasket
                            "visit_enter_class_scope",     //
                            "visit_property_declaration",  // banana
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"banana"}));
  }

  {
    // ASI after field without initializer.
    spy_visitor v = parse_and_visit_statement(u8"class FruitBasket { banana }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  // FruitBasket
                            "visit_enter_class_scope",     //
                            "visit_property_declaration",  // banana
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"banana"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { prop = init; }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_variable_use",          // init
                            "visit_property_declaration",  // prop
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"prop"}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"init"}));
  }

  {
    // ASI after field with initializer.
    spy_visitor v = parse_and_visit_statement(u8"class C { prop = init }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_variable_use",          // init
                            "visit_property_declaration",  // prop
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"prop"}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"init"}));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { static prop = init }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_variable_use",          // init
                            "visit_property_declaration",  // prop
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"prop"}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"init"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { #prop = init; }");
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"#prop"}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"init"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(
        u8"class C { #prop = init;\nf() {this.#prop;} }");
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"#prop"},
                    spy_visitor::visited_property_declaration{u8"f"}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"init"}));
  }

  {
    // ASI after field name before private identifier.
    spy_visitor v = parse_and_visit_statement(u8"class C { #first\n#second }");
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"#first"},
                    spy_visitor::visited_property_declaration{u8"#second"}));
  }

  {
    // ASI after initializer before private identifier.
    spy_visitor v =
        parse_and_visit_statement(u8"class C { #first = x\n#second }");
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"#first"},
                    spy_visitor::visited_property_declaration{u8"#second"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { 'fieldName'; }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_property_declaration",  // 'fieldName'
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
  }

  {
    // ASI after field without initializer.
    spy_visitor v = parse_and_visit_statement(u8"class C { 'fieldName' }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_property_declaration",  // 'fieldName'
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { 'fieldName' = init; }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_variable_use",          // init
                            "visit_property_declaration",  // 'fieldName'
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"init"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { 3.14 = pi; }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_variable_use",          // pi
                            "visit_property_declaration",  // 'fieldName'
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"pi"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { [x + y]; }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_variable_use",          // x
                            "visit_variable_use",          // y
                            "visit_property_declaration",  // (x + y)
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"x"},
                            spy_visitor::visited_variable_use{u8"y"}));
  }

  {
    // ASI after field without initializer.
    spy_visitor v = parse_and_visit_statement(u8"class C { [x + y] }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_variable_use",          // x
                            "visit_variable_use",          // y
                            "visit_property_declaration",  // (x + y)
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"x"},
                            spy_visitor::visited_variable_use{u8"y"}));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { [x + y] = init; }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  //
                            "visit_enter_class_scope",     //
                            "visit_variable_use",          // x
                            "visit_variable_use",          // y
                            "visit_variable_use",          // init
                            "visit_property_declaration",  // (x + y)
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
    // TODO(strager): Is this order correct?
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"x"},
                            spy_visitor::visited_variable_use{u8"y"},
                            spy_visitor::visited_variable_use{u8"init"}));
  }

  // TODO(strager): '*field=init' is an error.
  // TODO(strager): 'async field=init' is an error.
  // TODO(strager): 'get field=init' is an error.
  // TODO(strager): 'set field=init' is an error.
}

TEST(test_parse, class_fields_without_initializer_allow_asi_after_name) {
  {
    spy_visitor v = parse_and_visit_statement(u8"class C { f\ng() {} }");
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // f
                            "visit_property_declaration",       // g
                            "visit_enter_function_scope",       // g
                            "visit_enter_function_scope_body",  // g
                            "visit_exit_function_scope",        // g
                            "visit_exit_class_scope"));         // C
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"f"},
                            spy_visitor::visited_property_declaration{u8"g"}));
  }

  std::vector<string8> class_declarations{
      u8"method() {}",   u8"*method() {}", u8"[expr]() {}",
      u8"'method'() {}", u8"3.14() {}",
  };
  for (string8 keyword : keywords) {
    class_declarations.emplace_back(keyword + u8"() {}");
  }
  for (const string8& second_member : class_declarations) {
    {
      padded_string code(u8"class C { myField\n" + second_member + u8" }");
      SCOPED_TRACE(code);
      spy_visitor v = parse_and_visit_statement(code.string_view());
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{u8"myField"},
                      ::testing::_));
    }

    for (string8 first_member : {u8"3.14", u8"'bananas'", u8"[expr]"}) {
      padded_string code(u8"class C { " + first_member + u8"\n" +
                         second_member + u8" }");
      SCOPED_TRACE(code);
      spy_visitor v = parse_and_visit_statement(code.string_view());
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{std::nullopt},
                      ::testing::_));
    }
  }
}

TEST(test_parse, class_methods_should_not_use_function_keyword) {
  {
    spy_visitor v;
    padded_string code(u8"class C { function f() {} }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",       // C
                                      "visit_enter_class_scope",          //
                                      "visit_property_declaration",       // f
                                      "visit_enter_function_scope",       //
                                      "visit_enter_function_scope_body",  //
                                      "visit_exit_function_scope",        //
                                      "visit_exit_class_scope"));
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code, diag_methods_should_not_use_function_keyword,  //
                    function_token, strlen(u8"class C { "), u8"function")));
  }

  {
    spy_visitor v;
    padded_string code(u8"class C { async function f() {} }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code, diag_methods_should_not_use_function_keyword,  //
            function_token, strlen(u8"class C { async "), u8"function")));
  }

  {
    spy_visitor v;
    padded_string code(u8"class C { function* f() {} }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code, diag_methods_should_not_use_function_keyword,  //
                    function_token, strlen(u8"class C { "), u8"function")));
  }

  {
    spy_visitor v;
    padded_string code(u8"class C { static function f() {} }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code, diag_methods_should_not_use_function_keyword,  //
            function_token, strlen(u8"class C { static "), u8"function")));
  }
}

TEST(test_parse, class_statement_with_keyword_property) {
  for (string8 keyword : keywords) {
    {
      string8 code = u8"class C { " + keyword + u8"(){} }";
      SCOPED_TRACE(out_string8(code));
      spy_visitor v = parse_and_visit_statement(code.c_str());
      ASSERT_EQ(v.property_declarations.size(), 1);
      EXPECT_EQ(v.property_declarations[0].name, keyword);
    }

    {
      string8 code = u8"class C { *" + keyword + u8"(){} }";
      SCOPED_TRACE(out_string8(code));
      spy_visitor v = parse_and_visit_statement(code.c_str());
      ASSERT_EQ(v.property_declarations.size(), 1);
      EXPECT_EQ(v.property_declarations[0].name, keyword);
    }

    for (string8 prefix : {u8"async", u8"get", u8"set", u8"static",
                           u8"static async", u8"static get", u8"static set"}) {
      string8 code = u8"class C { " + prefix + u8" " + keyword + u8"(){} }";
      SCOPED_TRACE(out_string8(code));
      spy_visitor v = parse_and_visit_statement(code.c_str());
      ASSERT_EQ(v.property_declarations.size(), 1);
      EXPECT_EQ(v.property_declarations[0].name, keyword);
    }

    {
      string8 code = u8"class C { " + keyword + u8" }";
      SCOPED_TRACE(out_string8(code));
      spy_visitor v = parse_and_visit_statement(code.c_str());
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{keyword}));
    }

    {
      string8 code = u8"class C { " + keyword + u8"; }";
      SCOPED_TRACE(out_string8(code));
      spy_visitor v = parse_and_visit_statement(code.c_str());
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{keyword}));
    }

    {
      string8 code = u8"class C { " + keyword + u8" = init; }";
      SCOPED_TRACE(out_string8(code));
      spy_visitor v = parse_and_visit_statement(code.c_str());
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{keyword}));
    }
  }

  for (string8 keyword : strict_reserved_keywords) {
    string8 property = escape_first_character_in_keyword(keyword);
    for (string8 prefix :
         {u8"", u8"*", u8"async", u8"async *", u8"get", u8"set", u8"static",
          u8"static *", u8"static async", u8"static async *", u8"static get",
          u8"static set"}) {
      padded_string code(u8"class C { " + prefix + u8" " + property +
                         u8"(){} }");
      SCOPED_TRACE(code);
      spy_visitor v = parse_and_visit_statement(code.string_view());
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{keyword}));
    }
  }
}

TEST(test_parse, typescript_class_statement_with_readonly_keyword_property) {
  for (string8 keyword : keywords) {
    {
      string8 code = u8"class C { readonly " + keyword + u8"; }";
      SCOPED_TRACE(out_string8(code));
      spy_visitor v = parse_and_visit_typescript_statement(code.c_str());
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{keyword}));
    }
  }
}

TEST(test_parse, typescript_class_with_keyword_generic_method) {
  for (string8 keyword : keywords) {
    {
      string8 code = u8"class C { " + keyword + u8"<T>(){} }";
      SCOPED_TRACE(out_string8(code));
      spy_visitor v = parse_and_visit_typescript_statement(code.c_str());
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{keyword}));
    }
  }

  {
    // A generic method named 'async' should not be async.
    spy_visitor v = parse_and_visit_typescript_statement(
        u8"class C { async<T>() { let await; await(x); } }"_sv);
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // async
                            "visit_enter_function_scope",       // async
                            "visit_variable_declaration",       // T
                            "visit_enter_function_scope_body",  // async
                            "visit_variable_declaration",       // await
                            "visit_variable_use",               // await
                            "visit_variable_use",               // x
                            "visit_exit_function_scope",        // async
                            "visit_exit_class_scope"));         // C
  }
}

TEST(test_parse, class_statement_with_number_methods) {
  {
    spy_visitor v = parse_and_visit_statement(u8"class Wat { 42.0() { } }"_sv);

    ASSERT_EQ(v.variable_declarations.size(), 1);
    EXPECT_EQ(v.variable_declarations[0].name, u8"Wat");

    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // Wat
                            "visit_enter_class_scope",          //
                            "visit_property_declaration",       // 42.0
                            "visit_enter_function_scope",       //
                            "visit_enter_function_scope_body",  //
                            "visit_exit_function_scope",        //
                            "visit_exit_class_scope"));
  }
}

TEST(test_parse, class_expression) {
  {
    spy_visitor v = parse_and_visit_statement(u8"(class C { })"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",     //
                                      "visit_variable_declaration",  // C
                                      "visit_exit_class_scope"));
    ASSERT_EQ(v.variable_declarations.size(), 1);
    EXPECT_EQ(v.variable_declarations[0].name, u8"C");
    EXPECT_EQ(v.variable_declarations[0].kind, variable_kind::_class);
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"(class { })"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",  //
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"(class { a() {} [b]() {} })"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",          //
                                      "visit_property_declaration",       // a
                                      "visit_enter_function_scope",       // a
                                      "visit_enter_function_scope_body",  // a
                                      "visit_exit_function_scope",        // a
                                      "visit_variable_use",               // b
                                      "visit_property_declaration",       // [b]
                                      "visit_enter_function_scope",       // [b]
                                      "visit_enter_function_scope_body",  // [b]
                                      "visit_exit_function_scope",        // [b]
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"(class A extends B {})"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",     //
                                      "visit_variable_use",          // B
                                      "visit_variable_declaration",  // A
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"(class extends C {})"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",  //
                                      "visit_variable_use",       // C
                                      "visit_exit_class_scope"));
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"(class C {#x = 10; m() {this.#x;}})"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",
                                      "visit_variable_declaration",       // C
                                      "visit_property_declaration",       // x
                                      "visit_property_declaration",       // m
                                      "visit_enter_function_scope",       //
                                      "visit_enter_function_scope_body",  //
                                      "visit_exit_function_scope",        //
                                      "visit_exit_class_scope"));
    EXPECT_THAT(v.errors, IsEmpty());
  }
}

TEST(test_parse, class_statement_allows_stray_semicolons) {
  spy_visitor v = parse_and_visit_statement(u8"class C{ ; f(){} ; }"_sv);
  ASSERT_EQ(v.property_declarations.size(), 1);
  EXPECT_EQ(v.property_declarations[0].name, u8"f");
}

TEST(test_parse, class_method_without_parameter_list) {
  {
    spy_visitor v;
    padded_string code(u8"class C { method { body; } }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          //
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_enter_function_scope_body",  // method
                            "visit_variable_use",               // body
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_missing_function_parameter_list,  //
                              expected_parameter_list,
                              strlen(u8"class C { method"), u8"")));
  }

  {
    spy_visitor v;
    padded_string code(u8"class C { [method+name] { body; } }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_missing_function_parameter_list,  //
                              expected_parameter_list,
                              strlen(u8"class C { [method+name]"), u8"")));
  }

  {
    spy_visitor v;
    padded_string code(u8"class C { 'method name' { body; } }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_missing_function_parameter_list,  //
                              expected_parameter_list,
                              strlen(u8"class C { 'method name'"), u8"")));
  }
}

TEST(test_parse, stray_identifier_before_class_method) {
  {
    spy_visitor v;
    padded_string code(u8"class C { junkIdentifier method(arg) { body; } }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          //
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_variable_declaration",       // arg
                            "visit_enter_function_scope_body",  // method
                            "visit_variable_use",               // body
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"method"}));
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(&code, diag_unexpected_token,  //
                                              token, strlen(u8"class C { "),
                                              u8"junkIdentifier")));
  }

  {
    padded_string code(
        u8"class C { #junkIdentifier #method(arg) { body; } }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"#method"}));
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(&code, diag_unexpected_token,  //
                                              token, strlen(u8"class C { "),
                                              u8"#junkIdentifier")));
  }

  {
    spy_visitor v;
    padded_string code(
        u8"class C { junkIdentifier *method(arg) { body; } }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          //
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_variable_declaration",       // arg
                            "visit_enter_function_scope_body",  // method
                            "visit_variable_use",               // body
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"method"}));
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(&code, diag_unexpected_token,  //
                                              token, strlen(u8"class C { "),
                                              u8"junkIdentifier")));
  }
}

TEST(test_parse, stray_left_curly_in_class_is_ignored) {
  // TODO(strager): Is this the right approach? What about 'class C { { } }'?
  {
    spy_visitor v;
    padded_string code(u8"class C { { method() {} }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"method"}));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_unexpected_token,  //
                              token, strlen(u8"class C { "), u8"{")));
  }
}

TEST(test_parse, stray_keyword_in_class_body) {
  {
    spy_visitor v;
    padded_string code(
        u8"class C { if method(arg) { body; } instanceof myField; }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.errors,
                UnorderedElementsAre(
                    DIAG_TYPE_OFFSETS(&code, diag_unexpected_token,  //
                                      token, strlen(u8"class C { "), u8"if"),
                    DIAG_TYPE_OFFSETS(
                        &code, diag_unexpected_token,  //
                        token, strlen(u8"class C { if method(arg) { body; } "),
                        u8"instanceof")));
  }
}

TEST(test_parse, class_statement_as_do_while_statement_body_is_disallowed) {
  {
    padded_string code(u8"do class C {} while (cond);"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_exit_class_scope",      // C
                                      "visit_variable_use"));        // cond
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_3_FIELDS(
            diag_class_statement_not_allowed_in_body, kind_of_statement,
            statement_kind::do_while_loop,                                //
            expected_body, offsets_matcher(&code, strlen(u8"do"), u8""),  //
            class_keyword,
            offsets_matcher(&code, strlen(u8"do "), u8"class"))));
  }
}

TEST(test_parse, class_statement_as_if_statement_body_is_disallowed) {
  {
    padded_string code(u8"if (cond) class C {} after"_sv);
    spy_visitor v;
    parser p(&code, &v);
    p.parse_and_visit_module(v);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_use",          // cond
                                      "visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_exit_class_scope",      // C
                                      "visit_variable_use",          // after
                                      "visit_end_of_module"));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_3_FIELDS(
            diag_class_statement_not_allowed_in_body, kind_of_statement,
            statement_kind::if_statement,  //
            expected_body,
            offsets_matcher(&code, strlen(u8"if (cond)"), u8""),  //
            class_keyword,
            offsets_matcher(&code, strlen(u8"if (cond) "), u8"class"))));
  }

  {
    padded_string code(u8"if (cond) class C {} else {}"_sv);
    spy_visitor v;
    parser p(&code, &v);
    p.parse_and_visit_module(v);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_use",          // cond
                                      "visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_exit_class_scope",      // C
                                      "visit_enter_block_scope",     // else
                                      "visit_exit_block_scope",      // else
                                      "visit_end_of_module"));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_3_FIELDS(
            diag_class_statement_not_allowed_in_body, kind_of_statement,
            statement_kind::if_statement,  //
            expected_body,
            offsets_matcher(&code, strlen(u8"if (cond)"), u8""),  //
            class_keyword,
            offsets_matcher(&code, strlen(u8"if (cond) "), u8"class"))));
  }

  {
    padded_string code(u8"if (cond) {} else class C {}"_sv);
    spy_visitor v;
    parser p(&code, &v);
    p.parse_and_visit_module(v);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_use",          // cond
                                      "visit_enter_block_scope",     // if
                                      "visit_exit_block_scope",      // if
                                      "visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_exit_class_scope",      // C
                                      "visit_end_of_module"));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_3_FIELDS(
            diag_class_statement_not_allowed_in_body, kind_of_statement,
            statement_kind::if_statement,  //
            expected_body,
            offsets_matcher(&code, strlen(u8"if (cond) {} else"), u8""),  //
            class_keyword,
            offsets_matcher(&code, strlen(u8"if (cond) {} else "),
                            u8"class"))));
  }
}

TEST(test_parse, class_statement_as_for_statement_body_is_disallowed) {
  {
    padded_string code(u8"for (;cond;) class C {}"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_use",          // cond
                                      "visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_exit_class_scope"));    // C
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_3_FIELDS(
            diag_class_statement_not_allowed_in_body, kind_of_statement,
            statement_kind::for_loop,  //
            expected_body,
            offsets_matcher(&code, strlen(u8"for (;cond;)"), u8""),  //
            class_keyword,
            offsets_matcher(&code, strlen(u8"for (;cond;) "), u8"class"))));
  }
}

TEST(test_parse, class_statement_as_while_statement_body_is_disallowed) {
  {
    padded_string code(u8"while (cond) class C {}"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_use",          // cond
                                      "visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_exit_class_scope"));    // C
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_3_FIELDS(
            diag_class_statement_not_allowed_in_body, kind_of_statement,
            statement_kind::while_loop,  //
            expected_body,
            offsets_matcher(&code, strlen(u8"while (cond)"), u8""),  //
            class_keyword,
            offsets_matcher(&code, strlen(u8"while (cond) "), u8"class"))));
  }
}

TEST(test_parse, class_statement_as_with_statement_body_is_disallowed) {
  {
    padded_string code(u8"with (obj) class C {}"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_use",          // obj
                                      "visit_enter_with_scope",      // with
                                      "visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_exit_class_scope",      // C
                                      "visit_exit_with_scope"));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_3_FIELDS(
            diag_class_statement_not_allowed_in_body, kind_of_statement,
            statement_kind::with_statement,  //
            expected_body,
            offsets_matcher(&code, strlen(u8"with (obj)"), u8""),  //
            class_keyword,
            offsets_matcher(&code, strlen(u8"with (obj) "), u8"class"))));
  }
}

TEST(test_parse, class_in_async_function_is_allowed) {
  {
    spy_visitor v = parse_and_visit_statement(
        u8"async function f() {"
        u8"  class C {}"
        u8"}");
    EXPECT_THAT(v.errors, IsEmpty());
  }
}

TEST(test_parse, class_named_await_in_async_function) {
  {
    spy_visitor v = parse_and_visit_statement(u8"class await {}");
    EXPECT_THAT(v.errors, IsEmpty());
  }

  {
    spy_visitor v = parse_and_visit_statement(
        u8"function f() {"
        u8"class await {}"
        u8"}");
    EXPECT_THAT(v.errors, IsEmpty());
  }

  {
    padded_string code(u8"async function g() { class await {} }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    p.parse_and_visit_module(v);
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code, diag_cannot_declare_class_named_await_in_async_function,
            name, strlen(u8"async function g() { class "), u8"await")));
  }
}

TEST(test_parse, async_static_method_is_disallowed) {
  {
    spy_visitor v;
    padded_string code(
        u8"class C { async static m() { await myPromise; } }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));

    EXPECT_EQ(v.property_declarations[0].name, u8"m");
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"myPromise"}));

    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          //
                            "visit_property_declaration",       // m
                            "visit_enter_function_scope",       // m
                            "visit_enter_function_scope_body",  // m
                            "visit_variable_use",               // myPromise
                            "visit_exit_function_scope",        // m
                            "visit_exit_class_scope"));

    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code, diag_async_static_method,  //
                    async_static, strlen(u8"class C { "), u8"async static")));
  }

  {
    spy_visitor v;
    padded_string code(
        u8"class C { async static static() { await myPromise; } }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_EQ(v.property_declarations[0].name, u8"static");
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code, diag_async_static_method,  //
                    async_static, strlen(u8"class C { "), u8"async static")));
  }

  {
    spy_visitor v;
    padded_string code(
        u8"class C { async static *m() { await myPromise; yield 42; } }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_EQ(v.property_declarations[0].name, u8"m");
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code, diag_async_static_method,  //
                    async_static, strlen(u8"class C { "), u8"async static")));
  }
}

TEST(test_parse, static_method_allows_newline_after_static_keyword) {
  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { static\n m() { } }"_sv);
    EXPECT_EQ(v.property_declarations[0].name, u8"m");
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { static\n *m() { } }"_sv);
    EXPECT_EQ(v.property_declarations[0].name, u8"m");
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { static\n async *m() { } }"_sv);
    EXPECT_EQ(v.property_declarations[0].name, u8"m");
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { static\n async\n *m() { } }"_sv);
    EXPECT_EQ(v.property_declarations[0].name, u8"async");
    EXPECT_EQ(v.property_declarations[1].name, u8"m");
  }
}

TEST(test_parse, async_method_prohibits_newline_after_async_keyword) {
  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { async\n m() { } }"_sv);
    EXPECT_EQ(v.property_declarations[0].name, u8"async");
    EXPECT_EQ(v.property_declarations[1].name, u8"m");
  }

  {
    spy_visitor v =
        parse_and_visit_statement(u8"class C { async\n static m() { } }"_sv);
    EXPECT_EQ(v.property_declarations[0].name, u8"async");
    EXPECT_EQ(v.property_declarations[1].name, u8"m");
  }

  {
    spy_visitor v = parse_and_visit_statement(u8"class C { async\n = 42 }"_sv);
    EXPECT_EQ(v.property_declarations[0].name, u8"async");
  }
}

TEST(test_parse, typescript_style_const_field) {
  {
    spy_visitor v;
    padded_string code(u8"class C { const f = null }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"f"}));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_typescript_style_const_field,  //
                              const_token, strlen(u8"class C { "), u8"const")));
  }
  {
    spy_visitor v;
    padded_string code(u8"class C { const f }"_sv);
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.property_declarations,
                ElementsAre(spy_visitor::visited_property_declaration{u8"f"}));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_typescript_style_const_field,  //
                              const_token, strlen(u8"class C { "), u8"const")));
  }
}

TEST(test_parse, class_expression_body_is_visited_first_in_expression) {
  {
    padded_string code(u8"[before, class C { m() { inside; } }, after];"sv);
    spy_visitor v = parse_and_visit_statement(&code);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",          // C
                                      "visit_variable_declaration",       // C
                                      "visit_property_declaration",       // m
                                      "visit_enter_function_scope",       // m
                                      "visit_enter_function_scope_body",  // m
                                      "visit_variable_use",         // inside
                                      "visit_exit_function_scope",  //
                                      "visit_exit_class_scope",     // C
                                      "visit_variable_use",         // before
                                      "visit_variable_use"));       // after
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"inside"},
                            spy_visitor::visited_variable_use{u8"before"},
                            spy_visitor::visited_variable_use{u8"after"}));
  }

  {
    padded_string code(
        u8"[before, class C { m() { inside; } }.prop, after] = [1,2,3];"sv);
    spy_visitor v = parse_and_visit_statement(&code);
    EXPECT_THAT(v.visits, ElementsAre("visit_enter_class_scope",          // C
                                      "visit_variable_declaration",       // C
                                      "visit_property_declaration",       // m
                                      "visit_enter_function_scope",       // m
                                      "visit_enter_function_scope_body",  // m
                                      "visit_variable_use",           // inside
                                      "visit_exit_function_scope",    //
                                      "visit_exit_class_scope",       // C
                                      "visit_variable_assignment",    // before
                                      "visit_variable_assignment"));  // after
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"inside"}));
    EXPECT_THAT(
        v.variable_assignments,
        ElementsAre(spy_visitor::visited_variable_assignment{u8"before"},
                    spy_visitor::visited_variable_assignment{u8"after"}));
  }
}

TEST(test_parse, field_with_type_is_disallowed_in_javascript) {
  {
    padded_string code(u8"class C { fieldName: FieldType; }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"fieldName"}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"FieldType"}));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_typescript_type_annotations_not_allowed_in_javascript,  //
            type_colon, strlen(u8"class C { fieldName"), u8":")));
  }
}

TEST(test_parse, field_with_type_is_allowed_in_typescript) {
  {
    spy_visitor v = parse_and_visit_typescript_statement(
        u8"class C { fieldName: FieldType; }"_sv);
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  // C
                            "visit_enter_class_scope",     // C
                            "visit_variable_type_use",     // FieldType
                            "visit_property_declaration",  // fieldName
                            "visit_exit_class_scope"));    // C
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"fieldName"}));
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"FieldType"}));
  }
}

TEST(test_parse, class_index_signature_is_disallowed_in_javascript) {
  {
    padded_string code(u8"class C { [key: KeyType]: ValueType; }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    p.parse_and_visit_module_catching_fatal_parse_errors(v);
    // TODO(strager): Improve this error message.
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code, diag_unexpected_token,  //
                              token, strlen(u8"class C { [key"), u8":")));
  }
}

TEST(test_parse, class_index_signature_is_allowed_in_typescript) {
  {
    spy_visitor v = parse_and_visit_typescript_statement(
        u8"class C { [key: KeyType]: ValueType; }"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",         // C
                                      "visit_enter_class_scope",            // C
                                      "visit_enter_index_signature_scope",  //
                                      "visit_variable_type_use",     // KeyType
                                      "visit_variable_declaration",  // key
                                      "visit_variable_type_use",  // ValueType
                                      "visit_exit_index_signature_scope",  //
                                      "visit_exit_class_scope"));          // C
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"KeyType"},
                            spy_visitor::visited_variable_use{u8"ValueType"}));
    // TODO(strager): We probably should create a new kind of variable instead
    // of 'parameter'.
    EXPECT_THAT(
        v.variable_declarations,
        ElementsAre(
            spy_visitor::visited_variable_declaration{
                u8"C", variable_kind::_class, variable_init_kind::normal},
            spy_visitor::visited_variable_declaration{
                u8"key", variable_kind::_parameter,
                variable_init_kind::normal}));
  }
}

TEST(test_parse, optional_properties_are_disallowed_in_javascript) {
  {
    padded_string code(u8"class C { field1?; field2? = init; }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_property_declaration",  // field1
                                      "visit_variable_use",          // init
                                      "visit_property_declaration",  // field2
                                      "visit_exit_class_scope"));    // C
    EXPECT_THAT(
        v.errors,
        ElementsAre(
            DIAG_TYPE_OFFSETS(
                &code,
                diag_typescript_optional_properties_not_allowed_in_javascript,  //
                question, strlen(u8"class C { field1"), u8"?"),
            DIAG_TYPE_OFFSETS(
                &code,
                diag_typescript_optional_properties_not_allowed_in_javascript,  //
                question, strlen(u8"class C { field1?; field2"), u8"?")));
  }
}

TEST(test_parse, optional_methods_are_disallowed_in_classes) {
  for (parser_options options : {parser_options(), typescript_options}) {
    SCOPED_TRACE(options.typescript ? "typescript" : "javascript");
    padded_string code(u8"class C { method?() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v, options);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_typescript_optional_properties_not_allowed_on_methods,  //
            question, strlen(u8"class C { method"), u8"?")));
  }
}

TEST(test_parse, assignment_asserted_fields_are_disallowed_in_javascript) {
  {
    padded_string code(u8"class C { field1!; field2! = init; }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_property_declaration",  // field1
                                      "visit_variable_use",          // init
                                      "visit_property_declaration",  // field2
                                      "visit_exit_class_scope"));    // C
    EXPECT_THAT(
        v.errors,
        ElementsAre(
            DIAG_TYPE_OFFSETS(
                &code,
                diag_typescript_assignment_asserted_fields_not_allowed_in_javascript,  //
                bang, strlen(u8"class C { field1"), u8"!"),
            DIAG_TYPE_OFFSETS(
                &code,
                diag_typescript_assignment_asserted_fields_not_allowed_in_javascript,  //
                bang, strlen(u8"class C { field1?; field2"), u8"!")));
  }
}

TEST(test_parse, assignment_asserted_fields_are_allowed_in_typescript) {
  {
    spy_visitor v = parse_and_visit_typescript_statement(
        u8"class C { field1!; field2! = init; }"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_property_declaration",  // field1
                                      "visit_variable_use",          // init
                                      "visit_property_declaration",  // field2
                                      "visit_exit_class_scope"));    // C
  }
}

TEST(test_parse, assignment_asserted_methods_are_not_allowed) {
  {
    padded_string code(u8"class C { method!() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v, typescript_options);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_enter_function_scope_body",  // method
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));         // C
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_typescript_assignment_asserted_fields_not_allowed_on_methods,  //
            bang, strlen(u8"class C { method"), u8"!")));
  }
}

TEST(test_parse, readonly_fields_are_disallowed_in_javascript) {
  {
    padded_string code(u8"class C { readonly field; }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_property_declaration",  // field
                                      "visit_exit_class_scope"));    // C
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_typescript_readonly_fields_not_allowed_in_javascript,  //
            readonly_keyword, strlen(u8"class C { "), u8"readonly")));
  }

  {
    padded_string code(u8"class C { readonly field = null; }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_typescript_readonly_fields_not_allowed_in_javascript,  //
            readonly_keyword, strlen(u8"class C { "), u8"readonly")));
  }

  {
    padded_string code(u8"class C { readonly field\nmethod() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_typescript_readonly_fields_not_allowed_in_javascript,  //
            readonly_keyword, strlen(u8"class C { "), u8"readonly")));
  }

  {
    padded_string code(u8"class C { readonly field\n[methodName]() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_typescript_readonly_fields_not_allowed_in_javascript,  //
            readonly_keyword, strlen(u8"class C { "), u8"readonly")));
  }

  {
    padded_string code(u8"class C { readonly async\nmethod() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // async
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_enter_function_scope_body",  // method
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));         // C
    EXPECT_THAT(
        v.errors,
        ElementsAre(DIAG_TYPE_OFFSETS(
            &code,
            diag_typescript_readonly_fields_not_allowed_in_javascript,  //
            readonly_keyword, strlen(u8"class C { "), u8"readonly")));
  }

  {
    padded_string code(u8"class C { readonly field? method() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.errors,
        UnorderedElementsAre(
            DIAG_TYPE(diag_missing_semicolon_after_field),
            DIAG_TYPE(
                diag_typescript_optional_properties_not_allowed_in_javascript),
            DIAG_TYPE_OFFSETS(
                &code,
                diag_typescript_readonly_fields_not_allowed_in_javascript,  //
                readonly_keyword, strlen(u8"class C { "), u8"readonly")));
  }
}

TEST(test_parse, readonly_fields_are_allowed_in_typescript) {
  {
    spy_visitor v = parse_and_visit_typescript_statement(
        u8"class C { readonly field; }"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_property_declaration",  // field
                                      "visit_exit_class_scope"));    // C
  }

  {
    spy_visitor v = parse_and_visit_typescript_statement(
        u8"class C { static readonly field; }"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_property_declaration",  // field
                                      "visit_exit_class_scope"));    // C
  }

  {
    spy_visitor v = parse_and_visit_typescript_statement(
        u8"class C { readonly #field; }"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_declaration",  // C
                                      "visit_enter_class_scope",     // C
                                      "visit_property_declaration",  // #field
                                      "visit_exit_class_scope"));    // C
  }
}

TEST(test_parse, readonly_methods_are_invalid) {
  {
    padded_string code(u8"class C { readonly method() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_enter_function_scope_body",  // method
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));         // C
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code,
                    diag_typescript_readonly_method,  //
                    readonly_keyword, strlen(u8"class C { "), u8"readonly")));
  }
}

TEST(test_parse, readonly_static_field_is_disallowed) {
  {
    padded_string code(u8"class C { readonly static field; }"_sv);
    spy_visitor v;
    parser p(&code, &v, typescript_options);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",  // C
                            "visit_enter_class_scope",     // C
                            "visit_property_declaration",  // field
                            "visit_exit_class_scope"));    // C
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"field"}));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code,
                              diag_readonly_static_field,  //
                              readonly_static, strlen(u8"class C { "),
                              u8"readonly static")));
  }
}

TEST(test_parse, generic_methods_are_disallowed_in_javascript) {
  {
    padded_string code(u8"class C { method<T>() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_variable_declaration",       // T
                            "visit_enter_function_scope_body",  // method
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));         // C
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"method"}));
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code,
                    diag_typescript_generics_not_allowed_in_javascript,  //
                    opening_less, strlen(u8"class C { method"), u8"<")));
  }
}

TEST(test_parse, generic_methods_are_allowed_in_typescript) {
  {
    spy_visitor v =
        parse_and_visit_typescript_statement(u8"class C { method<T>() {} }"_sv);
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_variable_declaration",       // T
                            "visit_enter_function_scope_body",  // method
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));         // C
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"method"}));
  }
}

TEST(test_parse, call_signatures_are_disallowed_in_typescript_classes) {
  {
    padded_string code(u8"class C { () {} }"_sv);
    spy_visitor v;
    parser p(&code, &v, typescript_options);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.visits,
        ElementsAre("visit_variable_declaration",       // C
                    "visit_enter_class_scope",          // C
                    "visit_property_declaration",       // (call signature)
                    "visit_enter_function_scope",       // (call signature)
                    "visit_enter_function_scope_body",  // (call signature)
                    "visit_exit_function_scope",        // (call signature)
                    "visit_exit_class_scope"));         // C
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
    EXPECT_THAT(v.errors, ElementsAre(DIAG_TYPE_OFFSETS(
                              &code,
                              diag_missing_class_method_name,  //
                              expected_name, strlen(u8"class C { "), u8"")));
  }

  {
    padded_string code(u8"class C { <T>() {} }"_sv);
    spy_visitor v;
    parser p(&code, &v, typescript_options);
    EXPECT_TRUE(p.parse_and_visit_statement(v));
    EXPECT_THAT(
        v.visits,
        ElementsAre("visit_variable_declaration",       // C
                    "visit_enter_class_scope",          // C
                    "visit_property_declaration",       // (call signature)
                    "visit_enter_function_scope",       // (call signature)
                    "visit_variable_declaration",       // T
                    "visit_enter_function_scope_body",  // (call signature)
                    "visit_exit_function_scope",        // (call signature)
                    "visit_exit_class_scope"));         // C
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{std::nullopt}));
    EXPECT_THAT(v.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    &code,
                    diag_typescript_call_signatures_not_allowed_in_classes,  //
                    expected_method_name, strlen(u8"class C { "), u8"")));
  }
}

TEST(test_parse, access_specifiers_are_disallowed_in_javascript) {
  for (string8 specifier : {u8"public", u8"protected", u8"private"}) {
    {
      padded_string code(u8"class C { " + specifier + u8" method() {} }");
      SCOPED_TRACE(code);
      spy_visitor v;
      parser p(&code, &v);
      EXPECT_TRUE(p.parse_and_visit_statement(v));
      EXPECT_THAT(v.visits,
                  ElementsAre("visit_variable_declaration",       // C
                              "visit_enter_class_scope",          // C
                              "visit_property_declaration",       // method
                              "visit_enter_function_scope",       // method
                              "visit_enter_function_scope_body",  // method
                              "visit_exit_function_scope",        // method
                              "visit_exit_class_scope"));         // C
      EXPECT_THAT(
          v.property_declarations,
          ElementsAre(spy_visitor::visited_property_declaration{u8"method"}));
      EXPECT_THAT(
          v.errors,
          ElementsAre(DIAG_TYPE_OFFSETS(
              &code,
              diag_typescript_access_specifiers_not_allowed_in_javascript,  //
              specifier, strlen(u8"class C { "), specifier)));
    }

    {
      padded_string code(u8"class C { " + specifier + u8" field }");
      SCOPED_TRACE(code);
      spy_visitor v;
      parser p(&code, &v);
      EXPECT_TRUE(p.parse_and_visit_statement(v));
      EXPECT_THAT(
          v.errors,
          ElementsAre(DIAG_TYPE_OFFSETS(
              &code,
              diag_typescript_access_specifiers_not_allowed_in_javascript,  //
              specifier, strlen(u8"class C { "), specifier)));
    }

    {
      padded_string code(u8"class C { " + specifier + u8" field = init; }");
      SCOPED_TRACE(code);
      spy_visitor v;
      parser p(&code, &v);
      EXPECT_TRUE(p.parse_and_visit_statement(v));
      EXPECT_THAT(
          v.errors,
          ElementsAre(DIAG_TYPE_OFFSETS(
              &code,
              diag_typescript_access_specifiers_not_allowed_in_javascript,  //
              specifier, strlen(u8"class C { "), specifier)));
    }

    {
      padded_string code(u8"class C { " + specifier +
                         u8" field\nmethod() {} }");
      SCOPED_TRACE(code);
      spy_visitor v;
      parser p(&code, &v);
      EXPECT_TRUE(p.parse_and_visit_statement(v));
      EXPECT_THAT(
          v.errors,
          ElementsAre(DIAG_TYPE_OFFSETS(
              &code,
              diag_typescript_access_specifiers_not_allowed_in_javascript,  //
              specifier, strlen(u8"class C { "), specifier)));
    }

    {
      padded_string code(u8"class C { " + specifier +
                         u8" field\n[methodName]() {} }");
      SCOPED_TRACE(code);
      spy_visitor v;
      parser p(&code, &v);
      EXPECT_TRUE(p.parse_and_visit_statement(v));
      EXPECT_THAT(
          v.errors,
          ElementsAre(DIAG_TYPE_OFFSETS(
              &code,
              diag_typescript_access_specifiers_not_allowed_in_javascript,  //
              specifier, strlen(u8"class C { "), specifier)));
    }

    {
      padded_string code(u8"class C { " + specifier +
                         u8" field? method() {} }");
      SCOPED_TRACE(code);
      spy_visitor v;
      parser p(&code, &v);
      EXPECT_TRUE(p.parse_and_visit_statement(v));
      EXPECT_THAT(
          v.errors,
          UnorderedElementsAre(
              DIAG_TYPE(
                  diag_typescript_optional_properties_not_allowed_in_javascript),
              DIAG_TYPE(diag_missing_semicolon_after_field),
              DIAG_TYPE_OFFSETS(
                  &code,
                  diag_typescript_access_specifiers_not_allowed_in_javascript,  //
                  specifier, strlen(u8"class C { "), specifier)));
    }

    {
      padded_string code(u8"class C { " + specifier +
                         u8" async\nmethod() { const await = null; } }");
      SCOPED_TRACE(code);
      spy_visitor v;
      parser p(&code, &v);
      EXPECT_TRUE(p.parse_and_visit_statement(v));
      EXPECT_THAT(
          v.errors,
          ElementsAre(DIAG_TYPE_OFFSETS(
              &code,
              diag_typescript_access_specifiers_not_allowed_in_javascript,  //
              specifier, strlen(u8"class C { "), specifier)));
    }
  }
}

TEST(test_parse, access_specifiers_are_allowed_in_typescript) {
  for (string8 specifier : {u8"public", u8"protected", u8"private"}) {
    padded_string code(u8"class C { " + specifier + u8" method() {} }");
    SCOPED_TRACE(code);
    spy_visitor v = parse_and_visit_typescript_statement(code.string_view());
    EXPECT_THAT(v.visits,
                ElementsAre("visit_variable_declaration",       // C
                            "visit_enter_class_scope",          // C
                            "visit_property_declaration",       // method
                            "visit_enter_function_scope",       // method
                            "visit_enter_function_scope_body",  // method
                            "visit_exit_function_scope",        // method
                            "visit_exit_class_scope"));         // C
    EXPECT_THAT(
        v.property_declarations,
        ElementsAre(spy_visitor::visited_property_declaration{u8"method"}));
  }
}
}
}

// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew "strager" Glazar
//
// This file is part of quick-lint-js.
//
// quick-lint-js is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// quick-lint-js is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with quick-lint-js.  If not, see <https://www.gnu.org/licenses/>.
