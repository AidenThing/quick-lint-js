// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <quick-lint-js/array.h>
#include <quick-lint-js/cli/cli-location.h>
#include <quick-lint-js/container/concat.h>
#include <quick-lint-js/container/padded-string.h>
#include <quick-lint-js/diag-collector.h>
#include <quick-lint-js/diag-matcher.h>
#include <quick-lint-js/diag/diagnostic-types.h>
#include <quick-lint-js/fe/language.h>
#include <quick-lint-js/fe/parse.h>
#include <quick-lint-js/parse-support.h>
#include <quick-lint-js/port/char8.h>
#include <quick-lint-js/spy-visitor.h>
#include <string>
#include <string_view>
#include <vector>

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::IsEmpty;
using ::testing::UnorderedElementsAre;

namespace quick_lint_js {
namespace {
class Test_Parse_Loop : public Test_Parse_Expression {};

TEST_F(Test_Parse_Loop, do_while) {
  {
    Test_Parser p(u8"do { a; } while (b)"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",
                          }));
  }

  {
    Test_Parser p(u8"do do {a;} while(b) while(c);"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // a
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // b
                              "visit_variable_use",       // c
                          }));
  }

  {
    Test_Parser p(u8"do do {a;} while(b); while(c);"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // a
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // b
                              "visit_variable_use",       // c
                          }));
  }

  {
    // 'while(a)' is the body of 'do'-'while(b)'.
    Test_Parser p(u8"do while(a) {b;} while(c);"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // a
                              "visit_enter_block_scope",  // {
                              "visit_variable_use",       // b
                              "visit_exit_block_scope",   // }
                              "visit_variable_use",       // c
                          }));
  }
}

TEST_F(Test_Parse_Loop, do_while_without_parens) {
  {
    Test_Parser p(u8"do {} while cond"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // cond
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Expected_Parentheses_Around_Do_While_Condition,  //
                condition, u8"do {} while "_sv.size(), u8"cond"_sv),
        }));
  }

  {
    Test_Parser p(u8"do {} while cond;"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // cond
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Expected_Parentheses_Around_Do_While_Condition,  //
                condition, u8"do {} while "_sv.size(), u8"cond"_sv),
        }));
  }

  {
    Test_Parser p(u8"{ do {} while cond }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // cond
                              "visit_exit_block_scope",   //
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Expected_Parentheses_Around_Do_While_Condition,  //
                condition, u8"{ do {} while "_sv.size(), u8"cond"_sv),
        }));
  }

  {
    Test_Parser p(u8"do {} while (cond"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // cond
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_2_FIELDS(
                        Diag_Expected_Parenthesis_Around_Do_While_Condition,  //
                        where,
                        Offsets_Matcher(p.code, u8"do {} while (cond"_sv.size(),
                                        u8""_sv),  //
                        token, u8')'),
                }));
  }

  {
    Test_Parser p(u8"do {} while cond)"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // cond
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_2_FIELDS(
                        Diag_Expected_Parenthesis_Around_Do_While_Condition,  //
                        where,
                        Offsets_Matcher(p.code, u8"do {} while "_sv.size(),
                                        u8""_sv),  //
                        token, u8'('),
                }));
  }
}

TEST_F(Test_Parse_Loop, do_while_without_body) {
  {
    Test_Parser p(u8"do\nwhile (cond);"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // cond
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Missing_Body_For_Do_While_Statement,  //
                        do_token, 0, u8"do"_sv),
                }));
  }

  {
    Test_Parser p(u8"{ do while (cond); }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  // {
                              "visit_variable_use",       // cond
                              "visit_exit_block_scope",   // }
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Missing_Body_For_Do_While_Statement,  //
                        do_token, u8"{ "_sv.size(), u8"do"_sv),
                }));
  }
}

TEST_F(Test_Parse_Loop, do_while_without_while_and_condition) {
  {
    Test_Parser p(u8"do {} "_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_OFFSETS(
                p.code,
                Diag_Missing_While_And_Condition_For_Do_While_Statement,  //
                do_token, 0, u8"do"_sv, expected_while, u8"do {}"_sv.size(),
                u8""_sv),
        }));
  }

  {
    Test_Parser p(u8"do {}; while (x);"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // x
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_OFFSETS(
                p.code,
                Diag_Missing_While_And_Condition_For_Do_While_Statement,  //
                do_token, 0, u8"do"_sv, expected_while, u8"do {}"_sv.size(),
                u8""_sv),
        }));
  }
}

TEST_F(Test_Parse_Loop, c_style_for_loop) {
  {
    Test_Parser p(u8"for (;;) { a; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (init; cond; after) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       //
                              "visit_variable_use",       //
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",
                          }));
    EXPECT_THAT(p.variable_uses,
                ElementsAreArray({u8"init", u8"cond", u8"body", u8"after"}));
  }

  for (String8_View variable_kind : {u8"const"_sv, u8"let"_sv}) {
    Test_Parser p(concat(u8"for ("_sv, variable_kind,
                         u8" i = 0; cond; after) { body; }"_sv));
    SCOPED_TRACE(p.code);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  //
                              "visit_variable_use",          //
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",      //
                              "visit_variable_use",          //
                              "visit_exit_for_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var i = 0; ; ) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_declaration",  //
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (i = 0, j = 0; ; ) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_assignment",  // i
                              "visit_variable_assignment",  // j
                              "visit_enter_block_scope",    //
                              "visit_variable_use",         // body
                              "visit_exit_block_scope",
                          }));
  }
}

TEST_F(Test_Parse_Loop, c_style_for_loop_with_in_operator) {
  {
    Test_Parser p(u8"for (a in b; c; d) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_In_Disallowed_In_C_Style_For_Loop,  //
                        in_token, u8"for (a "_sv.size(), u8"in"_sv),
                }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",         // b
                              "visit_variable_assignment",  // a
                              "visit_variable_use",         // c
                              "visit_enter_block_scope",    //
                              "visit_exit_block_scope",     //
                              "visit_variable_use",         // d
                          }));
  }

  {
    Test_Parser p(u8"for (let x = a in b; c; d) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_In_Disallowed_In_C_Style_For_Loop,  //
                        in_token, u8"for (let x = a "_sv.size(), u8"in"_sv),
                }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_use",          // a
                              "visit_variable_use",          // b
                              "visit_variable_declaration",  // x
                              "visit_variable_use",          // c
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_variable_use",          // d
                              "visit_exit_for_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = a in b; c; d) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_In_Disallowed_In_C_Style_For_Loop,  //
                        in_token, u8"for (var x = a "_sv.size(), u8"in"_sv),
                }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          // a
                              "visit_variable_use",          // b
                              "visit_variable_declaration",  // x
                              "visit_variable_use",          // c
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_variable_use",          // d
                          }));
  }
}

TEST_F(Test_Parse_Loop, for_loop_with_missing_component) {
  {
    Test_Parser p(u8"for () {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Header_Of_For_Loop,  //
                              where, u8"for "_sv.size(), u8"()"_sv),
        }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                          }));
  }

  {
    Test_Parser p(u8"for (myVar) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_OFFSETS(
                p.code,
                Diag_Missing_For_Loop_Rhs_Or_Components_After_Expression,  //
                header, u8"for "_sv.size(), u8"(myVar)"_sv, for_token, 0,
                u8"for"_sv),
        }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // myVar
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (let myVar) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_OFFSETS(
                p.code,
                Diag_Missing_For_Loop_Rhs_Or_Components_After_Declaration,  //
                header, u8"for "_sv.size(), u8"(let myVar)"_sv, for_token, 0,
                u8"for"_sv),
        }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  // myVar
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({let_noinit_decl(u8"myVar"_sv)}));
  }

  {
    Test_Parser p(u8"for (init; cond) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_OFFSETS(
                p.code, Diag_C_Style_For_Loop_Is_Missing_Third_Component,  //
                existing_semicolon, u8"for (init"_sv.size(), u8";"_sv,
                expected_last_component, u8"for (init; cond"_sv.size(),
                u8")"_sv),
        }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // init
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                          }));
  }
}

TEST_F(Test_Parse_Loop, for_loop_with_missing_semicolons) {
  {
    Test_Parser p(u8"for (a b; c) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code,
                Diag_Missing_Semicolon_Between_For_Loop_Init_And_Condition,  //
                expected_semicolon, u8"for (a"_sv.size(), u8""_sv),
        }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // a
                              "visit_variable_use",       // b
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // c
                          }));
  }

  {
    Test_Parser p(u8"for (a; b c) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code,
                Diag_Missing_Semicolon_Between_For_Loop_Condition_And_Update,  //
                expected_semicolon, u8"for (a; b"_sv.size(), u8""_sv),
        }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // a
                              "visit_variable_use",       // b
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // c
                          }));
  }
}

TEST_F(Test_Parse_Loop, for_loop_with_extra_semicolons) {
  {
    Test_Parser p(u8"for (;;;) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Unexpected_Semicolon_In_C_Style_For_Loop,  //
                              semicolon, u8"for (;;"_sv.size(), u8";"_sv),
        }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (;; ;;;) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        UnorderedElementsAre(
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Unexpected_Semicolon_In_C_Style_For_Loop,  //
                              semicolon, u8"for (;; "_sv.size(), u8";"_sv),
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Unexpected_Semicolon_In_C_Style_For_Loop,  //
                              semicolon, u8"for (;; ;"_sv.size(), u8";"_sv),
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Unexpected_Semicolon_In_C_Style_For_Loop,  //
                              semicolon, u8"for (;; ;;"_sv.size(), u8";"_sv)));
  }

  {
    Test_Parser p(u8"for (a;b;c;d) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Unexpected_Semicolon_In_C_Style_For_Loop,  //
                              semicolon, u8"for (a;b;c"_sv.size(), u8";"_sv),
        }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // a
                              "visit_variable_use",       // b
                              "visit_variable_use",       // d
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // c
                          }));
    EXPECT_THAT(p.variable_uses,
                ElementsAreArray({u8"a", u8"b", u8"d", u8"c"}));
  }

  {
    Test_Parser p(u8"for (a of b; c; d) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                UnorderedElementsAre(
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Unexpected_Semicolon_In_For_Of_Loop,  //
                        semicolon, u8"for (a of b"_sv.size(), u8";"_sv),
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Unexpected_Semicolon_In_For_Of_Loop,  //
                        semicolon, u8"for (a of b; c"_sv.size(), u8";"_sv)));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",         // b
                              "visit_variable_assignment",  // a
                              "visit_variable_use",         // c
                              "visit_variable_use",         // d
                              "visit_enter_block_scope",    //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"b", u8"c", u8"d"}));
  }

  {
    Test_Parser p(u8"for (var a of b; c) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Unexpected_Semicolon_In_For_Of_Loop,  //
                        semicolon, u8"for (var a of b"_sv.size(), u8";"_sv),
                }));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          // b
                              "visit_variable_declaration",  // a
                              "visit_variable_use",          // c
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var a in b; c; d) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        UnorderedElementsAre(
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Unexpected_Semicolon_In_For_In_Loop,  //
                semicolon, u8"for (var a of b"_sv.size(), u8";"_sv),
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Unexpected_Semicolon_In_For_In_Loop,  //
                semicolon, u8"for (var a of b; c"_sv.size(), u8";"_sv)));
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_declaration",  // a
                              "visit_variable_use",          // b
                              "visit_variable_use",          // c
                              "visit_variable_use",          // d
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
  }
}

TEST_F(Test_Parse_Loop, for_in_loop) {
  {
    Test_Parser p(u8"for (x in xs) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",         //
                              "visit_variable_assignment",  //
                              "visit_enter_block_scope",    //
                              "visit_variable_use",         //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_assignments, ElementsAreArray({u8"x"}));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"xs", u8"body"}));
  }

  {
    Test_Parser p(u8"for (let x in xs) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_use",          //
                              "visit_variable_declaration",  //
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({let_noinit_decl(u8"x"_sv)}));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"xs", u8"body"}));
  }

  {
    Test_Parser p(u8"for (var x in xs) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_declaration",  // x
                              "visit_variable_use",          // xs
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          // body
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({var_noinit_decl(u8"x"_sv)}));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"xs", u8"body"}));
  }

  {
    Test_Parser p(u8"for (const x in []) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(p.errors, IsEmpty());
  }
}

TEST_F(Test_Parse_Loop, for_in_loop_with_destructuring) {
  {
    Test_Parser p(u8"for ([x] in xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_assignments, ElementsAreArray({u8"x"}));
  }

  {
    Test_Parser p(u8"for ({x} in xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_assignments, ElementsAreArray({u8"x"}));
  }

  {
    Test_Parser p(u8"for (let [x] in xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({let_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (let {x} in xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({let_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (const [x] in xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({const_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (const {x} in xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({const_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (var [x] in xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({var_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (var {x} in xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({var_noinit_decl(u8"x"_sv)}));
  }
}

TEST_F(Test_Parse_Loop, for_in_loop_with_var_initializer) {
  {
    Test_Parser p(u8"for (var x = init in xs) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          // init
                              "visit_variable_declaration",  // x
                              "visit_variable_use",          // xs
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          // body
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({var_init_decl(u8"x"_sv)}));
    EXPECT_THAT(p.variable_uses,
                ElementsAreArray({u8"init", u8"xs", u8"body"}));
  }

  {
    Test_Parser p(u8"for (var x = 10 in []) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors, IsEmpty());
  }

  {
    Test_Parser p(u8"for (var x = ++y in []) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          // y
                              "visit_variable_assignment",   // y
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = -y in []) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          // y
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = y + z in []) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          // y
                              "visit_variable_use",          // z
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = () => y in []) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_function_scope",       //
                              "visit_enter_function_scope_body",  //
                              "visit_variable_use",               // y
                              "visit_exit_function_scope",        //
                              "visit_variable_declaration",       // x
                              "visit_enter_block_scope",          //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = (z) => y in []) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_function_scope",       //
                              "visit_variable_declaration",       // z
                              "visit_enter_function_scope_body",  //
                              "visit_variable_use",               // y
                              "visit_exit_function_scope",        //
                              "visit_variable_declaration",       // x
                              "visit_enter_block_scope",          //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = async () => y in []) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_function_scope",       //
                              "visit_enter_function_scope_body",  //
                              "visit_variable_use",               // y
                              "visit_exit_function_scope",        //
                              "visit_variable_declaration",       // x
                              "visit_enter_block_scope",          //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = async (z) => y in []) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_function_scope",       //
                              "visit_variable_declaration",       // z
                              "visit_enter_function_scope_body",  //
                              "visit_variable_use",               // y
                              "visit_exit_function_scope",        //
                              "visit_variable_declaration",       // x
                              "visit_enter_block_scope",          //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = <T,>() => y in []) {}"_sv,
                  typescript_options);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_function_scope",       //
                              "visit_variable_declaration",       // T
                              "visit_enter_function_scope_body",  //
                              "visit_variable_use",               // y
                              "visit_exit_function_scope",        //
                              "visit_variable_declaration",       // x
                              "visit_enter_block_scope",          //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = y ? z : w in []) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          // y
                              "visit_variable_use",          // z
                              "visit_variable_use",          // w
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (var x = yield y in []) {}"_sv);
    auto guard = p.enter_function(Function_Attributes::generator);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          // y
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
  }

  // Previously, there was a bug which caused errors in parse_expression after
  // 'in' to be reported twice.
  {
    Test_Parser p(u8"for (var x = 0 in ()) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE(Diag_Missing_Expression_Between_Parentheses),
                }));
  }
}

TEST_F(Test_Parse_Loop, invalid_for_in_loop) {
  {
    Test_Parser p(u8"for (const x = 10 in []) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code,
                Diag_Cannot_Assign_To_Loop_Variable_In_For_Of_Or_In_Loop,  //
                equal_token, u8"for (const x "_sv.size(), u8"="_sv),
        }));
  }

  {
    Test_Parser p(u8"for (let x = 10 in []) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code,
                Diag_Cannot_Assign_To_Loop_Variable_In_For_Of_Or_In_Loop,  //
                equal_token, u8"for (let x "_sv.size(), u8"="_sv),
        }));
  }
}

TEST_F(Test_Parse_Loop, for_of_loop) {
  {
    Test_Parser p(u8"for (x of xs) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",         //
                              "visit_variable_assignment",  //
                              "visit_enter_block_scope",    //
                              "visit_variable_use",         //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_assignments, ElementsAreArray({u8"x"}));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"xs", u8"body"}));
  }

  {
    Test_Parser p(u8"for (let x of xs) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_use",          //
                              "visit_variable_declaration",  //
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({let_noinit_decl(u8"x"_sv)}));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"xs", u8"body"}));
  }

  {
    Test_Parser p(u8"for (var x of xs) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",          //
                              "visit_variable_declaration",  //
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({var_noinit_decl(u8"x"_sv)}));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"xs", u8"body"}));
  }

  {
    Test_Parser p(u8"for await (let x of xs) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_use",          //
                              "visit_variable_declaration",  //
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (let of myArray) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",    //
                              "visit_variable_use",       // myArray
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Let_With_No_Bindings,  //
                                      where, u8"for ("_sv.size(), u8"let"_sv),
                }));
  }

  {
    Test_Parser p(u8"for (const x of []) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(p.errors, IsEmpty());
  }
}

TEST_F(Test_Parse_Loop, for_of_loop_with_destructuring) {
  {
    Test_Parser p(u8"for ([x] of xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_assignments, ElementsAreArray({u8"x"}));
  }

  {
    Test_Parser p(u8"for ({x} of xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_assignments, ElementsAreArray({u8"x"}));
  }

  {
    Test_Parser p(u8"for (let [x] of xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({let_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (let {x} of xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({let_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (const [x] of xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({const_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (const {x} of xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({const_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (var [x] of xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({var_noinit_decl(u8"x"_sv)}));
  }

  {
    Test_Parser p(u8"for (var {x} of xs) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({var_noinit_decl(u8"x"_sv)}));
  }
}

TEST_F(Test_Parse_Loop, invalid_for_of_loop) {
  {
    Test_Parser p(u8"for (const x = 10 of []) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code,
                Diag_Cannot_Assign_To_Loop_Variable_In_For_Of_Or_In_Loop,  //
                equal_token, u8"for (const x "_sv.size(), u8"="_sv),
        }));
  }

  {
    Test_Parser p(u8"for (let x = 10 of []) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  // x
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code,
                Diag_Cannot_Assign_To_Loop_Variable_In_For_Of_Or_In_Loop,  //
                equal_token, u8"for (let x "_sv.size(), u8"="_sv),
        }));
  }

  {
    Test_Parser p(u8"for (var x = 10 of []) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_declaration",  //
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code,
                Diag_Cannot_Assign_To_Loop_Variable_In_For_Of_Or_In_Loop,  //
                equal_token, u8"for (let x "_sv.size(), u8"="_sv),
        }));
  }
}

TEST_F(Test_Parse_Loop, for_loop_without_body) {
  {
    Test_Parser p(u8"for (let x of myArray) "_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_use",          // myArray
                              "visit_variable_declaration",  // x
                              "visit_exit_for_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Body_For_For_Statement,  //
                              for_and_header,
                              u8"for (let x of myArray)"_sv.size(), u8""_sv),
        }));
  }

  {
    Test_Parser p(u8"{ for (let x of myArray) }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",
                              "visit_enter_for_scope",       //
                              "visit_variable_use",          // myArray
                              "visit_variable_declaration",  // x
                              "visit_exit_for_scope",        //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Body_For_For_Statement,  //
                              for_and_header,
                              u8"{ for (let x of myArray)"_sv.size(), u8""_sv),
        }));
  }
}

TEST_F(Test_Parse_Loop, for_loop_without_header) {
  {
    Test_Parser p(u8"for x = y;"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",         // y
                              "visit_variable_assignment",  // x
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Missing_For_Loop_Header,  //
                                      for_token, 0, u8"for"_sv),
                }));
  }

  {
    Test_Parser p(u8"{ for } x = y;"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",    //
                              "visit_exit_block_scope",     //
                              "visit_variable_use",         // y
                              "visit_variable_assignment",  // x
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Missing_For_Loop_Header,  //
                                      for_token, u8"{ "_sv.size(), u8"for"_sv),
                }));
  }
}

TEST_F(Test_Parse_Loop, while_statement) {
  {
    Test_Parser p(u8"while (cond) body;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // cond
                              "visit_variable_use",  // body
                          }));
  }

  {
    Test_Parser p(u8"while (cond) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",
                          }));
  }
}

TEST_F(Test_Parse_Loop, while_without_parens) {
  {
    Test_Parser p(u8"while cond { body; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Expected_Parentheses_Around_While_Condition,  //
                condition, u8"while "_sv.size(), u8"cond"_sv),
        }));
  }

  {
    Test_Parser p(u8"while (cond { body; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_FIELDS(
                Diag_Expected_Parenthesis_Around_While_Condition,  //
                where,
                Offsets_Matcher(p.code, u8"while (cond"_sv.size(), u8""_sv),  //
                token, u8')'),
        }));
  }

  {
    Test_Parser p(u8"while cond) { body; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_FIELDS(
                Diag_Expected_Parenthesis_Around_While_Condition,  //
                where,
                Offsets_Matcher(p.code, u8"while "_sv.size(), u8""_sv),  //
                token, u8'('),
        }));
  }
}

TEST_F(Test_Parse_Loop, while_without_condition) {
  {
    Test_Parser p(u8"while { go(); break; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // go
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Missing_Condition_For_While_Statement,  //
                        while_keyword, 0, u8"while"_sv),
                }));
  }
}

TEST_F(Test_Parse_Loop, while_without_body) {
  {
    Test_Parser p(u8"while (cond) "_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // cond
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Body_For_While_Statement,  //
                              while_and_condition, u8"while (cond)"_sv.size(),
                              u8""_sv),
        }));
  }
}

TEST_F(Test_Parse_Loop, break_statement) {
  {
    Test_Parser p(u8"break;"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, IsEmpty());
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Invalid_Break,  //
                                      break_statement, 0, u8"break"_sv),
                }));
  }

  {
    Test_Parser p(u8"for (;;) { } break;"_sv, capture_diags);
    p.parse_and_visit_statement();
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors, ElementsAreArray({
                              DIAG_TYPE_OFFSETS(p.code, Diag_Invalid_Break,  //
                                                break_statement,
                                                u8"for (;;) { } "_sv.size(),
                                                u8"break"_sv),
                          }));
  }

  {
    Test_Parser p(u8"for (;;) { function f() { break; } }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Invalid_Break,  //
                                      break_statement,
                                      u8"for (;;) { function f() { "_sv.size(),
                                      u8"break"_sv),
                }));
  }

  {
    Test_Parser p(u8"for (;;) { () => { break; } }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Invalid_Break,  //
                              break_statement,
                              u8"for (;;) { () => { "_sv.size(), u8"break"_sv),
        }));
  }

  {
    Test_Parser p(u8"switch (0) { default: break; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"do { break; } while (0);"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (;;) { break; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"while (0) { break; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (;;) { for (;;) { break; } break; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(
        u8"switch (0) { default: switch(0) { default: break; } break; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_exit_block_scope",
                          }));
  }

  // TODO(#72): Visit the label.
  {
    Test_Parser p(u8"break label;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, IsEmpty());
  }
}

TEST_F(Test_Parse_Loop, continue_statement) {
  {
    Test_Parser p(u8"continue;"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, IsEmpty());
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Invalid_Continue,  //
                                      continue_statement, 0, u8"continue"_sv),
                }));
  }

  {
    Test_Parser p(u8"switch (0) { default: continue; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    ASSERT_THAT(p.errors,
                ElementsAre(DIAG_TYPE_OFFSETS(
                    p.code, Diag_Invalid_Continue,  //
                    continue_statement, u8"switch (0) { default: "_sv.size(),
                    u8"continue"_sv)));
  }

  {
    Test_Parser p(u8"for (;;) { function f() { continue; } }"_sv,
                  capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Invalid_Continue,  //
                                      continue_statement,
                                      u8"for (;;) { function f() { "_sv.size(),
                                      u8"continue"_sv),
                }));
  }

  {
    Test_Parser p(u8"for (;;) { () => { continue; } }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Invalid_Continue,  //
                                      continue_statement,
                                      u8"for (;;) { () => { "_sv.size(),
                                      u8"continue"_sv),
                }));
  }

  {
    Test_Parser p(u8"do { continue; } while (0);"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (;;) { continue; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"while (0) { continue; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (;;) { for (;;) { continue; } continue; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_exit_block_scope",
                          }));
  }

  // TODO(#72): Visit the label.
  {
    Test_Parser p(u8"continue label;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, IsEmpty());
  }
}

TEST_F(Test_Parse_Loop,
       break_and_continue_statements_do_not_allow_newline_before_label) {
  {
    Test_Parser p(u8"for (;;) { break\nnotALabel; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // notALabel
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"for (;;) { continue\nnotALabel; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // notALabel
                              "visit_exit_block_scope",
                          }));
  }
}

TEST_F(Test_Parse_Loop,
       break_and_continue_statements_allows_contextual_keyword_as_label) {
  for (String8_View statement : {u8"break"_sv, u8"continue"_sv}) {
    for (String8_View keyword : contextual_keywords) {
      Padded_String code(concat(keyword, u8": for (;;) { "_sv, statement,
                                u8" "_sv, keyword, u8"; }"_sv));
      SCOPED_TRACE(code);

      {
        // Top-level.
        Test_Parser p(code.string_view());
        p.parse_and_visit_statement();
      }

      {
        Test_Parser p(code.string_view());
        auto guard = p.enter_function(Function_Attributes::normal);
        p.parse_and_visit_statement();
      }
    }
  }

  // TODO(#214): Disallow labels named 'await' in async functions.
  // TODO(#214): Disallow labels named 'yield' in generator functions.
}

TEST_F(Test_Parse_Loop,
       for_loop_async_arrow_with_of_parameter_is_init_expression) {
  Test_Parser p(u8"for (async of => x; y; z);"_sv);
  p.parse_and_visit_statement();
  EXPECT_THAT(p.visits, ElementsAreArray({
                            "visit_enter_function_scope",       //
                            "visit_variable_declaration",       // of
                            "visit_enter_function_scope_body",  //
                            "visit_variable_use",               // x
                            "visit_exit_function_scope",        //
                            "visit_variable_use",               // y
                            "visit_variable_use",               // z
                        }));
}

TEST_F(Test_Parse_Loop,
       cannot_assign_to_variable_named_async_without_parentheses_in_for_of) {
  Test_Parser p(u8"for (async of xs) ;"_sv, capture_diags);
  p.parse_and_visit_statement();
  EXPECT_THAT(p.variable_assignments, ElementsAreArray({u8"async"}));
  EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"xs"}));
  EXPECT_THAT(
      p.errors,
      ElementsAreArray({
          DIAG_TYPE_OFFSETS(
              p.code,
              Diag_Cannot_Assign_To_Variable_Named_Async_In_For_Of_Loop,  //
              async_identifier, u8"for ("_sv.size(), u8"async"_sv),
      }));
}

TEST_F(Test_Parse_Loop, for_loop_in_for_loop_header_crash) {
  // There used to be a use-after-free bug caused by a buffering_visitor copying
  // memory into another buffering_visitor, then the parser's
  // buffering_visitor_memory_ being rewind-ed. This test makes sure a
  // regression doesn't happen again (assuming Address Sanitizer catches the
  // use-after-free).
  Test_Parser p(
      u8R"(
        for (var f = () => {
          for (var xs = [x, x, x, x, x, x, x, x, x, x, x, x, x, x];;) {}
        };;) {}
      )"_sv,
      capture_diags);
  p.parse_and_visit_statement();
  EXPECT_THAT(p.variable_uses, Not(IsEmpty()));
  EXPECT_THAT(p.variable_uses, ::testing::Each(u8"x"));
  EXPECT_THAT(p.errors, IsEmpty());
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
