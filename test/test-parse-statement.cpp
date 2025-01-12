// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#include <gmock/gmock-more-matchers.h>
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
using namespace std::literals::string_literals;

namespace quick_lint_js {
namespace {
class Test_Parse_Statement : public Test_Parse_Expression {};

TEST_F(Test_Parse_Statement, return_statement) {
  {
    Test_Parser p(u8"return a;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"a"}));
  }

  {
    Test_Parser p(u8"return a\nreturn b"_sv, capture_diags);
    p.parse_and_visit_statement();
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors, IsEmpty());
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",
                              "visit_variable_use",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"a", u8"b"}));
  }

  {
    Test_Parser p(u8"return a; return b;"_sv, capture_diags);
    p.parse_and_visit_statement();
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors, IsEmpty());
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",
                              "visit_variable_use",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"a", u8"b"}));
  }

  {
    Test_Parser p(u8"if (true) return; x;"_sv, capture_diags);
    p.parse_and_visit_statement();
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors, IsEmpty());
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"x"}));
  }

  {
    Test_Parser p(u8"if (true) { return } else { other }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // other
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"other"}));
  }
}

TEST_F(Test_Parse_Statement, return_statement_disallows_newline) {
  {
    Test_Parser p(u8"return\nx"_sv, capture_diags);

    // Parse 'return'.
    p.parse_and_visit_statement(
        Parser::Parse_Statement_Type::any_statement_in_block);
    EXPECT_THAT(p.variable_uses, IsEmpty());

    // Parse 'x' (separate statement from 'return')
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"x"}));

    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Return_Statement_Returns_Nothing,  //
                              return_keyword, 0, u8"return"_sv),
        }));
  }

  {
    Test_Parser p(u8"if (true) return\nx"_sv);

    // Parse 'if (true) return'.
    p.parse_and_visit_statement(Parser::Parse_Statement_Type::any_statement);
    EXPECT_THAT(p.variable_uses, IsEmpty());

    // Parse 'x' (separate statement from 'return')
    p.parse_and_visit_statement();
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"x"}));
  }

  // TODO(strager): These cases might be dead code instead (e.g. a method call).
  // Report a different error for potentially dead code.
  // TODO(strager): This list is incomplete.
  for (String8_View second_line : {
           u8"!true"_sv,
           u8"'string'"_sv,
           u8"() => {}"_sv,
           u8"(2 + 2)"_sv,
           u8"+42"_sv,
           u8"-42"_sv,
           u8"/=pattern/"_sv,
           u8"/pattern/"_sv,
           u8"42"_sv,
           u8"['a', 'b', 'c']"_sv,
           u8"`template${withSubstitution}`"_sv,
           u8"`template`"_sv,
           u8"await myPromise"_sv,
           u8"false"_sv,
           u8"function f() { }"_sv,
           u8"myVariable"_sv,
           u8"new Promise()"_sv,
           u8"null"_sv,
           u8"super.method()"_sv,
           u8"this"_sv,
           u8"true"_sv,
           u8"typeof banana"_sv,
           u8"{}"_sv,
           u8"~bits"_sv,
           u8"<div>hi</div>"_sv,
           u8"<p></p>"_sv,
           // TODO(strager): Contextual keywords (let, from, yield, etc.).
           // TODO(strager): Function without name. (Must be an expression, not
           // a statement.)
       }) {
    {
      Test_Parser p(concat(u8"return\n"_sv, second_line), jsx_options,
                    capture_diags);
      SCOPED_TRACE(p.code);
      p.parse_and_visit_module();
      EXPECT_THAT(p.errors,
                  ElementsAreArray({
                      DIAG_TYPE_OFFSETS(
                          p.code, Diag_Return_Statement_Returns_Nothing,  //
                          return_keyword, 0, u8"return"_sv),
                  }));
    }

    {
      Test_Parser p(concat(u8"{ return\n"_sv, second_line, u8"}"_sv),
                    jsx_options, capture_diags);
      SCOPED_TRACE(p.code);
      p.parse_and_visit_module();
      EXPECT_THAT(p.errors,
                  ElementsAreArray({
                      DIAG_TYPE_OFFSETS(
                          p.code, Diag_Return_Statement_Returns_Nothing,  //
                          return_keyword, u8"{ "_sv.size(), u8"return"_sv),
                  }));
    }

    {
      Test_Parser p(
          concat(u8"async function f() { return\n"_sv, second_line, u8"}"_sv),
          jsx_options, capture_diags);
      SCOPED_TRACE(p.code);
      p.parse_and_visit_module();
      EXPECT_THAT(p.errors,
                  ElementsAreArray({
                      DIAG_TYPE_OFFSETS(
                          p.code, Diag_Return_Statement_Returns_Nothing,  //
                          return_keyword, u8"async function f() { "_sv.size(),
                          u8"return"_sv),
                  }));
    }

    {
      Test_Parser p(concat(u8"switch (cond) {\n"_sv, u8"default:\n"_sv,
                           u8"return\n"_sv, second_line, u8"}"_sv),
                    jsx_options, capture_diags);
      SCOPED_TRACE(p.code);
      p.parse_and_visit_module();
      EXPECT_THAT(
          p.errors,
          ElementsAreArray({
              DIAG_TYPE_OFFSETS(
                  p.code, Diag_Return_Statement_Returns_Nothing,  //
                  return_keyword, u8"switch (cond) {\ndefault:\n"_sv.size(),
                  u8"return"_sv),
          }));
    }
  }
}

TEST_F(Test_Parse_Statement, return_statement_disallows_newline_in_block) {
  {
    Test_Parser p(u8"for (let x of []) return\nx"_sv);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_for_scope",       //
                              "visit_variable_declaration",  // x
                              "visit_exit_for_scope",        //
                              "visit_variable_use",          // x
                              "visit_end_of_module",
                          }));
  }

  {
    Test_Parser p(u8"if (cond) return\nx"_sv);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // cond
                              "visit_variable_use",  // x
                              "visit_end_of_module",
                          }));
  }

  {
    Test_Parser p(u8"if (cond) {} else return\nx"_sv);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  // (if)
                              "visit_exit_block_scope",   // (if)
                              "visit_variable_use",       // x
                              "visit_end_of_module",
                          }));
  }

  {
    Test_Parser p(u8"while (cond) return\nx"_sv);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // cond
                              "visit_variable_use",  // x
                              "visit_end_of_module",
                          }));
  }
}

TEST_F(Test_Parse_Statement, empty_paren_after_control_statement) {
  {
    Test_Parser p(u8"if(){}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_2_OFFSETS(
                        p.code, Diag_Empty_Paren_After_Control_Statement,
                        expected_expression, u8"if("_sv.size(), u8""_sv, token,
                        u8""_sv.size(), u8"if"_sv),
                }));
  }

  {
    Test_Parser p(u8"switch(){}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_2_OFFSETS(
                        p.code, Diag_Empty_Paren_After_Control_Statement,
                        expected_expression, u8"switch("_sv.size(), u8""_sv,
                        token, u8""_sv.size(), u8"switch"_sv),
                }));
  }

  {
    Test_Parser p(u8"while(){}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_2_OFFSETS(
                        p.code, Diag_Empty_Paren_After_Control_Statement,
                        expected_expression, u8"while("_sv.size(), u8""_sv,
                        token, u8""_sv.size(), u8"while"_sv),
                }));
  }

  {
    Test_Parser p(u8"with(){}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_2_OFFSETS(
                        p.code, Diag_Empty_Paren_After_Control_Statement,
                        expected_expression, u8"with("_sv.size(), u8""_sv,
                        token, u8""_sv.size(), u8"with"_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, throw_statement) {
  {
    Test_Parser p(u8"throw new Error('ouch');"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"Error"}));
  }

  {
    Test_Parser p(u8"throw;"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Expected_Expression_Before_Semicolon,  //
                        where, u8"throw"_sv.size(), u8";"_sv),
                }));
  }

  {
    Test_Parser p(u8"throw\nnew Error();"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Expected_Expression_Before_Newline,  //
                        where, u8"throw"_sv.size(), u8""_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, parse_and_visit_try) {
  {
    Test_Parser p(u8"try {} finally {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"try {} catch (e) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_enter_block_scope",     //
                              "visit_variable_declaration",  //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({catch_decl(u8"e"_sv)}));
  }

  {
    Test_Parser p(u8"try {} catch {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  // try
                              "visit_exit_block_scope",   // try
                              "visit_enter_block_scope",  // catch
                              "visit_exit_block_scope",   // catch
                          }));
    EXPECT_THAT(p.variable_declarations, IsEmpty());
  }

  {
    Test_Parser p(u8"try {} catch (e) {} finally {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_enter_block_scope",     //
                              "visit_variable_declaration",  //
                              "visit_exit_block_scope",      //
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray({catch_decl(u8"e"_sv)}));
  }

  {
    Test_Parser p(u8"try {f();} catch (e) {g();} finally {h();}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",      //
                              "visit_enter_block_scope",     //
                              "visit_variable_declaration",  //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",      //
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"f", u8"g", u8"h"}));
  }

  {
    Test_Parser p(u8"try {} catch ({message, code}) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_enter_block_scope",     // (catch)
                              "visit_variable_declaration",  // message
                              "visit_variable_declaration",  // code
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray(
                    {catch_decl(u8"message"_sv), catch_decl(u8"code"_sv)}));
  }

  {
    Test_Parser p(u8"try {} catch ([message, code]) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     //
                              "visit_exit_block_scope",      //
                              "visit_enter_block_scope",     // (catch)
                              "visit_variable_declaration",  // message
                              "visit_variable_declaration",  // code
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_declarations,
                ElementsAreArray(
                    {catch_decl(u8"message"_sv), catch_decl(u8"code"_sv)}));
  }
}

TEST_F(Test_Parse_Statement, catch_without_try) {
  {
    Test_Parser p(u8"catch (e) { body; }"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     //
                              "visit_variable_declaration",  // e
                              "visit_variable_use",          // body
                              "visit_exit_block_scope",      //
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Catch_Without_Try,  //
                                      catch_token, 0, u8"catch"_sv),
                }));
  }

  {
    Test_Parser p(u8"catch (e) { body; } finally { body; }"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     //
                              "visit_variable_declaration",  // e
                              "visit_variable_use",          // body
                              "visit_exit_block_scope",      //
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          // body
                              "visit_exit_block_scope",      //
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Catch_Without_Try,  //
                                      catch_token, 0, u8"catch"_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, finally_without_try) {
  {
    Test_Parser p(u8"finally { body; }"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",   //
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Finally_Without_Try,  //
                                      finally_token, 0, u8"finally"_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, try_without_catch_or_finally) {
  {
    Test_Parser p(u8"try { tryBody; }\nlet x = 3;"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     // (try)
                              "visit_variable_use",          // tryBody
                              "visit_exit_block_scope",      // (try)
                              "visit_variable_declaration",  // x
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_OFFSETS(
                p.code, Diag_Missing_Catch_Or_Finally_For_Try_Statement,  //
                try_token, 0, u8"try"_sv, expected_catch_or_finally,
                u8"try { tryBody; }"_sv.size(), u8""_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, try_without_body) {
  {
    Test_Parser p(u8"try\nlet x = 3;"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_declaration",  // x
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Body_For_Try_Statement,  //
                              try_token, 0, u8"try"_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, catch_without_body) {
  {
    Test_Parser p(u8"try {} catch\nlet x = 3;"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     // (try)
                              "visit_exit_block_scope",      // (try)
                              "visit_enter_block_scope",     // (catch)
                              "visit_exit_block_scope",      // (catch)
                              "visit_variable_declaration",  // x
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Body_For_Catch_Clause,  //
                              catch_token, u8"try {} catch"_sv.size(), u8""_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, finally_without_body) {
  {
    Test_Parser p(u8"try {} finally\nlet x = 3;"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     // (try)
                              "visit_exit_block_scope",      // (try)
                              "visit_variable_declaration",  // x
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Missing_Body_For_Finally_Clause,  //
                        finally_token, u8"try {} "_sv.size(), u8"finally"_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, catch_without_variable_name_in_parentheses) {
  {
    Test_Parser p(u8"try {} catch () { body; }"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  // (try)
                              "visit_exit_block_scope",   // (try)
                              "visit_enter_block_scope",  // (catch)
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",   // (catch)
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_3_FIELDS(
                        Diag_Missing_Catch_Variable_Between_Parentheses,
                        left_paren_to_right_paren,
                        Offsets_Matcher(p.code, u8"try {} catch "_sv.size(),
                                        u8"()"_sv),  //
                        left_paren,
                        Offsets_Matcher(p.code, u8"try {} catch "_sv.size(),
                                        u8"("_sv),  //
                        right_paren,
                        Offsets_Matcher(p.code, u8"try {} catch ("_sv.size(),
                                        u8")"_sv)),
                }));
  }

  {
    Test_Parser p(u8"try {} catch ('ball') { body; }"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  // (try)
                              "visit_exit_block_scope",   // (try)
                              "visit_enter_block_scope",  // (catch)
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",   // (catch)
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Expected_Variable_Name_For_Catch,  //
                              unexpected_token, u8"try {} catch ("_sv.size(),
                              u8"'ball'"_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, if_without_else) {
  {
    Test_Parser p(u8"if (a) { b; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       //
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"if (a) b;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  //
                              "visit_variable_use",
                          }));
  }
}

TEST_F(Test_Parse_Statement, if_with_else) {
  {
    Test_Parser p(u8"if (a) { b; } else { c; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       //
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       //
                              "visit_exit_block_scope",   //
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"if (a) b; else c;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  //
                              "visit_variable_use",  //
                              "visit_variable_use",
                          }));
  }

  {
    Test_Parser p(u8"if (a) async () => {}; else b;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",               // a
                              "visit_enter_function_scope",       //
                              "visit_enter_function_scope_body",  // {
                              "visit_exit_function_scope",        // }
                              "visit_variable_use",               // b
                          }));
  }
}

TEST_F(Test_Parse_Statement, if_without_body) {
  {
    Test_Parser p(u8"if (a)\nelse e;"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // a
                              "visit_variable_use",  // e
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Body_For_If_Statement,  //
                              expected_body, u8"if (a)"_sv.size(), u8""_sv),
        }));
  }

  {
    Test_Parser p(u8"{\nif (a)\n} b;"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // a
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Body_For_If_Statement,  //
                              expected_body, u8"{\nif (a)"_sv.size(), u8""_sv),
        }));
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // a
                              "visit_exit_block_scope",   //
                              "visit_variable_use",       // b
                          }));
  }

  {
    Test_Parser p(u8"if (a)"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // a
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_Body_For_If_Statement,  //
                              expected_body, u8"if (a)"_sv.size(), u8""_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, if_without_parens) {
  {
    Test_Parser p(u8"if cond { body; }"_sv, capture_diags);
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
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Expected_Parentheses_Around_If_Condition,  //
                              condition, u8"if "_sv.size(), u8"cond"_sv),
        }));
  }

  {
    Test_Parser p(u8"if (cond { body; }"_sv, capture_diags);
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
                Diag_Expected_Parenthesis_Around_If_Condition,  //
                where,
                Offsets_Matcher(p.code, u8"if (cond"_sv.size(), u8""_sv),  //
                token, u8')'),
        }));
  }

  {
    Test_Parser p(u8"if cond) { body; }"_sv, capture_diags);
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
                Diag_Expected_Parenthesis_Around_If_Condition,               //
                where, Offsets_Matcher(p.code, u8"if "_sv.size(), u8""_sv),  //
                token, u8'('),
        }));
  }
}

TEST_F(Test_Parse_Statement, if_without_condition) {
  {
    Test_Parser p(u8"if { yay(); } else { nay(); }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  // (if)
                              "visit_variable_use",       // yay
                              "visit_exit_block_scope",   // (if)
                              "visit_enter_block_scope",  // (else)
                              "visit_variable_use",       // nay
                              "visit_exit_block_scope",   // (else)
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Missing_Condition_For_If_Statement,  //
                        if_keyword, 0, u8"if"_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, else_without_if) {
  {
    Test_Parser p(u8"else { body; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors, ElementsAreArray({
                              DIAG_TYPE_OFFSETS(p.code, Diag_Else_Has_No_If,  //
                                                else_token, 0, u8"else"_sv),
                          }));
  }
}

TEST_F(Test_Parse_Statement, missing_if_after_else) {
  {
    Test_Parser p(u8"if (false) {} else (true) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  // if
                              "visit_exit_block_scope",   // if
                              "visit_enter_block_scope",  // else
                              "visit_exit_block_scope",   // else
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Missing_If_After_Else,  //
                        expected_if, u8"if (false) {} else"_sv.size(), u8""_sv),
                }));
  }

  {
    Test_Parser p(u8"if (x) {} else (y) {} else {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // x
                              "visit_enter_block_scope",  // if
                              "visit_exit_block_scope",   // if
                              "visit_variable_use",       // y
                              "visit_enter_block_scope",  // first else
                              "visit_exit_block_scope",   // first else
                              "visit_enter_block_scope",  // second else
                              "visit_exit_block_scope",   // second else
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Missing_If_After_Else,  //
                                      expected_if, u8"if (x) {} else"_sv.size(),
                                      u8""_sv),
                }));
  }

  {
    Test_Parser p(u8"if (false) {} else true {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
    ElementsAre(DIAG_TYPE_OFFSETS(
        p.code, Diag_Missing_Semicolon_After_Statement,  //
        where, u8"if (false) {} else true"_sv.size(), u8""_sv));
  }

  {
    Test_Parser p(u8"if (false) {} else (true)\n{}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors, IsEmpty());
  }

  {
    Test_Parser p(u8"if (false) {} else (true); {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors, IsEmpty());
  }

  {
    Test_Parser p(u8"if (false) {} else () {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  // if
                              "visit_exit_block_scope",   // if
                              "visit_enter_block_scope",  // else
                              "visit_exit_block_scope",   // else
                          }));
    EXPECT_THAT(
        p.errors,
        UnorderedElementsAre(
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Missing_Expression_Between_Parentheses,  //
                              left_paren_to_right_paren,
                              u8"if (false) {} else "_sv.size(), u8"()"_sv),
            DIAG_TYPE_OFFSETS(p.code, Diag_Missing_If_After_Else,  //
                              expected_if, u8"if (false) {} else"_sv.size(),
                              u8""_sv)))
        << "should not report Diag_Missing_Arrow_Operator_In_Arrow_Function";
  }

  {
    Test_Parser p(u8"if (false) {} else (x, y) {}"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  // if
                              "visit_exit_block_scope",   // if
                              "visit_variable_use",       // x
                              "visit_variable_use",       // y
                              "visit_enter_block_scope",  // else
                              "visit_exit_block_scope",   // else
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Missing_If_After_Else,  //
                        expected_if, u8"if (false) {} else"_sv.size(), u8""_sv),
                }))
        << "should not report Diag_Missing_Arrow_Operator_In_Arrow_Function";
  }
}

TEST_F(Test_Parse_Statement, block_statement) {
  {
    Test_Parser p(u8"{ }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"{ first; second; third; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       //
                              "visit_variable_use",       //
                              "visit_variable_use",       //
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"first",   //
                                                   u8"second",  //
                                                   u8"third"}));
  }
}

TEST_F(Test_Parse_Statement, incomplete_block_statement) {
  {
    Test_Parser p(u8"{ a; "_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // a
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Unclosed_Code_Block,  //
                                      block_open, 0, u8"{"_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, switch_statement) {
  {
    Test_Parser p(u8"switch (x) {}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // x
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"switch (true) {case y:}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // y
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"switch (true) {default:}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"switch (true) {case x: case y: default: case z:}"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // x
                              "visit_variable_use",       // y
                              "visit_variable_use",       // z
                              "visit_exit_block_scope",
                          }));
  }

  {
    Test_Parser p(u8"switch (true) { case true: x; let y; z; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",     //
                              "visit_variable_use",          // x
                              "visit_variable_declaration",  // y
                              "visit_variable_use",          // z
                              "visit_exit_block_scope",
                          }));
  }

  {
    SCOPED_TRACE("':' should not be treated as a type annotation");
    Test_Parser p(u8"switch (true) { case x: Type }"_sv, typescript_options);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // x
                              "visit_variable_use",       // Type
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"x", u8"Type"}));
  }
}

TEST_F(Test_Parse_Statement, switch_without_parens) {
  {
    Test_Parser p(u8"switch cond { case ONE: break; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // ONE
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Expected_Parentheses_Around_Switch_Condition,  //
                condition, u8"switch "_sv.size(), u8"cond"_sv),
        }));
  }

  {
    Test_Parser p(u8"switch (cond { case ONE: break; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // ONE
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_2_FIELDS(
                        Diag_Expected_Parenthesis_Around_Switch_Condition,  //
                        where,
                        Offsets_Matcher(p.code, u8"switch (cond"_sv.size(),
                                        u8""_sv),  //
                        token, u8')'),
                }));
  }

  {
    Test_Parser p(u8"switch cond) { case ONE: break; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // ONE
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_FIELDS(
                Diag_Expected_Parenthesis_Around_Switch_Condition,  //
                where,
                Offsets_Matcher(p.code, u8"switch "_sv.size(), u8""_sv),  //
                token, u8'('),
        }));
  }
}

TEST_F(Test_Parse_Statement, switch_without_condition) {
  {
    Test_Parser p(u8"switch { case ONE: break; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // ONE
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(
                        p.code, Diag_Missing_Condition_For_Switch_Statement,  //
                        switch_keyword, 0, u8"switch"_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, switch_without_body) {
  {
    Test_Parser p(u8"switch (cond);"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // cond
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Missing_Body_For_Switch_Statement,  //
                switch_and_condition, u8"switch (cond)"_sv.size(), u8""_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, switch_without_body_curlies) {
  {
    Test_Parser p(u8"switch (cond) case a: break; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // a
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Expected_Left_Curly,  //
                                      expected_left_curly,
                                      u8"switch (cond)"_sv.size(), u8""_sv),
                }));
  }

  {
    Test_Parser p(u8"switch (cond) default: body; break; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(p.errors,
                ElementsAreArray({
                    DIAG_TYPE_OFFSETS(p.code, Diag_Expected_Left_Curly,  //
                                      expected_left_curly,
                                      u8"switch (cond)"_sv.size(), u8""_sv),
                }));
  }
}

TEST_F(Test_Parse_Statement, switch_case_without_expression) {
  {
    Test_Parser p(u8"switch (cond) { case: banana; break; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // banana
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Expected_Expression_For_Switch_Case,  //
                case_token, u8"switch (cond) { "_sv.size(), u8"case"_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, switch_case_with_duplicated_cases) {
  {
    Test_Parser p(u8"switch (cond) {case x: case y: case y:}"_sv,
                  capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_OFFSETS(
                p.code, Diag_Duplicated_Cases_In_Switch_Statement,
                first_switch_case, u8"switch (cond) {case x: case "_sv.size(),
                u8"y"_sv, duplicated_switch_case,
                u8"switch (cond) {case x: case y: case "_sv.size(), u8"y"_sv),
        }));
  }
  {
    Test_Parser p(
        u8"switch (cond) {case MyEnum.A: break; case MyEnum.A: break;}"_sv,
        capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_OFFSETS(
                p.code, Diag_Duplicated_Cases_In_Switch_Statement,
                first_switch_case, u8"switch (cond) {case "_sv.size(),
                u8"MyEnum.A"_sv, duplicated_switch_case,
                u8"switch (cond) {case MyEnum.A: break; case "_sv.size(),
                u8"MyEnum.A"_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, switch_clause_outside_switch_statement) {
  {
    Test_Parser p(u8"case x:"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // x
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Unexpected_Case_Outside_Switch_Statement,  //
                              case_token, 0, u8"case"_sv),
        }));
  }

  {
    SCOPED_TRACE("':' should not be treated as a type annotation");
    Test_Parser p(u8"case x: Type"_sv, typescript_options, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // x
                              "visit_variable_use",  // Type
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Unexpected_Case_Outside_Switch_Statement,  //
                              case_token, 0, u8"case"_sv),
        }));
  }

  {
    Test_Parser p(u8"case\nif (y) {}"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // y
                              "visit_enter_block_scope",  //
                              "visit_exit_block_scope",   //
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code,
                              Diag_Unexpected_Case_Outside_Switch_Statement,  //
                              case_token, 0, u8"case"_sv),
        }));
  }

  {
    Test_Parser p(u8"default: next;"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // next
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Unexpected_Default_Outside_Switch_Statement,  //
                default_token, 0, u8"default"_sv),
        }));
  }

  {
    Test_Parser p(u8"default\nif (x) body;"_sv, capture_diags);
    p.parse_and_visit_module();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // x
                              "visit_variable_use",  // body
                              "visit_end_of_module",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Unexpected_Default_Outside_Switch_Statement,  //
                default_token, 0, u8"default"_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, with_statement) {
  {
    Test_Parser p(u8"with (cond) body;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",      // cond
                              "visit_enter_with_scope",  // with
                              "visit_variable_use",      // body
                              "visit_exit_with_scope",
                          }));
  }

  {
    Test_Parser p(u8"with (cond) { body; }"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_with_scope",   // with
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",   //
                              "visit_exit_with_scope",
                          }));
  }
}

TEST_F(Test_Parse_Statement, statement_before_first_switch_case) {
  {
    Test_Parser p(u8"switch (cond) { console.log('hi'); case ONE: break; }"_sv,
                  capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // console
                              "visit_variable_use",       // ONE
                              "visit_exit_block_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(p.code, Diag_Statement_Before_First_Switch_Case,
                              unexpected_statement,
                              u8"switch (cond) { "_sv.size(), u8"console"_sv),
        }));
  }
}

TEST_F(Test_Parse_Statement, with_statement_without_parens) {
  {
    Test_Parser p(u8"with cond { body; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_with_scope",   // with
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",   //
                              "visit_exit_with_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_OFFSETS(
                p.code, Diag_Expected_Parentheses_Around_With_Expression,  //
                expression, u8"with "_sv.size(), u8"cond"_sv),
        }));
  }

  {
    Test_Parser p(u8"with (cond { body; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_with_scope",   // with
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",   //
                              "visit_exit_with_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_FIELDS(
                Diag_Expected_Parenthesis_Around_With_Expression,  //
                where,
                Offsets_Matcher(p.code, u8"with (cond"_sv.size(), u8""_sv),  //
                token, u8')'),
        }));
  }

  {
    Test_Parser p(u8"with cond) { body; }"_sv, capture_diags);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",       // cond
                              "visit_enter_with_scope",   // with
                              "visit_enter_block_scope",  //
                              "visit_variable_use",       // body
                              "visit_exit_block_scope",   //
                              "visit_exit_with_scope",
                          }));
    EXPECT_THAT(
        p.errors,
        ElementsAreArray({
            DIAG_TYPE_2_FIELDS(
                Diag_Expected_Parenthesis_Around_With_Expression,  //
                where,
                Offsets_Matcher(p.code, u8"with "_sv.size(), u8""_sv),  //
                token, u8'('),
        }));
  }
}

TEST_F(Test_Parse_Statement, debugger_statement) {
  {
    Test_Parser p(u8"debugger; x;"_sv, capture_diags);
    p.parse_and_visit_statement();
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors, IsEmpty());
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",
                          }));
    EXPECT_THAT(p.variable_uses, ElementsAreArray({u8"x"}));
  }
}

TEST_F(Test_Parse_Statement, labelled_statement) {
  {
    Test_Parser p(u8"some_label: ; x;"_sv, capture_diags);
    p.parse_and_visit_statement();
    p.parse_and_visit_statement();
    EXPECT_THAT(p.errors, IsEmpty());
    // TODO(strager): Announce the label with a visit?
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // x
                          }));
  }

  {
    Test_Parser p(u8"foob: for (;;) body"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // body
                          }));
  }

  {
    Test_Parser p(u8"one: two: three: while (false) body;"_sv);
    p.parse_and_visit_statement();
    EXPECT_THAT(p.visits, ElementsAreArray({
                              "visit_variable_use",  // body
                          }));
  }
}

TEST_F(Test_Parse_Statement, statement_label_can_be_a_contextual_keyword) {
  for (String8_View keyword : contextual_keywords) {
    Padded_String code(String8(keyword) + u8": x;");
    SCOPED_TRACE(code);

    {
      // Top-level.
      Test_Parser p(code.string_view());
      p.parse_and_visit_statement();
      EXPECT_THAT(p.visits, ElementsAreArray({
                                "visit_variable_use",  // x
                            }));
    }

    {
      Test_Parser p(code.string_view());
      auto guard = p.enter_function(Function_Attributes::normal);
      p.parse_and_visit_statement();
      EXPECT_THAT(p.visits, ElementsAreArray({
                                "visit_variable_use",  // x
                            }));
    }
  }
}

TEST_F(Test_Parse_Statement, disallow_label_named_await_in_async_function) {
  Test_Parser p(u8"async function f() {await:}"_sv, capture_diags);
  p.parse_and_visit_statement();
  EXPECT_THAT(p.visits, ElementsAreArray({
                            "visit_variable_declaration",       // f
                            "visit_enter_function_scope",       //
                            "visit_enter_function_scope_body",  //
                            "visit_exit_function_scope",
                        }));
  EXPECT_THAT(
      p.errors,
      ElementsAreArray({
          DIAG_TYPE_2_OFFSETS(
              p.code, Diag_Label_Named_Await_Not_Allowed_In_Async_Function,  //
              await, u8"async function f() {"_sv.size(), u8"await"_sv, colon,
              u8"async function f() {await"_sv.size(), u8":"_sv),
      }));
}

TEST_F(Test_Parse_Statement, disallow_label_named_yield_in_generator_function) {
  Test_Parser p(u8"function *f() {yield:}"_sv, capture_diags);
  p.parse_and_visit_statement();
  EXPECT_THAT(p.visits, ElementsAreArray({
                            "visit_variable_declaration",       // f
                            "visit_enter_function_scope",       //
                            "visit_enter_function_scope_body",  //
                            "visit_exit_function_scope",
                        }));
  EXPECT_THAT(
      p.errors,
      ElementsAreArray({
          DIAG_TYPE_OFFSETS(p.code,
                            Diag_Missing_Semicolon_After_Statement,  //
                            where, u8"function *f() {yield"_sv.size(), u8""_sv),
          DIAG_TYPE_OFFSETS(p.code, Diag_Unexpected_Token,  //
                            token, u8"function *f() {yield"_sv.size(),
                            u8":"_sv),
      }));
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
