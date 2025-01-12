// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#ifndef QUICK_LINT_JS_DIAG_DIAGNOSTIC_TYPES_H
#define QUICK_LINT_JS_DIAG_DIAGNOSTIC_TYPES_H

#include <iosfwd>
#include <quick-lint-js/fe/language.h>
#include <quick-lint-js/fe/source-code-span.h>
#include <quick-lint-js/fe/token.h>
#include <quick-lint-js/i18n/translation.h>
#include <quick-lint-js/port/char8.h>

// QLJS_DIAG_TYPE should have the following signature:
//
// #define QLJS_DIAG_TYPE(error_name, error_code, severity, struct_body,
// format) ...
//
// * error_name: identifier
// * error_code: string literal
// * severity: Diagnostic_Severity value
// * struct_body: class member list, wrapped in { }
// * format: member function calls
//
// A class named *error_name* is created in the quick_lint_js namespace.
// *struct_body* is the body of the class.
//
// *format* should look like the following:
//
//    MESSAGE(QLJS_TRANSLATABLE("format string"), source_location)
//
// Within *format*:
//
// * MESSAGE's first argument must be QLJS_TRANSLATABLE(...)
// * MESSAGE's second argument must be a member variable of the *error_name*
//   class (i.e. listed in *struct_body*)
// * MESSAGE's second argument must have type *Source_Code_Span*
//
// When removing a diagnostic from this list, add an entry to
// QLJS_X_RESERVED_DIAG_TYPES.
#define QLJS_X_DIAG_TYPES                                                       \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Abstract_Field_Cannot_Have_Initializer, "E0295",                     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span equal;                                                 \
        Source_Code_Span abstract_keyword;                                      \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("abstract fields cannot have default values"),  \
              equal) MESSAGE(QLJS_TRANSLATABLE("field marked abstract here"),   \
                             abstract_keyword))                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Abstract_Methods_Cannot_Be_Async, "E0298",                           \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span async_keyword;                                         \
        Source_Code_Span abstract_keyword;                                      \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("abstract methods cannot be marked 'async'"),   \
              async_keyword))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Abstract_Methods_Cannot_Be_Generators, "E0299",                      \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span star;                                                  \
        Source_Code_Span abstract_keyword;                                      \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "abstract methods cannot be marked as a generator"),          \
              star))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Abstract_Property_Not_Allowed_In_Interface, "E0297",                 \
      Diagnostic_Severity::error, { Source_Code_Span abstract_keyword; },       \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "abstract properties are not allowed in interfaces"),         \
              abstract_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Abstract_Property_Not_Allowed_In_Non_Abstract_Class, "E0296",        \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span abstract_keyword;                                      \
        Source_Code_Span class_keyword;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "abstract properties are only allowed in abstract classes"),  \
              abstract_keyword)                                                 \
          MESSAGE(QLJS_TRANSLATABLE("class is not marked abstract"),            \
                  class_keyword))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Abstract_Methods_Cannot_Contain_Bodies, "E0294",                     \
      Diagnostic_Severity::error, { Source_Code_Span body_start; },             \
      MESSAGE(QLJS_TRANSLATABLE("abstract methods cannot contain a body"),      \
              body_start))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Adjacent_JSX_Without_Parent, "E0189", Diagnostic_Severity::error,    \
      {                                                                         \
        Source_Code_Span begin;                                                 \
        Source_Code_Span begin_of_second_element;                               \
        Source_Code_Span end;                                                   \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "missing '<>' and '</>' to enclose multiple children"),       \
              begin) MESSAGE(QLJS_TRANSLATABLE("children end here"), end))      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Arrow_Parameter_With_Type_Annotation_Requires_Parentheses, "E0255",  \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span parameter_and_annotation;                              \
        Source_Code_Span type_colon;                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing parentheses around parameter"),        \
              parameter_and_annotation)                                         \
          MESSAGE(QLJS_TRANSLATABLE(                                            \
                      "TypeScript type annotation requires parentheses"),       \
                  type_colon))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Question_In_Type_Expression_Should_Be_Void, "E0348",      \
      Diagnostic_Severity::error, { Source_Code_Span question; },               \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("invalid usage of ? as a prefix or suffix in "      \
                            "the a type expression, use '| void' instead"),     \
          question))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Assignment_Before_Variable_Declaration, "E0001",                     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span assignment;                                            \
        Source_Code_Span declaration;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("variable assigned before its declaration"),    \
              assignment)                                                       \
          MESSAGE(QLJS_TRANSLATABLE("variable declared here"), declaration))    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Assignment_Makes_Condition_Constant, "E0188",                        \
      Diagnostic_Severity::warning, { Source_Code_Span assignment_operator; },  \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'=' changes variables; to compare, use '===' instead"),      \
              assignment_operator))                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Assignment_To_Const_Global_Variable, "E0002",                        \
      Diagnostic_Severity::error, { Source_Code_Span assignment; },             \
      MESSAGE(QLJS_TRANSLATABLE("assignment to const global variable"),         \
              assignment))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Assignment_To_Const_Variable, "E0003", Diagnostic_Severity::error,   \
      {                                                                         \
        Source_Code_Span declaration;                                           \
        Source_Code_Span assignment;                                            \
        Variable_Kind var_kind;                                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("assignment to const variable"), assignment)    \
          MESSAGE(QLJS_TRANSLATABLE("const variable declared here"),            \
                  declaration))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Assignment_To_Imported_Variable, "E0185",                            \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span declaration;                                           \
        Source_Code_Span assignment;                                            \
        Variable_Kind var_kind;                                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("assignment to imported variable"),             \
              assignment)                                                       \
          MESSAGE(QLJS_TRANSLATABLE("imported variable declared here"),         \
                  declaration))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Assignment_To_Const_Variable_Before_Its_Declaration, "E0004",        \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span declaration;                                           \
        Source_Code_Span assignment;                                            \
        Variable_Kind var_kind;                                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "assignment to const variable before its declaration"),       \
              assignment)                                                       \
          MESSAGE(QLJS_TRANSLATABLE("const variable declared here"),            \
                  declaration))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Assignment_To_Undeclared_Variable, "E0059",                          \
      Diagnostic_Severity::warning, { Source_Code_Span assignment; },           \
      MESSAGE(QLJS_TRANSLATABLE("assignment to undeclared variable"),           \
              assignment))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Await_Operator_Outside_Async, "E0162", Diagnostic_Severity::error,   \
      { Source_Code_Span await_operator; },                                     \
      MESSAGE(QLJS_TRANSLATABLE("'await' is only allowed in async functions"),  \
              await_operator))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Await_Followed_By_Arrow_Function, "E0178",                           \
      Diagnostic_Severity::error, { Source_Code_Span await_operator; },         \
      MESSAGE(QLJS_TRANSLATABLE("'await' cannot be followed by an arrow "       \
                                "function; use 'async' instead"),               \
              await_operator))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Async_Static_Method, "E0269", Diagnostic_Severity::error,            \
      { Source_Code_Span async_static; },                                       \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "'async static' is not allowed; write 'static async' instead"),   \
          async_static))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Async_Export_Function, "E0326", Diagnostic_Severity::error,          \
      { Source_Code_Span async_export; },                                       \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "'async export' is not allowed; write 'export async' instead"),   \
          async_export))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Class_Fields_Cannot_Have_Initializers, "E0335",              \
      Diagnostic_Severity::error, { Source_Code_Span equal; },                  \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("'declare class' fields cannot be initalized"),     \
          equal))                                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Class_Methods_Cannot_Be_Async, "E0338",                      \
      Diagnostic_Severity::error, { Source_Code_Span async_keyword; },          \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'declare class' methods cannot be marked 'async'"),          \
              async_keyword))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Class_Methods_Cannot_Be_Generators, "E0337",                 \
      Diagnostic_Severity::error, { Source_Code_Span star; },                   \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'declare class' methods cannot be marked as a generator"),   \
              star))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Class_Methods_Cannot_Contain_Bodies, "E0333",                \
      Diagnostic_Severity::error, { Source_Code_Span body_start; },             \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("'declare class' methods cannot contain a body"),   \
          body_start))                                                          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Abstract_Class_Not_Allowed_In_JavaScript, "E0340",           \
      Diagnostic_Severity::error, { Source_Code_Span declare_keyword; },        \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript 'declare abstract class' is not "   \
                                "allowed in JavaScript"),                       \
              declare_keyword))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Class_Not_Allowed_In_JavaScript, "E0339",                    \
      Diagnostic_Severity::error, { Source_Code_Span declare_keyword; },        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript 'declare class' is not allowed in JavaScript"),   \
              declare_keyword))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Function_Cannot_Be_Async, "E0354",                           \
      Diagnostic_Severity::error, { Source_Code_Span async_keyword; },          \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("'declare function' cannot be marked 'async'"),     \
          async_keyword))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Function_Cannot_Be_Generator, "E0355",                       \
      Diagnostic_Severity::error, { Source_Code_Span star; },                   \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'declare function' cannot be marked as a generator"),        \
              star))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Function_Cannot_Have_Body, "E0353",                          \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span body_start;                                            \
        Source_Code_Span declare_keyword;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'declare function' cannot have a body"),       \
              body_start)                                                       \
          MESSAGE(QLJS_TRANSLATABLE("'declare function' here"),                 \
                  declare_keyword))                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Function_Not_Allowed_In_JavaScript, "E0352",                 \
      Diagnostic_Severity::error, { Source_Code_Span declare_keyword; },        \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript 'declare function' is not "         \
                                "allowed in JavaScript"),                       \
              declare_keyword))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Keyword_Is_Not_Allowed_Inside_Declare_Namespace, "E0358",    \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span declare_keyword;                                       \
        Source_Code_Span declare_namespace_declare_keyword;                     \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "'declare' should not be written inside a 'declare namespace'"),  \
          declare_keyword)                                                      \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("containing 'declare namespace' starts here"),  \
              declare_namespace_declare_keyword))                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Namespace_Cannot_Contain_Statement, "E0357",                 \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span first_statement_token;                                 \
        Source_Code_Span declare_keyword;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'declare namespace' cannot contain "           \
                                "statements, only declarations"),               \
              first_statement_token)                                            \
          MESSAGE(QLJS_TRANSLATABLE("'declare' here"), declare_keyword))        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Namespace_Cannot_Import_Module, "E0362",                     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        /* importing_keyword is either 'import', 'from', or 'require'. */       \
        Source_Code_Span importing_keyword;                                     \
        Source_Code_Span declare_keyword;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "cannot import a module from inside a 'declare namespace'"),  \
              importing_keyword)                                                \
          MESSAGE(QLJS_TRANSLATABLE("'declare namespace' starts here"),         \
                  declare_keyword))                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Var_Cannot_Have_Initializer, "E0351",                        \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span equal;                                                 \
        Source_Code_Span declare_keyword;                                       \
        Source_Code_Span declaring_token;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'declare {1}' cannot have initializer"),       \
              equal, declaring_token)                                           \
          MESSAGE(QLJS_TRANSLATABLE("'declare {1}' started here"),              \
                  declare_keyword, declaring_token))                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Declare_Var_Not_Allowed_In_JavaScript, "E0350",                      \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span declare_keyword;                                       \
        Source_Code_Span declaring_token;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript 'declare {1}' is not "              \
                                "allowed in JavaScript"),                       \
              declare_keyword, declaring_token))                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Function_Async_Function, "E0327", Diagnostic_Severity::error,        \
      { Source_Code_Span function_async; },                                     \
      MESSAGE(QLJS_TRANSLATABLE("'function async' is not allowed; write "       \
                                "'async function' instead"),                    \
              function_async))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Big_Int_Literal_Contains_Decimal_Point, "E0005",                     \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("BigInt literal contains decimal point"),       \
              where))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Big_Int_Literal_Contains_Exponent, "E0006",                          \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("BigInt literal contains exponent"), where))    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_C_Style_For_Loop_Is_Missing_Third_Component, "E0093",                \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span expected_last_component;                               \
        Source_Code_Span existing_semicolon;                                    \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "C-style for loop is missing its third component"),           \
              expected_last_component))                                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Assign_To_Loop_Variable_In_For_Of_Or_In_Loop, "E0173",        \
      Diagnostic_Severity::error, { Source_Code_Span equal_token; },            \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "cannot assign to loop variable in for of/in loop"),          \
              equal_token))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Access_Private_Identifier_Outside_Class, "E0208",             \
      Diagnostic_Severity::error, { Source_Code_Span private_identifier; },     \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("cannot access private identifier outside class"),  \
          private_identifier))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Assign_To_Variable_Named_Async_In_For_Of_Loop, "E0082",       \
      Diagnostic_Severity::error, { Source_Code_Span async_identifier; },       \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "assigning to 'async' in a for-of loop requires parentheses"),    \
          async_identifier))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Declare_Await_In_Async_Function, "E0069",                     \
      Diagnostic_Severity::error, { Source_Code_Span name; },                   \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("cannot declare 'await' inside async function"),    \
          name))                                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Declare_Class_Named_Let, "E0007",                             \
      Diagnostic_Severity::error, { Source_Code_Span name; },                   \
      MESSAGE(QLJS_TRANSLATABLE("classes cannot be named 'let'"), name))        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Declare_Variable_Named_Let_With_Let, "E0008",                 \
      Diagnostic_Severity::error, { Source_Code_Span name; },                   \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "let statement cannot declare variables named 'let'"),        \
              name))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Declare_Variable_With_Keyword_Name, "E0124",                  \
      Diagnostic_Severity::error, { Source_Code_Span keyword; },                \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("cannot declare variable named keyword '{0}'"),     \
          keyword))                                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Declare_Yield_In_Generator_Function, "E0071",                 \
      Diagnostic_Severity::error, { Source_Code_Span name; },                   \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "cannot declare 'yield' inside generator function"),          \
              name))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Export_Default_Variable, "E0076",                             \
      Diagnostic_Severity::error, { Source_Code_Span declaring_token; },        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "cannot declare and export variable with 'export default'"),  \
              declaring_token))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Export_Let, "E0009", Diagnostic_Severity::error,              \
      { Source_Code_Span export_name; },                                        \
      MESSAGE(QLJS_TRANSLATABLE("cannot export variable named 'let'"),          \
              export_name))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Export_Variable_Named_Keyword, "E0144",                       \
      Diagnostic_Severity::error, { Source_Code_Span export_name; },            \
      MESSAGE(QLJS_TRANSLATABLE("cannot export variable named keyword '{0}'"),  \
              export_name))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Import_Let, "E0010", Diagnostic_Severity::error,              \
      { Source_Code_Span import_name; },                                        \
      MESSAGE(QLJS_TRANSLATABLE("cannot import 'let'"), import_name))           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Import_Variable_Named_Keyword, "E0145",                       \
      Diagnostic_Severity::error, { Source_Code_Span import_name; },            \
      MESSAGE(QLJS_TRANSLATABLE("cannot import variable named keyword '{0}'"),  \
              import_name))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Import_From_Unquoted_Module, "E0235",                         \
      Diagnostic_Severity::error, { Source_Code_Span import_name; },            \
      MESSAGE(QLJS_TRANSLATABLE("missing quotes around module name '{0}'"),     \
              import_name))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Refer_To_Private_Variable_Without_Object, "E0155",            \
      Diagnostic_Severity::error, { Source_Code_Span private_identifier; },     \
      MESSAGE(QLJS_TRANSLATABLE("cannot reference private variables without "   \
                                "object; use 'this.'"),                         \
              private_identifier))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Cannot_Update_Variable_During_Declaration, "E0136",                  \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span declaring_token;                                       \
        Source_Code_Span updating_operator;                                     \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "cannot update variable with '{0}' while declaring it"),      \
              updating_operator)                                                \
          MESSAGE(QLJS_TRANSLATABLE(                                            \
                      "remove '{0}' to update an existing variable"),           \
                  declaring_token))                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Catch_Without_Try, "E0117", Diagnostic_Severity::error,              \
      { Source_Code_Span catch_token; },                                        \
      MESSAGE(QLJS_TRANSLATABLE("unexpected 'catch' without 'try'"),            \
              catch_token))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Class_Statement_Not_Allowed_In_Body, "E0149",                        \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Statement_Kind kind_of_statement;                                       \
        Source_Code_Span expected_body;                                         \
        Source_Code_Span class_keyword;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing body for {1:headlinese}"),             \
              expected_body, kind_of_statement)                                 \
          MESSAGE(QLJS_TRANSLATABLE("a class statement is not allowed as the "  \
                                    "body of {1:singular}"),                    \
                  class_keyword, kind_of_statement))                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Character_Disallowed_In_Identifiers, "E0011",                        \
      Diagnostic_Severity::error, { Source_Code_Span character; },              \
      MESSAGE(QLJS_TRANSLATABLE("character is not allowed in identifiers"),     \
              character))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Comma_Not_Allowed_After_Spread_Parameter, "E0070",                   \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span comma;                                                 \
        Source_Code_Span spread;                                                \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("commas are not allowed after spread parameter"),   \
          comma))                                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Comma_Not_Allowed_Before_First_Generic_Parameter, "E0262",           \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_comma; },       \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "leading commas are not allowed in generic parameter lists"),     \
          unexpected_comma))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Comma_Not_Allowed_Between_Class_Methods, "E0209",                    \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_comma; },       \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("commas are not allowed between class methods"),    \
          unexpected_comma))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Config_Json_Syntax_Error, "E0164", Diagnostic_Severity::error,       \
      { Source_Code_Span where; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("JSON syntax error"), where))                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Config_Global_Groups_Group_Type_Mismatch, "E0170",                   \
      Diagnostic_Severity::error, { Source_Code_Span group; },                  \
      MESSAGE(QLJS_TRANSLATABLE("\"global-groups\" entries must be strings"),   \
              group))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Config_Global_Groups_Type_Mismatch, "E0169",                         \
      Diagnostic_Severity::error, { Source_Code_Span value; },                  \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "\"global-groups\" must be a boolean or an array"),           \
              value))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Config_Globals_Descriptor_Type_Mismatch, "E0171",                    \
      Diagnostic_Severity::error, { Source_Code_Span descriptor; },             \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "\"globals\" descriptor must be a boolean or an object"),     \
              descriptor))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Config_Globals_Descriptor_Shadowable_Type_Mismatch, "E0166",         \
      Diagnostic_Severity::error, { Source_Code_Span value; },                  \
      MESSAGE(QLJS_TRANSLATABLE("\"globals\" descriptor \"shadowable\" "        \
                                "property must be a boolean"),                  \
              value))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Config_Globals_Descriptor_Writable_Type_Mismatch, "E0167",           \
      Diagnostic_Severity::error, { Source_Code_Span value; },                  \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("\"globals\" descriptor \"writable\" property "     \
                            "must be a boolean"),                               \
          value))                                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Config_Globals_Type_Mismatch, "E0168", Diagnostic_Severity::error,   \
      { Source_Code_Span value; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("\"globals\" must be an object"), value))       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Depth_Limit_Exceeded, "E0203", Diagnostic_Severity::error,           \
      { Source_Code_Span token; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("depth limit exceeded"), token))                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Dot_Not_Allowed_After_Generic_Arguments_In_Type, "E0259",            \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span dot;                                                   \
        Source_Code_Span property_name;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'.' is not allowed after generic arguments; "  \
                                "write [\"{1}\"] instead"),                     \
              dot, property_name))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Dot_Dot_Is_Not_An_Operator, "E0053", Diagnostic_Severity::error,     \
      { Source_Code_Span dots; },                                               \
      MESSAGE(QLJS_TRANSLATABLE("missing property name between '.' and '.'"),   \
              dots))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Duplicated_Cases_In_Switch_Statement, "E0347",                       \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span first_switch_case;                                     \
        Source_Code_Span duplicated_switch_case;                                \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("duplicated case clause in switch statement"),  \
              duplicated_switch_case)                                           \
          MESSAGE(QLJS_TRANSLATABLE("this case will run instead"),              \
                  first_switch_case))                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Else_Has_No_If, "E0065", Diagnostic_Severity::error,                 \
      { Source_Code_Span else_token; },                                         \
      MESSAGE(QLJS_TRANSLATABLE("'else' has no corresponding 'if'"),            \
              else_token))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Equals_Does_Not_Distribute_Over_Or, "E0190",                         \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span or_operator;                                           \
        Source_Code_Span equals_operator;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing comparison; '{1}' does not extend "    \
                                "to the right side of '{0}'"),                  \
              or_operator, equals_operator)                                     \
          MESSAGE(QLJS_TRANSLATABLE("'{0}' found here"), equals_operator))      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Escaped_Character_Disallowed_In_Identifiers, "E0012",                \
      Diagnostic_Severity::error, { Source_Code_Span escape_sequence; },        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "escaped character is not allowed in identifiers"),           \
              escape_sequence))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Escaped_Code_Point_In_Identifier_Out_Of_Range, "E0013",              \
      Diagnostic_Severity::error, { Source_Code_Span escape_sequence; },        \
      MESSAGE(QLJS_TRANSLATABLE("code point out of range"), escape_sequence))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Escaped_Code_Point_In_Unicode_Out_Of_Range, "E0207",                 \
      Diagnostic_Severity::error, { Source_Code_Span escape_sequence; },        \
      MESSAGE(QLJS_TRANSLATABLE("code point in Unicode escape sequence must "   \
                                "not be greater than U+10FFFF"),                \
              escape_sequence))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Escaped_Hyphen_Not_Allowed_In_JSX_Tag, "E0019",                      \
      Diagnostic_Severity::error, { Source_Code_Span escape_sequence; },        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "escaping '-' is not allowed in tag names; write '-' instead"),   \
          escape_sequence))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Extra_Comma_Not_Allowed_Between_Arguments, "E0068",                  \
      Diagnostic_Severity::error, { Source_Code_Span comma; },                  \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "extra ',' is not allowed between function call arguments"),  \
              comma))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Extra_Comma_Not_Allowed_Between_Enum_Members, "E0248",               \
      Diagnostic_Severity::error, { Source_Code_Span comma; },                  \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("extra ',' is not allowed between enum members"),   \
          comma))                                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Misleading_Comma_Operator_In_Index_Operation, "E0450",               \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span comma;                                                 \
        Source_Code_Span left_square;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("misleading use of ',' operator in index"),     \
              comma)                                                            \
          MESSAGE(QLJS_TRANSLATABLE("index starts here"), left_square))         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Misleading_Comma_Operator_In_Conditional_Statement, "E0451",         \
      Diagnostic_Severity::warning, { Source_Code_Span comma; },                \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "misleading use of ',' operator in conditional statement"),   \
              comma))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Empty_Paren_After_Control_Statement, "E0452",                        \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span token;                                                 \
        Source_Code_Span expected_expression;                                   \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("expected expression after '('"),               \
              expected_expression)                                              \
          MESSAGE(QLJS_TRANSLATABLE("'{1}' statement starts here"), token,      \
                  token))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_As_Before_Imported_Namespace_Alias, "E0126",                \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span star_through_alias_token;                              \
        Source_Code_Span alias;                                                 \
        Source_Code_Span star_token;                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("expected 'as' between '{1}' and '{2}'"),       \
              star_through_alias_token, star_token, alias))                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Comma_To_Separate_Object_Literal_Entries, "E0131",          \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_token; },       \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("expected ',' between object literal entries"),     \
          unexpected_token))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Expression_Before_Newline, "E0014",                         \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("expected expression before newline"), where))  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Expression_For_Switch_Case, "E0140",                        \
      Diagnostic_Severity::error, { Source_Code_Span case_token; },             \
      MESSAGE(QLJS_TRANSLATABLE("expected expression after 'case'"),            \
              case_token))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Expression_Before_Semicolon, "E0015",                       \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("expected expression before semicolon"),        \
              where))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_From_And_Module_Specifier, "E0129",                         \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("expected 'from \"name_of_module.mjs\"'"),      \
              where))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_From_Before_Module_Specifier, "E0128",                      \
      Diagnostic_Severity::error, { Source_Code_Span module_specifier; },       \
      MESSAGE(QLJS_TRANSLATABLE("expected 'from' before module specifier"),     \
              module_specifier))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Hex_Digits_In_Unicode_Escape, "E0016",                      \
      Diagnostic_Severity::error, { Source_Code_Span escape_sequence; },        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "expected hexadecimal digits in Unicode escape sequence"),    \
              escape_sequence))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Left_Curly, "E0107", Diagnostic_Severity::error,            \
      { Source_Code_Span expected_left_curly; },                                \
      MESSAGE(QLJS_TRANSLATABLE("expected '{{'"), expected_left_curly))         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Right_Paren_For_Function_Call, "E0141",                     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span expected_right_paren;                                  \
        Source_Code_Span left_paren;                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("expected ')' to close function call"),         \
              expected_right_paren)                                             \
          MESSAGE(QLJS_TRANSLATABLE("function call started here"),              \
                  left_paren))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parentheses_Around_Do_While_Condition, "E0084",             \
      Diagnostic_Severity::error, { Source_Code_Span condition; },              \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "do-while loop needs parentheses around condition"),          \
              condition))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parenthesis_Around_Do_While_Condition, "E0085",             \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span where;                                                 \
        Char8 token;                                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "do-while loop is missing '{1}' around condition"),           \
              where, token))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parentheses_Around_If_Condition, "E0017",                   \
      Diagnostic_Severity::error, { Source_Code_Span condition; },              \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "if statement needs parentheses around condition"),           \
              condition))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parenthesis_Around_If_Condition, "E0018",                   \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span where;                                                 \
        Char8 token;                                                            \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("if statement is missing '{1}' around condition"),  \
          where, token))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parentheses_Around_Switch_Condition, "E0091",               \
      Diagnostic_Severity::error, { Source_Code_Span condition; },              \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "switch statement needs parentheses around condition"),       \
              condition))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parenthesis_Around_Switch_Condition, "E0092",               \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span where;                                                 \
        Char8 token;                                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "switch statement is missing '{1}' around condition"),        \
              where, token))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parentheses_Around_While_Condition, "E0087",                \
      Diagnostic_Severity::error, { Source_Code_Span condition; },              \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("while loop needs parentheses around condition"),   \
          condition))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parenthesis_Around_While_Condition, "E0088",                \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span where;                                                 \
        Char8 token;                                                            \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("while loop is missing '{1}' around condition"),    \
          where, token))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parentheses_Around_With_Expression, "E0089",                \
      Diagnostic_Severity::error, { Source_Code_Span expression; },             \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "with statement needs parentheses around expression"),        \
              expression))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Parenthesis_Around_With_Expression, "E0090",                \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span where;                                                 \
        Char8 token;                                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "with statement is missing '{1}' around expression"),         \
              where, token))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Variable_Name_For_Catch, "E0135",                           \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_token; },       \
      MESSAGE(QLJS_TRANSLATABLE("expected variable name for 'catch'"),          \
              unexpected_token))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Expected_Variable_Name_For_Import_As, "E0175",                       \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_token; },       \
      MESSAGE(QLJS_TRANSLATABLE("expected variable name for 'import'-'as'"),    \
              unexpected_token))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Exporting_Requires_Default, "E0067", Diagnostic_Severity::error,     \
      { Source_Code_Span expression; },                                         \
      MESSAGE(QLJS_TRANSLATABLE("exporting requires 'default'"), expression))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Exporting_Requires_Curlies, "E0066", Diagnostic_Severity::error,     \
      { Source_Code_Span names; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("exporting requires '{{' and '}'"), names))     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Exporting_String_Name_Only_Allowed_For_Export_From, "E0153",         \
      Diagnostic_Severity::error, { Source_Code_Span export_name; },            \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "forwarding exports are only allowed in export-from"),        \
              export_name))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Finally_Without_Try, "E0118", Diagnostic_Severity::error,            \
      { Source_Code_Span finally_token; },                                      \
      MESSAGE(QLJS_TRANSLATABLE("unexpected 'finally' without 'try'"),          \
              finally_token))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Function_Statement_Not_Allowed_In_Body, "E0148",                     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Statement_Kind kind_of_statement;                                       \
        Source_Code_Span expected_body;                                         \
        Source_Code_Span function_keywords;                                     \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing body for {1:headlinese}"),             \
              expected_body, kind_of_statement)                                 \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("a function statement is not allowed as the "   \
                                "body of {1:singular}"),                        \
              function_keywords, kind_of_statement))                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Generator_Function_Star_Belongs_After_Keyword_Function, "E0204",     \
      Diagnostic_Severity::error, { Source_Code_Span star; },                   \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "generator function '*' belongs after keyword function"),     \
              star))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Generator_Function_Star_Belongs_Before_Name, "E0133",                \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span function_name;                                         \
        Source_Code_Span star;                                                  \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "generator function '*' belongs before function name"),       \
              star))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Multiple_Commas_In_Generic_Parameter_List, "E0263",                  \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_comma; },       \
      MESSAGE(QLJS_TRANSLATABLE("only one comma is allowed between or after "   \
                                "generic parameters"),                          \
              unexpected_comma))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_In_Disallowed_In_C_Style_For_Loop, "E0108",                          \
      Diagnostic_Severity::error, { Source_Code_Span in_token; },               \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'in' disallowed in C-style for loop initializer"),           \
              in_token))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Indexing_Requires_Expression, "E0075", Diagnostic_Severity::error,   \
      { Source_Code_Span squares; },                                            \
      MESSAGE(QLJS_TRANSLATABLE("indexing requires an expression"), squares))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Expression_Left_Of_Assignment, "E0020",                      \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("invalid expression left of assignment"),       \
              where))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Hex_Escape_Sequence, "E0060", Diagnostic_Severity::error,    \
      { Source_Code_Span escape_sequence; },                                    \
      MESSAGE(QLJS_TRANSLATABLE("invalid hex escape sequence: {0}"),            \
              escape_sequence))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Lone_Literal_In_Object_Literal, "E0021",                     \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("invalid lone literal in object literal"),      \
              where))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Parameter, "E0151", Diagnostic_Severity::error,              \
      { Source_Code_Span parameter; },                                          \
      MESSAGE(QLJS_TRANSLATABLE("invalid function parameter"), parameter))      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Quotes_Around_String_Literal, "E0197",                       \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span opening_quote;                                         \
        Char8 suggested_quote;                                                  \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'{0}' is not allowed for strings; use {1} instead"),         \
              opening_quote, suggested_quote))                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Rhs_For_Dot_Operator, "E0074", Diagnostic_Severity::error,   \
      { Source_Code_Span dot; },                                                \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'.' operator needs a key name; use + to concatenate "        \
                  "strings; use [] to access with a dynamic key"),              \
              dot))                                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Utf_8_Sequence, "E0022", Diagnostic_Severity::error,         \
      { Source_Code_Span sequence; },                                           \
      MESSAGE(QLJS_TRANSLATABLE("invalid UTF-8 sequence"), sequence))           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_JSX_Attribute_Has_Wrong_Capitalization, "E0192",                     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span attribute_name;                                        \
        String8_View expected_attribute_name;                                   \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "attribute has wrong capitalization; write '{1}' instead"),   \
              attribute_name, expected_attribute_name))                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_JSX_Attribute_Renamed_By_React, "E0193",                             \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span attribute_name;                                        \
        String8_View react_attribute_name;                                      \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "misspelled React attribute; write '{1}' instead"),           \
              attribute_name, react_attribute_name))                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_JSX_Event_Attribute_Should_Be_Camel_Case, "E0191",                   \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span attribute_name;                                        \
        String8_View expected_attribute_name;                                   \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("event attributes must be camelCase: '{1}'"),   \
              attribute_name, expected_attribute_name))                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_JSX_Not_Allowed_In_JavaScript, "E0177", Diagnostic_Severity::error,  \
      { Source_Code_Span jsx_start; },                                          \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "React/JSX is not allowed in vanilla JavaScript code"),       \
              jsx_start))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_JSX_Not_Allowed_In_TypeScript, "E0306", Diagnostic_Severity::error,  \
      { Source_Code_Span jsx_start; },                                          \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("React/JSX is not allowed in TypeScript code"),     \
          jsx_start))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Keywords_Cannot_Contain_Escape_Sequences, "E0023",                   \
      Diagnostic_Severity::error, { Source_Code_Span escape_sequence; },        \
      MESSAGE(QLJS_TRANSLATABLE("keywords cannot contain escape sequences"),    \
              escape_sequence))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Label_Named_Await_Not_Allowed_In_Async_Function, "E0206",            \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span await;                                                 \
        Source_Code_Span colon;                                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "label named 'await' not allowed in async function"),         \
              await))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Legacy_Octal_Literal_May_Not_Be_Big_Int, "E0032",                    \
      Diagnostic_Severity::error, { Source_Code_Span characters; },             \
      MESSAGE(QLJS_TRANSLATABLE("legacy octal literal may not be BigInt"),      \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Legacy_Octal_Literal_May_Not_Contain_Underscores, "E0152",           \
      Diagnostic_Severity::error, { Source_Code_Span underscores; },            \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "legacy octal literals may not contain underscores"),         \
              underscores))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Let_With_No_Bindings, "E0024", Diagnostic_Severity::error,           \
      { Source_Code_Span where; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("{0} with no bindings"), where))                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Lexical_Declaration_Not_Allowed_In_Body, "E0150",                    \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Statement_Kind kind_of_statement;                                       \
        Source_Code_Span expected_body;                                         \
        Source_Code_Span declaring_keyword;                                     \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing body for {1:headlinese}"),             \
              expected_body, kind_of_statement)                                 \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("a lexical declaration is not allowed as the "  \
                                "body of {1:singular}"),                        \
              declaring_keyword, kind_of_statement))                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Functions_Or_Methods_Should_Not_Have_Arrow_Operator, "E0174",        \
      Diagnostic_Severity::error, { Source_Code_Span arrow_operator; },         \
      MESSAGE(QLJS_TRANSLATABLE("functions/methods should not have '=>'"),      \
              arrow_operator))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Methods_Should_Not_Use_Function_Keyword, "E0072",                    \
      Diagnostic_Severity::error, { Source_Code_Span function_token; },         \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("methods should not use the 'function' keyword"),   \
          function_token))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Mismatched_JSX_Tags, "E0187", Diagnostic_Severity::error,            \
      {                                                                         \
        Source_Code_Span opening_tag_name;                                      \
        Source_Code_Span closing_tag_name;                                      \
        String8_View opening_tag_name_pretty;                                   \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("mismatched JSX tags; expected '</{1}>'"),      \
              closing_tag_name, opening_tag_name_pretty)                        \
          MESSAGE(QLJS_TRANSLATABLE("opening '<{1}>' tag here"),                \
                  opening_tag_name, opening_tag_name_pretty))                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Array_Close, "E0157", Diagnostic_Severity::error,            \
      {                                                                         \
        Source_Code_Span left_square;                                           \
        Source_Code_Span expected_right_square;                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing end of array; expected ']'"),          \
              expected_right_square)                                            \
          MESSAGE(QLJS_TRANSLATABLE("array started here"), left_square))        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Arrow_Operator_In_Arrow_Function, "E0176",                   \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("missing arrow operator for arrow function"),   \
              where))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Arrow_Function_Parameter_List, "E0105",                      \
      Diagnostic_Severity::error, { Source_Code_Span arrow; },                  \
      MESSAGE(QLJS_TRANSLATABLE("missing parameters for arrow function"),       \
              arrow))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_Catch_Clause, "E0119", Diagnostic_Severity::error,  \
      { Source_Code_Span catch_token; },                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing body for catch clause"),               \
              catch_token))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_Class, "E0111", Diagnostic_Severity::error,         \
      { Source_Code_Span class_keyword_and_name_and_heritage; },                \
      MESSAGE(QLJS_TRANSLATABLE("missing body for class"),                      \
              class_keyword_and_name_and_heritage))                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_Do_While_Statement, "E0101",                        \
      Diagnostic_Severity::error, { Source_Code_Span do_token; },               \
      MESSAGE(QLJS_TRANSLATABLE("missing body for do-while loop"), do_token))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_Finally_Clause, "E0121",                            \
      Diagnostic_Severity::error, { Source_Code_Span finally_token; },          \
      MESSAGE(QLJS_TRANSLATABLE("missing body for finally clause"),             \
              finally_token))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_For_Statement, "E0094",                             \
      Diagnostic_Severity::error, { Source_Code_Span for_and_header; },         \
      MESSAGE(QLJS_TRANSLATABLE("missing body for 'for' loop"),                 \
              for_and_header))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_If_Statement, "E0064", Diagnostic_Severity::error,  \
      { Source_Code_Span expected_body; },                                      \
      MESSAGE(QLJS_TRANSLATABLE("missing body for 'if' statement"),             \
              expected_body))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_Switch_Statement, "E0106",                          \
      Diagnostic_Severity::error, { Source_Code_Span switch_and_condition; },   \
      MESSAGE(QLJS_TRANSLATABLE("missing body for 'switch' statement"),         \
              switch_and_condition))                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_Try_Statement, "E0120",                             \
      Diagnostic_Severity::error, { Source_Code_Span try_token; },              \
      MESSAGE(QLJS_TRANSLATABLE("missing body for try statement"), try_token))  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_TypeScript_Interface, "E0245",                      \
      Diagnostic_Severity::error,                                               \
      { Source_Code_Span interface_keyword_and_name_and_heritage; },            \
      MESSAGE(QLJS_TRANSLATABLE("missing body for TypeScript interface"),       \
              interface_keyword_and_name_and_heritage))                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_TypeScript_Namespace, "E0356",                      \
      Diagnostic_Severity::error, { Source_Code_Span expected_body; },          \
      MESSAGE(QLJS_TRANSLATABLE("missing body for TypeScript namespace"),       \
              expected_body))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Body_For_While_Statement, "E0104",                           \
      Diagnostic_Severity::error, { Source_Code_Span while_and_condition; },    \
      MESSAGE(QLJS_TRANSLATABLE("missing body for while loop"),                 \
              while_and_condition))                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Catch_Or_Finally_For_Try_Statement, "E0122",                 \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span expected_catch_or_finally;                             \
        Source_Code_Span try_token;                                             \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "missing catch or finally clause for try statement"),         \
              expected_catch_or_finally)                                        \
          MESSAGE(QLJS_TRANSLATABLE("try statement starts here"), try_token))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Catch_Variable_Between_Parentheses, "E0130",                 \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span left_paren_to_right_paren;                             \
        Source_Code_Span left_paren;                                            \
        Source_Code_Span right_paren;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "missing catch variable name between parentheses"),           \
              left_paren_to_right_paren))                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Comma_Between_Object_Literal_Entries, "E0025",               \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("missing comma between object literal entries"),    \
          where))                                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Comma_Between_Generic_Parameters, "E0265",                   \
      Diagnostic_Severity::error, { Source_Code_Span expected_comma; },         \
      MESSAGE(QLJS_TRANSLATABLE("missing comma between generic parameters"),    \
              expected_comma))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Comma_Between_Variable_Declarations, "E0132",                \
      Diagnostic_Severity::error, { Source_Code_Span expected_comma; },         \
      MESSAGE(QLJS_TRANSLATABLE("missing ',' between variable declarations"),   \
              expected_comma))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Colon_In_Conditional_Expression, "E0146",                    \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span expected_colon;                                        \
        Source_Code_Span question;                                              \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing ':' in conditional expression"),       \
              expected_colon)                                                   \
          MESSAGE(QLJS_TRANSLATABLE("'?' creates a conditional expression"),    \
                  question))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Condition_For_If_Statement, "E0138",                         \
      Diagnostic_Severity::error, { Source_Code_Span if_keyword; },             \
      MESSAGE(QLJS_TRANSLATABLE("missing condition for if statement"),          \
              if_keyword))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Condition_For_While_Statement, "E0139",                      \
      Diagnostic_Severity::error, { Source_Code_Span while_keyword; },          \
      MESSAGE(QLJS_TRANSLATABLE("missing condition for while statement"),       \
              while_keyword))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Condition_For_Switch_Statement, "E0137",                     \
      Diagnostic_Severity::error, { Source_Code_Span switch_keyword; },         \
      MESSAGE(QLJS_TRANSLATABLE("missing condition for switch statement"),      \
              switch_keyword))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Dots_For_Attribute_Spread, "E0186",                          \
      Diagnostic_Severity::error, { Source_Code_Span expected_dots; },          \
      MESSAGE(QLJS_TRANSLATABLE("missing '...' in JSX attribute spread"),       \
              expected_dots))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Equal_After_Variable, "E0202", Diagnostic_Severity::error,   \
      { Source_Code_Span expected_equal; },                                     \
      MESSAGE(QLJS_TRANSLATABLE("missing '=' after variable"),                  \
              expected_equal))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Expression_Between_Parentheses, "E0078",                     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span left_paren_to_right_paren;                             \
        Source_Code_Span left_paren;                                            \
        Source_Code_Span right_paren;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing expression between parentheses"),      \
              left_paren_to_right_paren))                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_For_Loop_Header, "E0125", Diagnostic_Severity::error,        \
      { Source_Code_Span for_token; },                                          \
      MESSAGE(QLJS_TRANSLATABLE("missing header and body for 'for' loop"),      \
              for_token))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_For_Loop_Rhs_Or_Components_After_Expression, "E0097",        \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span header;                                                \
        Source_Code_Span for_token;                                             \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "for loop needs an iterable, or condition and update clauses"),   \
          header)                                                               \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE(                                                \
                  "use 'while' instead to loop until a condition is false"),    \
              for_token))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_For_Loop_Rhs_Or_Components_After_Declaration, "E0098",       \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span header;                                                \
        Source_Code_Span for_token;                                             \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "for loop needs an iterable, or condition and update clauses"),   \
          header))                                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Function_Parameter_List, "E0073",                            \
      Diagnostic_Severity::error,                                               \
      { Source_Code_Span expected_parameter_list; },                            \
      MESSAGE(QLJS_TRANSLATABLE("missing function parameter list"),             \
              expected_parameter_list))                                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Function_Body, "E0172", Diagnostic_Severity::error,          \
      { Source_Code_Span expected_body; },                                      \
      MESSAGE(QLJS_TRANSLATABLE("missing body for function"), expected_body))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Header_Of_For_Loop, "E0096", Diagnostic_Severity::error,     \
      { Source_Code_Span where; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("missing for loop header"), where))             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Initializer_In_Const_Declaration, "E0205",                   \
      Diagnostic_Severity::error, { Source_Code_Span variable_name; },          \
      MESSAGE(QLJS_TRANSLATABLE("missing initializer in const declaration"),    \
              variable_name))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Key_For_Object_Entry, "E0154", Diagnostic_Severity::error,   \
      { Source_Code_Span expression; },                                         \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "unexpected expression; missing key for object entry"),       \
              expression))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Class_Method_Name, "E0229", Diagnostic_Severity::error,      \
      { Source_Code_Span expected_name; },                                      \
      MESSAGE(QLJS_TRANSLATABLE("missing name for class method"),               \
              expected_name))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Name_In_Function_Statement, "E0061",                         \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("missing name in function statement"), where))  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Name_In_Class_Statement, "E0080",                            \
      Diagnostic_Severity::error, { Source_Code_Span class_keyword; },          \
      MESSAGE(QLJS_TRANSLATABLE("missing name of class"), class_keyword))       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Name_Of_Exported_Class, "E0081",                             \
      Diagnostic_Severity::error, { Source_Code_Span class_keyword; },          \
      MESSAGE(QLJS_TRANSLATABLE("missing name of exported class"),              \
              class_keyword))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Name_Of_Exported_Function, "E0079",                          \
      Diagnostic_Severity::error, { Source_Code_Span function_keyword; },       \
      MESSAGE(QLJS_TRANSLATABLE("missing name of exported function"),           \
              function_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Name_Or_Parentheses_For_Function, "E0062",                   \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span where;                                                 \
        Source_Code_Span function;                                              \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing name or parentheses for function"),    \
              where))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Operand_For_Operator, "E0026", Diagnostic_Severity::error,   \
      { Source_Code_Span where; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("missing operand for operator"), where))        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Separator_Between_Object_Type_Entries, "E0257",              \
      Diagnostic_Severity::error, { Source_Code_Span expected_separator; },     \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "missing ',', ';', or newline between object type entries"),  \
              expected_separator))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Redundant_Delete_Statement_On_Variable, "E0086",                     \
      Diagnostic_Severity::warning, { Source_Code_Span delete_expression; },    \
      MESSAGE(QLJS_TRANSLATABLE("redundant delete statement on variable"),      \
              delete_expression))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_If_After_Else, "E0184", Diagnostic_Severity::error,          \
      { Source_Code_Span expected_if; },                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing 'if' after 'else'"), expected_if))     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Operator_Between_Expression_And_Arrow_Function, "E0063",     \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "missing operator between expression and arrow function"),    \
              where))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Parentheses_Around_Exponent_With_Unary_Lhs, "E0195",         \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span exponent_expression;                                   \
        Source_Code_Span unary_operator;                                        \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("missing parentheses around operand of '{0}'"),     \
          exponent_expression)                                                  \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("'{0}' operator cannot be used before '**' "    \
                                "without parentheses"),                         \
              unary_operator))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Parentheses_Around_Self_Invoked_Function, "E0211",           \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span invocation;                                            \
        Source_Code_Span func_start;                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "missing parentheses around self-invoked function"),          \
              invocation)                                                       \
          MESSAGE(QLJS_TRANSLATABLE("function starts here"), func_start))       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Parentheses_Around_Unary_Lhs_Of_Exponent, "E0194",           \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span unary_expression;                                      \
        Source_Code_Span exponent_operator;                                     \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "missing parentheses around left-hand side of '**'"),         \
              unary_expression)                                                 \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("'**' operator cannot be used after unary "     \
                                "'{1}' without parentheses"),                   \
              exponent_operator, unary_expression))                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Property_Name_For_Dot_Operator, "E0142",                     \
      Diagnostic_Severity::error, { Source_Code_Span dot; },                    \
      MESSAGE(QLJS_TRANSLATABLE("missing property name after '.' operator"),    \
              dot))                                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Semicolon_After_Abstract_Method, "E0293",                    \
      Diagnostic_Severity::error, { Source_Code_Span expected_semicolon; },     \
      MESSAGE(QLJS_TRANSLATABLE("missing semicolon after abstract method"),     \
              expected_semicolon))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Semicolon_After_Declare_Class_Method, "E0334",               \
      Diagnostic_Severity::error, { Source_Code_Span expected_semicolon; },     \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("missing semicolon after 'declare class' method"),  \
          expected_semicolon))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Semicolon_After_Statement, "E0027",                          \
      Diagnostic_Severity::error, { Source_Code_Span where; },                  \
      MESSAGE(QLJS_TRANSLATABLE("missing semicolon after statement"), where))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Semicolon_After_Field, "E0223", Diagnostic_Severity::error,  \
      { Source_Code_Span expected_semicolon; },                                 \
      MESSAGE(QLJS_TRANSLATABLE("missing semicolon after field"),               \
              expected_semicolon))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Semicolon_After_Index_Signature, "E0226",                    \
      Diagnostic_Severity::error, { Source_Code_Span expected_semicolon; },     \
      MESSAGE(QLJS_TRANSLATABLE("missing semicolon after index signature"),     \
              expected_semicolon))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Semicolon_After_Interface_Method, "E0292",                   \
      Diagnostic_Severity::error, { Source_Code_Span expected_semicolon; },     \
      MESSAGE(QLJS_TRANSLATABLE("missing semicolon after interface method"),    \
              expected_semicolon))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Semicolon_Between_For_Loop_Condition_And_Update, "E0100",    \
      Diagnostic_Severity::error, { Source_Code_Span expected_semicolon; },     \
      MESSAGE(QLJS_TRANSLATABLE("missing semicolon between condition and "      \
                                "update parts of for loop"),                    \
              expected_semicolon))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Semicolon_Between_For_Loop_Init_And_Condition, "E0099",      \
      Diagnostic_Severity::error, { Source_Code_Span expected_semicolon; },     \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("missing semicolon between init and condition "     \
                            "parts of for loop"),                               \
          expected_semicolon))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Token_After_Export, "E0113", Diagnostic_Severity::error,     \
      { Source_Code_Span export_token; },                                       \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "incomplete export; expected 'export default ...' or "        \
                  "'export {{name}' or 'export * from ...' or 'export class' "  \
                  "or 'export function' or 'export let'"),                      \
              export_token))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Type_Between_Intersection_Or_Union, "E0258",                 \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span left_operator;                                         \
        Source_Code_Span right_operator;                                        \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing type between '{1}' and '{0}'"),        \
              right_operator, left_operator))                                   \
                                                                                \
  /* TODO(strager): Make more specific errors, like 'missing type after :',     \
     'missing type after keyof', etc. */                                        \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_TypeScript_Type, "E0284", Diagnostic_Severity::error,        \
      { Source_Code_Span expected_type; },                                      \
      MESSAGE(QLJS_TRANSLATABLE("missing TypeScript type"), expected_type))     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Value_For_Object_Literal_Entry, "E0083",                     \
      Diagnostic_Severity::error, { Source_Code_Span key; },                    \
      MESSAGE(QLJS_TRANSLATABLE("missing value for object property"), key))     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_Variable_Name_In_Declaration, "E0123",                       \
      Diagnostic_Severity::error, { Source_Code_Span equal_token; },            \
      MESSAGE(QLJS_TRANSLATABLE("missing variable name"), equal_token))         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Missing_While_And_Condition_For_Do_While_Statement, "E0103",         \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span do_token;                                              \
        Source_Code_Span expected_while;                                        \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "missing 'while (condition)' for do-while statement"),        \
              expected_while)                                                   \
          MESSAGE(QLJS_TRANSLATABLE("do-while statement starts here"),          \
                  do_token))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Newline_Not_Allowed_Between_Async_And_Parameter_List, "E0163",       \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span async;                                                 \
        Source_Code_Span arrow;                                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("newline is not allowed between 'async' and "   \
                                "arrow function parameter list"),               \
              async) MESSAGE(QLJS_TRANSLATABLE("arrow is here"), arrow))        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Newline_Not_Allowed_Between_Async_And_Function_Keyword, "E0317",     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span async_keyword;                                         \
        Source_Code_Span function_keyword;                                      \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("newline is not allowed between 'async' and "   \
                                "'function'"),                                  \
              async_keyword)                                                    \
          MESSAGE(QLJS_TRANSLATABLE("'function' is here"), function_keyword))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Newline_Not_Allowed_After_Abstract_Keyword, "E0300",                 \
      Diagnostic_Severity::error, { Source_Code_Span abstract_keyword; },       \
      MESSAGE(QLJS_TRANSLATABLE("newline is not allowed after 'abstract'"),     \
              abstract_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Newline_Not_Allowed_After_Export_Declare, "E0382",                   \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span declare_keyword;                                       \
        Source_Code_Span export_keyword;                                        \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("newline is not allowed after 'export declare'"),   \
          declare_keyword, export_keyword))                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Newline_Not_Allowed_After_Interface_Keyword, "E0275",                \
      Diagnostic_Severity::error, { Source_Code_Span interface_keyword; },      \
      MESSAGE(QLJS_TRANSLATABLE("newline is not allowed after 'interface'"),    \
              interface_keyword))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Newline_Not_Allowed_After_Namespace_Keyword, "E0276",                \
      Diagnostic_Severity::error, { Source_Code_Span namespace_keyword; },      \
      MESSAGE(QLJS_TRANSLATABLE("newline is not allowed after '{0}'"),          \
              namespace_keyword))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Newline_Not_Allowed_After_Type_Keyword, "E0277",                     \
      Diagnostic_Severity::error, { Source_Code_Span type_keyword; },           \
      MESSAGE(QLJS_TRANSLATABLE("newline is not allowed after 'type'"),         \
              type_keyword))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Newline_Not_Allowed_Before_Assignment_Assertion_Operator, "E0241",   \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span bang;                                                  \
        Source_Code_Span field_name;                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "newline is not allowed between field name and '!'"),         \
              bang)                                                             \
          MESSAGE(QLJS_TRANSLATABLE("field declared here"), field_name))        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Number_Literal_Contains_Consecutive_Underscores, "E0028",            \
      Diagnostic_Severity::error, { Source_Code_Span underscores; },            \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "number literal contains consecutive underscores"),           \
              underscores))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Number_Literal_Contains_Trailing_Underscores, "E0029",               \
      Diagnostic_Severity::error, { Source_Code_Span underscores; },            \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("number literal contains trailing underscore(s)"),  \
          underscores))                                                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Octal_Literal_May_Not_Have_Exponent, "E0030",                        \
      Diagnostic_Severity::error, { Source_Code_Span characters; },             \
      MESSAGE(QLJS_TRANSLATABLE("octal literal may not have exponent"),         \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Octal_Literal_May_Not_Have_Decimal, "E0031",                         \
      Diagnostic_Severity::error, { Source_Code_Span characters; },             \
      MESSAGE(QLJS_TRANSLATABLE("octal literal may not have decimal"),          \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Object_Literal_Default_In_Expression, "E0253",                       \
      Diagnostic_Severity::error, { Source_Code_Span equal; },                  \
      MESSAGE(QLJS_TRANSLATABLE("use ':' instead of '=' in object literals"),   \
              equal))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Optional_Arrow_Parameter_Requires_Parentheses, "E0311",              \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span parameter_and_question;                                \
        Source_Code_Span question;                                              \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing parentheses around parameter"),        \
              parameter_and_question)                                           \
          MESSAGE(QLJS_TRANSLATABLE(                                            \
                      "TypeScript optional parameter requires parentheses"),    \
                  question))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Optional_Arrow_Parameter_With_Type_Annotation_Requires_Parentheses,  \
      "E0312", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span parameter_and_annotation;                              \
        Source_Code_Span question;                                              \
        Source_Code_Span type_colon;                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("missing parentheses around parameter"),        \
              parameter_and_annotation)                                         \
          MESSAGE(QLJS_TRANSLATABLE("TypeScript optional parameter with type "  \
                                    "annotation requires parentheses"),         \
                  question))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Optional_Parameter_Cannot_Have_Initializer, "E0310",                 \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span equal;                                                 \
        Source_Code_Span question;                                              \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("optional parameter cannot have both '?' and "  \
                                "initializer; remove '?'"),                     \
              question)                                                         \
          MESSAGE(QLJS_TRANSLATABLE("initializer starts here"), equal))         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Optional_Parameter_Cannot_Be_Followed_By_Required_Parameter,         \
      "E0379", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span optional_parameter;                                    \
        Source_Code_Span required_parameter;                                    \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("optional parameter cannot be followed by a "   \
                                "required parameter"),                          \
              optional_parameter)                                               \
          MESSAGE(QLJS_TRANSLATABLE("this required parameter appears after "    \
                                    "the optional parameter"),                  \
                  required_parameter))                                          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Integer_Literal_Will_Lose_Precision, "E0212",                        \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span characters;                                            \
        String8_View rounded_val;                                               \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("integer cannot be represented and will be "    \
                                "rounded to '{1}'"),                            \
              characters, rounded_val))                                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Private_Properties_Are_Not_Allowed_In_Object_Literals, "E0156",      \
      Diagnostic_Severity::error, { Source_Code_Span private_identifier; },     \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "private properties are not allowed in object literals"),     \
              private_identifier))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Readonly_Static_Field, "E0232", Diagnostic_Severity::error,          \
      { Source_Code_Span readonly_static; },                                    \
      MESSAGE(QLJS_TRANSLATABLE("'readonly static' is not allowed; write "      \
                                "'static readonly' instead"),                   \
              readonly_static))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Redeclaration_Of_Global_Variable, "E0033",                           \
      Diagnostic_Severity::error, { Source_Code_Span redeclaration; },          \
      MESSAGE(QLJS_TRANSLATABLE("redeclaration of global variable"),            \
              redeclaration))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Redeclaration_Of_Variable, "E0034", Diagnostic_Severity::error,      \
      {                                                                         \
        Source_Code_Span redeclaration;                                         \
        Source_Code_Span original_declaration;                                  \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("redeclaration of variable: {0}"),              \
              redeclaration)                                                    \
          MESSAGE(QLJS_TRANSLATABLE("variable already declared here"),          \
                  original_declaration))                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Redundant_Await, "E0266", Diagnostic_Severity::warning,              \
      { Source_Code_Span await_operator; },                                     \
      MESSAGE(QLJS_TRANSLATABLE("redundant 'await'"), await_operator))          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Regexp_Literal_Flags_Cannot_Contain_Unicode_Escapes, "E0035",        \
      Diagnostic_Severity::error, { Source_Code_Span escape_sequence; },        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "RegExp literal flags cannot contain Unicode escapes"),       \
              escape_sequence))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Return_Statement_Returns_Nothing, "E0179",                           \
      Diagnostic_Severity::warning, { Source_Code_Span return_keyword; },       \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("return statement returns nothing (undefined)"),    \
          return_keyword))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Spread_Parameter_Cannot_Be_This, "E0304",                            \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span this_keyword;                                          \
        Source_Code_Span spread_operator;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("cannot use '...' on 'this' parameter"),        \
              spread_operator))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Statement_Before_First_Switch_Case, "E0198",                         \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_statement; },   \
      MESSAGE(QLJS_TRANSLATABLE("unexpected statement before first switch "     \
                                "case, expected 'case' or 'default'"),          \
              unexpected_statement))                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Stray_Comma_In_Let_Statement, "E0036", Diagnostic_Severity::error,   \
      { Source_Code_Span where; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("stray comma in let statement"), where))        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Stray_Comma_In_Parameter, "E0180", Diagnostic_Severity::error,       \
      { Source_Code_Span comma; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("stray comma in function parameter"), comma))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_String_Namespace_Name_Is_Only_Allowed_With_Declare_Module, "E0359",  \
      Diagnostic_Severity::error, { Source_Code_Span module_name; },            \
      MESSAGE(QLJS_TRANSLATABLE("string module name is only allowed with "      \
                                "'declare module'"),                            \
              module_name))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_String_Namespace_Name_Is_Only_Allowed_At_Top_Level, "E0361",         \
      Diagnostic_Severity::error, { Source_Code_Span module_name; },            \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "module with string name is only allowed at the top level"),  \
              module_name))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_This_Parameter_Must_Be_First, "E0303", Diagnostic_Severity::error,   \
      {                                                                         \
        Source_Code_Span this_keyword;                                          \
        Source_Code_Span first_parameter_begin;                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'this' must be the first parameter"),          \
              this_keyword)                                                     \
          MESSAGE(QLJS_TRANSLATABLE("first parameter starts here"),             \
                  first_parameter_begin))                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_This_Parameter_Not_Allowed_In_Arrow_Functions, "E0301",              \
      Diagnostic_Severity::error, { Source_Code_Span this_keyword; },           \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'this' parameters are not allowed in arrow functions"),      \
              this_keyword))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_This_Parameter_Not_Allowed_In_JavaScript, "E0305",                   \
      Diagnostic_Severity::error, { Source_Code_Span this_keyword; },           \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'this' parameters are not allowed in JavaScript"),           \
              this_keyword))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_This_Parameter_Not_Allowed_When_Destructuring, "E0302",              \
      Diagnostic_Severity::error, { Source_Code_Span this_keyword; },           \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'this' parameter not allowed when destructuring"),           \
              this_keyword))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Abstract_Class_Not_Allowed_In_JavaScript, "E0244",        \
      Diagnostic_Severity::error, { Source_Code_Span abstract_keyword; },       \
      MESSAGE(QLJS_TRANSLATABLE("{0} classes are not allowed in JavaScript"),   \
              abstract_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Angle_Type_Assertion_Not_Allowed_In_Tsx, "E0283",         \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span bracketed_type;                                        \
        Source_Code_Span expected_as;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript <Type> type assertions are not "    \
                                "allowed in JSX mode"),                         \
              bracketed_type)                                                   \
          MESSAGE(QLJS_TRANSLATABLE(                                            \
                      "write the type assertion with 'as' here instead"),       \
                  expected_as))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_As_Const_With_Non_Literal_Typeable, "E0291",              \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span expression;                                            \
        Source_Code_Span as_const;                                              \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("'as const' is only allowed on literals (array, "   \
                            "object, string, boolean) and enum members"),       \
          expression)                                                           \
          MESSAGE(QLJS_TRANSLATABLE("'as const' located here"), as_const))      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_As_Type_Assertion_Not_Allowed_In_JavaScript, "E0281",     \
      Diagnostic_Severity::error, { Source_Code_Span as_keyword; },             \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript 'as' type assertions are not "      \
                                "allowed in JavaScript"),                       \
              as_keyword))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_As_Or_Satisfies_Used_For_Parameter_Type_Annotation,       \
      "E0282", Diagnostic_Severity::error, { Source_Code_Span bad_keyword; },   \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "use ':' instead of '{0}' to type a function parameter"),     \
              bad_keyword))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Assignment_Asserted_Fields_Not_Allowed_In_Declare_Class,  \
      "E0336", Diagnostic_Severity::error, { Source_Code_Span bang; },          \
      MESSAGE(QLJS_TRANSLATABLE("assignment-asserted fields are "               \
                                "not allowed in 'declare class'"),              \
              bang))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Assignment_Asserted_Fields_Not_Allowed_In_Interfaces,     \
      "E0238", Diagnostic_Severity::error, { Source_Code_Span bang; },          \
      MESSAGE(QLJS_TRANSLATABLE("assignment-asserted fields are "               \
                                "not supported in interfaces"),                 \
              bang))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Assignment_Asserted_Fields_Not_Allowed_In_JavaScript,     \
      "E0239", Diagnostic_Severity::error, { Source_Code_Span bang; },          \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript assignment-asserted fields are "    \
                                "not supported in JavaScript"),                 \
              bang))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Assignment_Asserted_Field_Cannot_Have_Initializer,        \
      "E0290", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span equal;                                                 \
        Source_Code_Span bang;                                                  \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "assignment-assertion fields cannot have default values"),    \
              equal)                                                            \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("here is the assignment assertion operator"),   \
              bang))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Assignment_Asserted_Field_Must_Have_A_Type, "E0236",      \
      Diagnostic_Severity::error, { Source_Code_Span bang; },                   \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "assignment-asserted field must have a type annotation"),     \
              bang))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Assignment_Asserted_Method, "E0240",                      \
      Diagnostic_Severity::error, { Source_Code_Span bang; },                   \
      MESSAGE(QLJS_TRANSLATABLE("'{0}' is not allowed on methods"), bang))      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Catch_Type_Annotation_Must_Be_Any, "E0256",               \
      Diagnostic_Severity::error, { Source_Code_Span type_expression; },        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "catch variable can only be typed as '*', 'any', or 'unknown'"),  \
          type_expression))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Class_Implements_Not_Allowed_In_JavaScript, "E0247",      \
      Diagnostic_Severity::error, { Source_Code_Span implements_keyword; },     \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript 'implements' is not allowed in JavaScript"),      \
              implements_keyword))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Delete_Cannot_Delete_Variables, "E0325",                  \
      Diagnostic_Severity::error, { Source_Code_Span delete_expression; },      \
      MESSAGE(QLJS_TRANSLATABLE("cannot delete variables in TypeScript"),       \
              delete_expression))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Enum_Auto_Member_Needs_Initializer_After_Computed,        \
      "E0252", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span auto_member_name;                                      \
        Source_Code_Span computed_expression;                                   \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("enum member needs initializer"),               \
              auto_member_name)                                                 \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("computed value disables enum autoincrement"),  \
              computed_expression))                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Enum_Computed_Name_Must_Be_Simple, "E0249",               \
      Diagnostic_Severity::error, { Source_Code_Span expression; },             \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "computed enum member name must be a simple string"),         \
              expression))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Enum_Is_Not_Allowed_In_JavaScript, "E0127",               \
      Diagnostic_Severity::error, { Source_Code_Span enum_keyword; },           \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript's 'enum' feature is not allowed in JavaScript"),  \
              enum_keyword))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Enum_Member_Name_Cannot_Be_Number, "E0250",               \
      Diagnostic_Severity::error, { Source_Code_Span number; },                 \
      MESSAGE(QLJS_TRANSLATABLE("enum member name cannot be numeric"),          \
              number))                                                          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Enum_Value_Must_Be_Constant, "E0251",                     \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span expression;                                            \
        Enum_Kind declared_enum_kind;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "{1:headlinese} value must be a compile-time constant"),      \
              expression, declared_enum_kind))                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Export_Equal_Not_Allowed_In_JavaScript, "E0370",          \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span equal;                                                 \
        Source_Code_Span export_keyword;                                        \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'export =' is not allowed; write 'export default' or "       \
                  "'module.exports =' (CommonJS) instead"),                     \
              equal)                                                            \
          MESSAGE(QLJS_TRANSLATABLE("'export' keyword here"), export_keyword))  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Implements_Must_Be_After_Extends, "E0246",                \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span implements_keyword;                                    \
        Source_Code_Span extends_keyword;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'extends' must be before 'implements'"),       \
              extends_keyword)                                                  \
          MESSAGE(QLJS_TRANSLATABLE(                                            \
                      "move the 'extends' clause before 'implements' here"),    \
                  implements_keyword))                                          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Import_Alias_Not_Allowed_In_JavaScript, "E0274",          \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span import_keyword;                                        \
        Source_Code_Span equal;                                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript import aliases are not allowed in JavaScript"),   \
              equal)                                                            \
          MESSAGE(QLJS_TRANSLATABLE("write 'const' instead of '{0}' here"),     \
                  import_keyword))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Index_Signature_Cannot_Be_Method, "E0227",                \
      Diagnostic_Severity::error, { Source_Code_Span left_paren; },             \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("index signature must be a field, not a method"),   \
          left_paren))                                                          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Index_Signature_Needs_Type, "E0225",                      \
      Diagnostic_Severity::error, { Source_Code_Span expected_type; },          \
      MESSAGE(QLJS_TRANSLATABLE("index signatures require a value type"),       \
              expected_type))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Infer_Outside_Conditional_Type, "E0367",                  \
      Diagnostic_Severity::error, { Source_Code_Span infer_keyword; },          \
      MESSAGE(QLJS_TRANSLATABLE("'infer' is only allowed between 'extends' "    \
                                "and '?' in conditional types"),                \
              infer_keyword))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Infer_Requires_Parentheses, "E0366",                      \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span infer_and_type;                                        \
        Source_Code_Span type;                                                  \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("parentheses are required around 'infer {1}'"),     \
          infer_and_type, type))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Function_Overload_Signature_Must_Have_Same_Name,          \
      "E0316", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span overload_name;                                         \
        Source_Code_Span function_name;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "function overload signature must be named '{1}'"),           \
              overload_name, function_name)                                     \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("overloaded function '{0}' declared here"),     \
              function_name))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Function_Overload_Signature_Must_Not_Have_Generator_Star, \
      "E0318", Diagnostic_Severity::error,                                      \
      { Source_Code_Span generator_star; },                                     \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "function overload signature cannot have generator '*'"),     \
              generator_star))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Generic_Arrow_Needs_Comma_In_JSX_Mode, "E0285",           \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span generic_parameters_less;                               \
        Source_Code_Span expected_comma;                                        \
        Source_Code_Span arrow;                                                 \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("generic arrow function needs ',' here in TSX"),    \
          expected_comma))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Generic_Parameter_List_Is_Empty, "E0264",                 \
      Diagnostic_Severity::error, { Source_Code_Span expected_parameter; },     \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "expected at least one parameter in generic parameter list"),     \
          expected_parameter))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Generics_Not_Allowed_In_JavaScript, "E0233",              \
      Diagnostic_Severity::error, { Source_Code_Span opening_less; },           \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript generics are not "                  \
                                "allowed in JavaScript code"),                  \
              opening_less))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Type_Export_Not_Allowed_In_JavaScript, "E0278",           \
      Diagnostic_Severity::error, { Source_Code_Span type_keyword; },           \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript type exports are not "              \
                                "allowed in JavaScript"),                       \
              type_keyword))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Inline_Type_Export_Not_Allowed_In_Type_Only_Export,       \
      "E0280", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span inline_type_keyword;                                   \
        Source_Code_Span type_only_keyword;                                     \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'type' cannot be used twice in export"),       \
              inline_type_keyword)                                              \
          MESSAGE(QLJS_TRANSLATABLE("remove this 'type'"), type_only_keyword))  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Inline_Type_Import_Not_Allowed_In_Type_Only_Import,       \
      "E0272", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span inline_type_keyword;                                   \
        Source_Code_Span type_only_keyword;                                     \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'type' cannot be used twice in import"),       \
              inline_type_keyword)                                              \
          MESSAGE(QLJS_TRANSLATABLE("remove this 'type'"), type_only_keyword))  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Interfaces_Cannot_Contain_Static_Blocks, "E0243",         \
      Diagnostic_Severity::error, { Source_Code_Span static_token; },           \
      MESSAGE(QLJS_TRANSLATABLE("interfaces cannot contain static blocks"),     \
              static_token))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Declare_Class_Cannot_Contain_Static_Block_Statement,      \
      "E0332", Diagnostic_Severity::error, { Source_Code_Span static_token; },  \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("'declare class' cannot contain static block"),     \
          static_token))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Interfaces_Not_Allowed_In_JavaScript, "E0213",            \
      Diagnostic_Severity::error, { Source_Code_Span interface_keyword; },      \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript's 'interface' feature is not "      \
                                "allowed in JavaScript code"),                  \
              interface_keyword))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Missing_Name_And_Colon_In_Named_Tuple_Type, "E0319",      \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span expected_name_and_colon;                               \
        Source_Code_Span existing_name;                                         \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("missing name for element in named tuple type"),    \
          expected_name_and_colon)                                              \
          MESSAGE(                                                              \
              QLJS_TRANSLATABLE("this tuple type is a named tuple type "        \
                                "because at least one element has a name"),     \
              existing_name))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Missing_Name_In_Named_Tuple_Type, "E0320",                \
      Diagnostic_Severity::error, { Source_Code_Span colon; },                  \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("missing name for element in named tuple type"),    \
          colon))                                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Named_Tuple_Element_Question_After_Name_And_Type,         \
      "E0322", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span type_question;                                         \
        Source_Code_Span name_question;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'?' belongs only after the tuple element "     \
                                "name, not also after the type"),               \
              type_question))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Named_Tuple_Element_Question_After_Type, "E0314",         \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span question;                                              \
        Source_Code_Span expected_question;                                     \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "'?' belongs after the tuple element name, not after the type"),  \
          question)                                                             \
          MESSAGE(QLJS_TRANSLATABLE("'?' goes here"), expected_question))       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Named_Tuple_Element_Spread_Before_Name_And_Type,          \
      "E0329", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span type_spread;                                           \
        Source_Code_Span name_spread;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'...' belongs only before the tuple element "  \
                                "name, not also before the type"),              \
              type_spread))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Named_Tuple_Element_Spread_Before_Type, "E0328",          \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span spread;                                                \
        Source_Code_Span expected_spread;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'...' belongs before the tuple element "       \
                                "name, not before the type"),                   \
              spread)                                                           \
          MESSAGE(QLJS_TRANSLATABLE("'...' goes here"), expected_spread))       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Namespace_Cannot_Export_Default, "E0363",                 \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span default_keyword;                                       \
        Source_Code_Span namespace_keyword; /* 'namespace' or 'module' */       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "cannot 'export default' from inside a namespace"),           \
              default_keyword)                                                  \
          MESSAGE(QLJS_TRANSLATABLE("namespace starts here"),                   \
                  namespace_keyword))                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Namespaces_Not_Allowed_In_JavaScript, "E0273",            \
      Diagnostic_Severity::error, { Source_Code_Span namespace_keyword; },      \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript namespaces are not allowed in JavaScript"),       \
              namespace_keyword))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Non_Null_Assertion_Not_Allowed_In_JavaScript, "E0261",    \
      Diagnostic_Severity::error, { Source_Code_Span bang; },                   \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "TypeScript non-null assertions are not allowed in JavaScript"),  \
          bang))                                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Optional_Parameters_Not_Allowed_In_JavaScript, "E0308",   \
      Diagnostic_Severity::error, { Source_Code_Span question; },               \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "TypeScript optional parameters are not allowed in JavaScript"),  \
          question))                                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Optional_Properties_Not_Allowed_In_JavaScript, "E0228",   \
      Diagnostic_Severity::error, { Source_Code_Span question; },               \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript optional properties are not "       \
                                "allowed in JavaScript code"),                  \
              question))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Optional_Tuple_Element_Cannot_Follow_Spread_Element,      \
      "E0323", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span optional_question;                                     \
        Source_Code_Span previous_spread;                                       \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "optional tuple elements cannot come after spread elements"),     \
          optional_question)                                                    \
          MESSAGE(QLJS_TRANSLATABLE("prior spread element is here"),            \
                  previous_spread))                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Parameter_Property_Cannot_Be_Destructured, "E0372",       \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span destructure_token;                                     \
        Source_Code_Span property_keyword;                                      \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("parameter properties cannot be destructured"),     \
          destructure_token)                                                    \
          MESSAGE(QLJS_TRANSLATABLE("property declared using '{0}' here"),      \
                  property_keyword))                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Parameter_Property_Cannot_Be_Rest, "E0377",               \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span spread;                                                \
        Source_Code_Span property_keyword;                                      \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "parameter properties cannot be a rest parameter"),           \
              spread)                                                           \
          MESSAGE(QLJS_TRANSLATABLE("property declared using '{0}' here"),      \
                  property_keyword))                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Parameter_Property_Not_Allowed_In_Declare_Class,          \
      "E0375", Diagnostic_Severity::error,                                      \
      {                                                                         \
        Source_Code_Span property_keyword;                                      \
        Source_Code_Span declare_keyword;                                       \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("parameter properties are not "                 \
                                "allowed in 'declare class'"),                  \
              property_keyword)                                                 \
          MESSAGE(QLJS_TRANSLATABLE("'declare' specified here"),                \
                  declare_keyword))                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Parameter_Property_Not_Allowed_In_JavaScript, "E0371",    \
      Diagnostic_Severity::error, { Source_Code_Span property_keyword; },       \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript parameter properties are not "      \
                                "allowed in JavaScript"),                       \
              property_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Parameter_Property_Only_Allowed_In_Class_Constructor,     \
      "E0378", Diagnostic_Severity::error,                                      \
      { Source_Code_Span property_keyword; },                                   \
      MESSAGE(QLJS_TRANSLATABLE("parameter properties are only "                \
                                "allowed in class constructors"),               \
              property_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Private_Not_Allowed_In_JavaScript, "E0222",               \
      Diagnostic_Severity::error, { Source_Code_Span specifier; },              \
      MESSAGE(QLJS_TRANSLATABLE("'private' is not allowed in JavaScript"),      \
              specifier))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Protected_Not_Allowed_In_JavaScript, "E0234",             \
      Diagnostic_Severity::error, { Source_Code_Span specifier; },              \
      MESSAGE(QLJS_TRANSLATABLE("'protected' is not allowed in JavaScript"),    \
              specifier))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Public_Not_Allowed_In_JavaScript, "E0289",                \
      Diagnostic_Severity::error, { Source_Code_Span specifier; },              \
      MESSAGE(QLJS_TRANSLATABLE("'public' is not allowed in JavaScript"),       \
              specifier))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Readonly_Fields_Not_Allowed_In_JavaScript, "E0230",       \
      Diagnostic_Severity::error, { Source_Code_Span readonly_keyword; },       \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript's 'readonly' feature is not "       \
                                "allowed in JavaScript code"),                  \
              readonly_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Readonly_Method, "E0231", Diagnostic_Severity::error,     \
      { Source_Code_Span readonly_keyword; },                                   \
      MESSAGE(QLJS_TRANSLATABLE("methods cannot be readonly"),                  \
              readonly_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Readonly_In_Type_Needs_Array_Or_Tuple_Type, "E0313",      \
      Diagnostic_Severity::error, { Source_Code_Span readonly_keyword; },       \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'readonly' only works with array types and tuple types"),    \
              readonly_keyword))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Required_Tuple_Element_After_Optional_Element, "E0321",   \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span expected_question;                                     \
        Source_Code_Span previous_optional_question;                            \
      },                                                                        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("expected '?' to mark tuple element as optional"),  \
          expected_question)                                                    \
          MESSAGE(QLJS_TRANSLATABLE("only optional tuple elements can follow "  \
                                    "this optional tuple element"),             \
                  previous_optional_question))                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Requires_Space_Between_Greater_And_Equal, "E0365",        \
      Diagnostic_Severity::error, { Source_Code_Span greater_equal; },          \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript requires whitespace between '>' and '=' here"),   \
              greater_equal))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Satisfies_Not_Allowed_In_JavaScript, "E0364",             \
      Diagnostic_Severity::error, { Source_Code_Span satisfies_keyword; },      \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript 'satisfies' operator is not "       \
                                "allowed in JavaScript"),                       \
              satisfies_keyword))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Type_Annotation_In_Expression, "E0254",                   \
      Diagnostic_Severity::error, { Source_Code_Span type_colon; },             \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "unexpected ':' in expression; did you mean 'as'?"),          \
              type_colon))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Type_Annotations_Not_Allowed_In_JavaScript, "E0224",      \
      Diagnostic_Severity::error, { Source_Code_Span type_colon; },             \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript type annotations are not "          \
                                "allowed in JavaScript code"),                  \
              type_colon))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Type_Alias_Not_Allowed_In_JavaScript, "E0267",            \
      Diagnostic_Severity::error, { Source_Code_Span type_keyword; },           \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("TypeScript types are not allowed in JavaScript"),  \
          type_keyword))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Type_Only_Import_Cannot_Import_Default_And_Named,         \
      "E0268", Diagnostic_Severity::error, { Source_Code_Span type_keyword; },  \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript type imports cannot import both "   \
                                "default and named exports"),                   \
              type_keyword))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Type_Import_Not_Allowed_In_JavaScript, "E0270",           \
      Diagnostic_Severity::error, { Source_Code_Span type_keyword; },           \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript type imports are not "              \
                                "allowed in JavaScript"),                       \
              type_keyword))                                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Spread_Element_Cannot_Be_Optional, "E0324",               \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span optional_question;                                     \
        Source_Code_Span spread;                                                \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("spread tuple elements cannot be optional"),    \
              optional_question)                                                \
          MESSAGE(QLJS_TRANSLATABLE("spread starts here"), spread))             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Style_Const_Field, "E0165", Diagnostic_Severity::error,   \
      { Source_Code_Span const_token; },                                        \
      MESSAGE(QLJS_TRANSLATABLE("const fields within classes are only "         \
                                "allowed in TypeScript, not JavaScript"),       \
              const_token))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_TypeScript_Variance_Keywords_In_Wrong_Order, "E0368",                \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span in_keyword;                                            \
        Source_Code_Span out_keyword;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "'out in' is not allowed; write 'in out' instead"),           \
              in_keyword))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_Block_Comment, "E0037", Diagnostic_Severity::error,         \
      { Source_Code_Span comment_open; },                                       \
      MESSAGE(QLJS_TRANSLATABLE("unclosed block comment"), comment_open))       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_Class_Block, "E0199", Diagnostic_Severity::error,           \
      { Source_Code_Span block_open; },                                         \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("unclosed class; expected '}' by end of file"),     \
          block_open))                                                          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_Code_Block, "E0134", Diagnostic_Severity::error,            \
      { Source_Code_Span block_open; },                                         \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "unclosed code block; expected '}' by end of file"),          \
              block_open))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_Interface_Block, "E0215", Diagnostic_Severity::error,       \
      { Source_Code_Span block_open; },                                         \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "unclosed interface; expected '}' by end of file"),           \
              block_open))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_Identifier_Escape_Sequence, "E0038",                        \
      Diagnostic_Severity::error, { Source_Code_Span escape_sequence; },        \
      MESSAGE(QLJS_TRANSLATABLE("unclosed identifier escape sequence"),         \
              escape_sequence))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_Object_Literal, "E0161", Diagnostic_Severity::error,        \
      {                                                                         \
        Source_Code_Span object_open;                                           \
        Source_Code_Span expected_object_close;                                 \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("unclosed object literal; expected '}'"),       \
              expected_object_close)                                            \
          MESSAGE(QLJS_TRANSLATABLE("object literal started here"),             \
                  object_open))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_Regexp_Literal, "E0039", Diagnostic_Severity::error,        \
      { Source_Code_Span regexp_literal; },                                     \
      MESSAGE(QLJS_TRANSLATABLE("unclosed regexp literal"), regexp_literal))    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_String_Literal, "E0040", Diagnostic_Severity::error,        \
      { Source_Code_Span string_literal; },                                     \
      MESSAGE(QLJS_TRANSLATABLE("unclosed string literal"), string_literal))    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_JSX_String_Literal, "E0181", Diagnostic_Severity::error,    \
      { Source_Code_Span string_literal_begin; },                               \
      MESSAGE(QLJS_TRANSLATABLE("unclosed string literal"),                     \
              string_literal_begin))                                            \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unclosed_Template, "E0041", Diagnostic_Severity::error,              \
      { Source_Code_Span incomplete_template; },                                \
      MESSAGE(QLJS_TRANSLATABLE("unclosed template"), incomplete_template))     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_At_Character, "E0042", Diagnostic_Severity::error,        \
      { Source_Code_Span character; },                                          \
      MESSAGE(QLJS_TRANSLATABLE("unexpected '@'"), character))                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Arrow_After_Expression, "E0160",                          \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span arrow;                                                 \
        Source_Code_Span expression;                                            \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("unexpected '{0}'"), arrow)                     \
          MESSAGE(QLJS_TRANSLATABLE("expected parameter for arrow function, "   \
                                    "but got an expression instead"),           \
                  expression))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Arrow_After_Literal, "E0158",                             \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span arrow;                                                 \
        Source_Code_Span literal_parameter;                                     \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("unexpected '{0}'"), arrow)                     \
          MESSAGE(QLJS_TRANSLATABLE("expected parameter for arrow function, "   \
                                    "but got a literal instead"),               \
                  literal_parameter))                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Backslash_In_Identifier, "E0043",                         \
      Diagnostic_Severity::error, { Source_Code_Span backslash; },              \
      MESSAGE(QLJS_TRANSLATABLE("unexpected '\\' in identifier"), backslash))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Case_Outside_Switch_Statement, "E0115",                   \
      Diagnostic_Severity::error, { Source_Code_Span case_token; },             \
      MESSAGE(QLJS_TRANSLATABLE("unexpected 'case' outside switch statement"),  \
              case_token))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Characters_In_Number, "E0044",                            \
      Diagnostic_Severity::error, { Source_Code_Span characters; },             \
      MESSAGE(QLJS_TRANSLATABLE("unexpected characters in number literal"),     \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Control_Character, "E0045", Diagnostic_Severity::error,   \
      { Source_Code_Span character; },                                          \
      MESSAGE(QLJS_TRANSLATABLE("unexpected control character"), character))    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Characters_In_Binary_Number, "E0046",                     \
      Diagnostic_Severity::error, { Source_Code_Span characters; },             \
      MESSAGE(QLJS_TRANSLATABLE("unexpected characters in binary literal"),     \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Characters_In_Octal_Number, "E0047",                      \
      Diagnostic_Severity::error, { Source_Code_Span characters; },             \
      MESSAGE(QLJS_TRANSLATABLE("unexpected characters in octal literal"),      \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Characters_In_Hex_Number, "E0048",                        \
      Diagnostic_Severity::error, { Source_Code_Span characters; },             \
      MESSAGE(QLJS_TRANSLATABLE("unexpected characters in hex literal"),        \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Default_Outside_Switch_Statement, "E0116",                \
      Diagnostic_Severity::error, { Source_Code_Span default_token; },          \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("unexpected 'default' outside switch statement"),   \
          default_token))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Greater_In_JSX_Text, "E0182",                             \
      Diagnostic_Severity::error, { Source_Code_Span greater; },                \
      MESSAGE(QLJS_TRANSLATABLE("'>' is not allowed directly in JSX text; "     \
                                "write {{'>'} or &gt; instead"),                \
              greater))                                                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Literal_In_Parameter_List, "E0159",                       \
      Diagnostic_Severity::error, { Source_Code_Span literal; },                \
      MESSAGE(QLJS_TRANSLATABLE("unexpected literal in parameter list; "        \
                                "expected parameter name"),                     \
              literal))                                                         \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Right_Curly_In_JSX_Text, "E0183",                         \
      Diagnostic_Severity::error, { Source_Code_Span right_curly; },            \
      MESSAGE(QLJS_TRANSLATABLE("'}' is not allowed directly in JSX text; "     \
                                "write {{'}'} instead"),                        \
              right_curly))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Question_In_Expression, "E0307",                          \
      Diagnostic_Severity::error, { Source_Code_Span question; },               \
      MESSAGE(QLJS_TRANSLATABLE("unexpected '?'"), question))                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Question_When_Destructuring, "E0309",                     \
      Diagnostic_Severity::error, { Source_Code_Span question; },               \
      MESSAGE(QLJS_TRANSLATABLE("unexpected '?' when destructuring"),           \
              question))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Semicolon_In_C_Style_For_Loop, "E0102",                   \
      Diagnostic_Severity::error, { Source_Code_Span semicolon; },              \
      MESSAGE(QLJS_TRANSLATABLE("C-style for loops have only three "            \
                                "semicolon-separated components"),              \
              semicolon))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Semicolon_In_For_In_Loop, "E0110",                        \
      Diagnostic_Severity::error, { Source_Code_Span semicolon; },              \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("for-in loop expression cannot have semicolons"),   \
          semicolon))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Semicolon_In_For_Of_Loop, "E0109",                        \
      Diagnostic_Severity::error, { Source_Code_Span semicolon; },              \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("for-of loop expression cannot have semicolons"),   \
          semicolon))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unopened_Block_Comment, "E0210", Diagnostic_Severity::error,         \
      { Source_Code_Span comment_close; },                                      \
      MESSAGE(QLJS_TRANSLATABLE("unopened block comment"), comment_close))      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unused_Variable_Shadows, "E0196", Diagnostic_Severity::warning,      \
      {                                                                         \
        Source_Code_Span shadowing_declaration;                                 \
        Source_Code_Span shadowed_declaration;                                  \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("new variable shadows existing variable"),      \
              shadowing_declaration)                                            \
          MESSAGE(QLJS_TRANSLATABLE("existing variable declared here"),         \
                  shadowed_declaration))                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_No_Digits_In_Binary_Number, "E0049", Diagnostic_Severity::error,     \
      { Source_Code_Span characters; },                                         \
      MESSAGE(QLJS_TRANSLATABLE("binary number literal has no digits"),         \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_No_Digits_In_Hex_Number, "E0050", Diagnostic_Severity::error,        \
      { Source_Code_Span characters; },                                         \
      MESSAGE(QLJS_TRANSLATABLE("hex number literal has no digits"),            \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_No_Digits_In_Octal_Number, "E0051", Diagnostic_Severity::error,      \
      { Source_Code_Span characters; },                                         \
      MESSAGE(QLJS_TRANSLATABLE("octal number literal has no digits"),          \
              characters))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Non_Null_Assertion_Not_Allowed_In_Parameter, "E0260",                \
      Diagnostic_Severity::error, { Source_Code_Span bang; },                   \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "TypeScript non-null assertion is not allowed on parameters"),    \
          bang))                                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Hash_Character, "E0052", Diagnostic_Severity::error,      \
      { Source_Code_Span where; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("unexpected '#'"), where))                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Bom_Before_Shebang, "E0095", Diagnostic_Severity::error,  \
      { Source_Code_Span bom; },                                                \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "unicode byte order mark (BOM) cannot appear before #! "      \
                  "at beginning of script"),                                    \
              bom))                                                             \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Identifier_In_Expression, "E0147",                        \
      Diagnostic_Severity::error, { Source_Code_Span unexpected; },             \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "unexpected identifier in expression; missing operator before"),  \
          unexpected))                                                          \
                                                                                \
  /* NOTE(strager): Try not to use this error. Find or make a more descriptive  \
     and helpful error instead. */                                              \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Token, "E0054", Diagnostic_Severity::error,               \
      { Source_Code_Span token; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("unexpected token"), token))                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Token_After_Export, "E0112", Diagnostic_Severity::error,  \
      { Source_Code_Span unexpected_token; },                                   \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "unexpected token in export; expected 'export default ...' "  \
                  "or 'export {{name}' or 'export * from ...' or 'export "      \
                  "class' or 'export function' or 'export let'"),               \
              unexpected_token))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Token_In_Variable_Declaration, "E0114",                   \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_token; },       \
      MESSAGE(QLJS_TRANSLATABLE("unexpected token in variable declaration; "    \
                                "expected variable name"),                      \
              unexpected_token))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unmatched_Indexing_Bracket, "E0055", Diagnostic_Severity::error,     \
      { Source_Code_Span left_square; },                                        \
      MESSAGE(QLJS_TRANSLATABLE("unmatched indexing bracket"), left_square))    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unmatched_Parenthesis, "E0056", Diagnostic_Severity::error,          \
      { Source_Code_Span where; },                                              \
      MESSAGE(QLJS_TRANSLATABLE("unmatched parenthesis"), where))               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unmatched_Right_Curly, "E0143", Diagnostic_Severity::error,          \
      { Source_Code_Span right_curly; },                                        \
      MESSAGE(QLJS_TRANSLATABLE("unmatched '}'"), right_curly))                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Use_Of_Undeclared_Parameter_In_Type_Predicate, "E0315",              \
      Diagnostic_Severity::error, { Source_Code_Span name; },                   \
      MESSAGE(QLJS_TRANSLATABLE("{0} is not the name of a parameter"), name))   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Use_Of_Undeclared_Type, "E0214", Diagnostic_Severity::warning,       \
      { Source_Code_Span name; },                                               \
      MESSAGE(QLJS_TRANSLATABLE("use of undeclared type: {0}"), name))          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Use_Of_Undeclared_Variable, "E0057", Diagnostic_Severity::warning,   \
      { Source_Code_Span name; },                                               \
      MESSAGE(QLJS_TRANSLATABLE("use of undeclared variable: {0}"), name))      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Variable_Used_Before_Declaration, "E0058",                           \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span use;                                                   \
        Source_Code_Span declaration;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("variable used before declaration: {0}"), use)  \
          MESSAGE(QLJS_TRANSLATABLE("variable declared here"), declaration))    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Function_Call_Before_Declaration_In_Block_Scope, "E0077",            \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span use;                                                   \
        Source_Code_Span declaration;                                           \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "function called before declaration in block scope: {0}"),    \
              use)                                                              \
          MESSAGE(QLJS_TRANSLATABLE("function declared here"), declaration))    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Import_Cannot_Have_Declare_Keyword, "E0360",                         \
      Diagnostic_Severity::error, { Source_Code_Span declare_keyword; },        \
      MESSAGE(QLJS_TRANSLATABLE("cannot use 'declare' keyword with 'import'"),  \
              declare_keyword))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Interface_Fields_Cannot_Have_Initializers, "E0221",                  \
      Diagnostic_Severity::error, { Source_Code_Span equal; },                  \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript interface fields cannot be initalized"),          \
              equal))                                                           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Interface_Methods_Cannot_Be_Async, "E0217",                          \
      Diagnostic_Severity::error, { Source_Code_Span async_keyword; },          \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript interface methods cannot be marked 'async'"),     \
              async_keyword))                                                   \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Interface_Methods_Cannot_Be_Generators, "E0218",                     \
      Diagnostic_Severity::error, { Source_Code_Span star; },                   \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "TypeScript interface methods cannot be marked as a generator"),  \
          star))                                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Interface_Methods_Cannot_Contain_Bodies, "E0220",                    \
      Diagnostic_Severity::error, { Source_Code_Span body_start; },             \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript interface methods cannot contain a body"),        \
              body_start))                                                      \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Interface_Properties_Cannot_Be_Explicitly_Public, "E0237",           \
      Diagnostic_Severity::error, { Source_Code_Span public_keyword; },         \
      MESSAGE(QLJS_TRANSLATABLE("interface properties "                         \
                                "cannot be marked public explicitly"),          \
              public_keyword))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Interface_Properties_Cannot_Be_Private, "E0219",                     \
      Diagnostic_Severity::error,                                               \
      { Source_Code_Span property_name_or_private_keyword; },                   \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "interface properties are always public and cannot be private"),  \
          property_name_or_private_keyword))                                    \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Interface_Properties_Cannot_Be_Protected, "E0288",                   \
      Diagnostic_Severity::error, { Source_Code_Span protected_keyword; },      \
      MESSAGE(QLJS_TRANSLATABLE("TypeScript interface properties are always "   \
                                "public and cannot be marked protected"),       \
              protected_keyword))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Interface_Properties_Cannot_Be_Static, "E0216",                      \
      Diagnostic_Severity::error, { Source_Code_Span static_keyword; },         \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "TypeScript interface properties cannot be 'static'"),        \
              static_keyword))                                                  \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Break, "E0200", Diagnostic_Severity::error,                  \
      { Source_Code_Span break_statement; },                                    \
      MESSAGE(QLJS_TRANSLATABLE(                                                \
                  "break can only be used inside of a loop or switch"),         \
              break_statement))                                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Invalid_Continue, "E0201", Diagnostic_Severity::error,               \
      { Source_Code_Span continue_statement; },                                 \
      MESSAGE(QLJS_TRANSLATABLE("continue can only be used inside of a loop"),  \
              continue_statement))                                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_String_Comp_Contains_Lower, "E0286",                       \
      Diagnostic_Severity::warning, { Source_Code_Span span_operator; },        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("lower case letters compared with toUpperCase"),    \
          span_operator))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_String_Comp_Contains_Upper, "E0287",                       \
      Diagnostic_Severity::warning, { Source_Code_Span span_operator; },        \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE("upper case letters compared with toLowerCase"),    \
          span_operator))                                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_Strict_Comp_Against_Array_Literal, "E0341",                \
      Diagnostic_Severity::warning, { Source_Code_Span equals_operator; },      \
      MESSAGE(                                                                  \
          QLJS_TRANSLATABLE(                                                    \
              "using '{0}' against an array literal does not compare items"),   \
          equals_operator))                                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_Comp_Against_Arrow_Function, "E0342",                      \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span equals_operator;                                       \
        String8_View comparison_result;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("using '{0}' against an arrow "                 \
                                "function always returns '{1}'"),               \
              equals_operator, comparison_result))                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_Comp_Against_Class_Literal, "E0343",                       \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span equals_operator;                                       \
        String8_View comparison_result;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("using '{0}' against a class "                  \
                                "literal always returns '{1}'"),                \
              equals_operator, comparison_result))                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_Strict_Comp_Against_Empty_Array_Literal, "E0344",          \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span equals_operator;                                       \
        String8_View comparison_result;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'{0} []' is always '{1}'"), equals_operator,   \
              comparison_result))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_Comp_Against_Object_Literal, "E0345",                      \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span equals_operator;                                       \
        String8_View comparison_result;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("using '{0}' against an object "                \
                                "literal always returns '{1}'"),                \
              equals_operator, comparison_result))                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_Comp_Against_Regular_Expression_Literal, "E0346",          \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span equals_operator;                                       \
        String8_View comparison_result;                                         \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("using '{0}' against a regular "                \
                                "expression literal always returns '{1}'"),     \
              equals_operator, comparison_result))                              \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Function_Parameter_Is_Parenthesized, "E0349",             \
      Diagnostic_Severity::error,                                               \
      { Source_Code_Span left_paren_to_right_paren; },                          \
      MESSAGE(QLJS_TRANSLATABLE("function parameter cannot be parenthesized"),  \
              left_paren_to_right_paren))                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Comma_After_Field_Initialization, "E0330",                \
      Diagnostic_Severity::error, { Source_Code_Span comma; },                  \
      MESSAGE(QLJS_TRANSLATABLE("',' should be ';' instead"), comma))           \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Colon_After_Generic_Definition, "E0331",                  \
      Diagnostic_Severity::error, { Source_Code_Span colon; },                  \
      MESSAGE(QLJS_TRANSLATABLE("':' should be 'extends' instead"), colon))     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Pointless_Nullish_Coalescing_Operator, "E0369",                      \
      Diagnostic_Severity::warning, { Source_Code_Span question_question; },    \
      MESSAGE(QLJS_TRANSLATABLE("nullish coalescing operator does nothing "     \
                                "when left operand is never null"),             \
              question_question))                                               \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Bang_Equal_Equal_Interpreted_As_Non_Null_Assertion, "E0373",         \
      Diagnostic_Severity::warning,                                             \
      {                                                                         \
        Source_Code_Span unexpected_space;                                      \
        Source_Code_Span bang;                                                  \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("unexpected whitespace between '!' and '=='"),  \
              unexpected_space)                                                 \
          MESSAGE(QLJS_TRANSLATABLE("'!' here treated as the TypeScript "       \
                                    "non-null assertion operator"),             \
                  bang))                                                        \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Unexpected_Space_Between_Bang_And_Equal_Equal, "E0374",              \
      Diagnostic_Severity::error, { Source_Code_Span unexpected_space; },       \
      MESSAGE(QLJS_TRANSLATABLE("unexpected whitespace between '!' and '=='"),  \
              unexpected_space))                                                \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_JSX_Prop_Is_Missing_Expression, "E0376",                             \
      Diagnostic_Severity::error,                                               \
      { Source_Code_Span left_brace_to_right_brace; },                          \
      MESSAGE(QLJS_TRANSLATABLE("JSX prop is missing an expression"),           \
              left_brace_to_right_brace))                                       \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Keyword_Contains_Escape_Characters, "E0381",                         \
      Diagnostic_Severity::error,                                               \
      { Source_Code_Span escape_character_in_keyword; },                        \
      MESSAGE(QLJS_TRANSLATABLE("Keywords in TypeScript does not allow"         \
                                " escape characters"),                          \
              escape_character_in_keyword))                                     \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Access_Specifier_Must_Precede_Other_Modifiers, "E0380",              \
      Diagnostic_Severity::error,                                               \
      {                                                                         \
        Source_Code_Span second_modifier;                                       \
        Source_Code_Span first_modifier;                                        \
      },                                                                        \
      MESSAGE(QLJS_TRANSLATABLE("'{0}' access specifier must precede "          \
                                "'{1}'"),                                       \
              second_modifier, first_modifier))                                 \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Spread_Must_Precede_Expression, "E0708",                             \
      Diagnostic_Severity::error, { Source_Code_Span spread; },                 \
      MESSAGE(QLJS_TRANSLATABLE("unexpected '...'; expected expression"),       \
              spread))                                                          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Spread_Must_Precede_Variable_Name, "E0709",                          \
      Diagnostic_Severity::error, { Source_Code_Span spread; },                 \
      MESSAGE(QLJS_TRANSLATABLE("expected variable name after '...'"),          \
              spread))                                                          \
                                                                                \
  QLJS_DIAG_TYPE(                                                               \
      Diag_Variable_Assigned_To_Self_Is_Noop, "E0383",                          \
      Diagnostic_Severity::warning,                                             \
      { Source_Code_Span assignment_statement; },                               \
      MESSAGE(QLJS_TRANSLATABLE("variable assignment to self is no-op"),        \
              assignment_statement))                                            \
                                                                                \
  /* END */

// QLJS_X_RESERVED_DIAG_TYPES lists reserved error codes. These codes were used
// in the past but no longer mean anything.
//
// When removing a diagnostic from QLJS_X_DIAG_TYPES, add an entry to
// QLJS_X_RESERVED_DIAG_TYPES.
//
// QLJS_DIAG_TYPE should have the following signature:
//
// #define QLJS_DIAG_TYPE(error_name, error_code, severity, struct_body,
// format) ...
//
// * error_name: (unset)
// * error_code: string literal
// * severity: (unset)
// * struct_body: (unset)
// * format: (unset)
#define QLJS_X_RESERVED_DIAG_TYPES \
  QLJS_DIAG_TYPE(, "E0242", , , )  \
  QLJS_DIAG_TYPE(, "E0271", , , )  \
  QLJS_DIAG_TYPE(, "E0279", , , )  \
  QLJS_DIAG_TYPE(, "E0707", , , )  \
  /* END */

namespace quick_lint_js {
#define QLJS_DIAG_TYPE(name, code, severity, struct_body, format_call) \
  struct name struct_body;
QLJS_X_DIAG_TYPES
#undef QLJS_DIAG_TYPE

// NOTE(strager): Enum members in Diag_Type are Upper_Snake_Case (matching the
// type names) instead of the usual lower_snake_case.
enum class Diag_Type {
#define QLJS_DIAG_TYPE(name, code, severity, struct_body, format_call) name,
  QLJS_X_DIAG_TYPES
#undef QLJS_DIAG_TYPE
};

std::ostream& operator<<(std::ostream&, Diag_Type);

template <class Error>
struct Diag_Type_From_Type_Detail;

#define QLJS_DIAG_TYPE(name, code, severity, struct_body, format_call) \
  template <>                                                          \
  struct Diag_Type_From_Type_Detail<name> {                            \
    static constexpr Diag_Type type = Diag_Type::name;                 \
  };
QLJS_X_DIAG_TYPES
#undef QLJS_DIAG_TYPE

template <class Error>
inline constexpr Diag_Type Diag_Type_From_Type =
    Diag_Type_From_Type_Detail<Error>::type;

inline constexpr int Diag_Type_Count = 0
#define QLJS_DIAG_TYPE(name, code, severity, struct_body, format_call) +1
    QLJS_X_DIAG_TYPES
#undef QLJS_DIAG_TYPE
    ;
}

#endif

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
