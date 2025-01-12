// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#include <array>
#include <cstdint>
#include <cstring>
#include <quick-lint-js/assert.h>
#include <quick-lint-js/diag/diagnostic-types.h>
#include <quick-lint-js/diag/diagnostic.h>
#include <quick-lint-js/fe/lex.h>
#include <quick-lint-js/port/char8.h>
#include <quick-lint-js/port/warning.h>
#include <quick-lint-js/util/cpp.h>
#include <quick-lint-js/util/narrow-cast.h>
#include <string_view>
#include <utility>

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER <= 192829337
// Work around the following false compilation error in MSVC version 19.28.29337
// (aka 14.28-16.8 aka 14.28.29333) and older:
//
// error C2131: expression did not evaluate to a constant
// message: failure was caused by out of range index 3; allowed range is 0 <=
//          index < 2
// message: while evaluating constexpr function
//          diagnostic_info_builder<Diag_Adjacent_JSX_Without_Parent>::add
// message: while evaluating constexpr function
//          info_for_diagnostic<Diag_Adjacent_JSX_Without_Parent>::get
#define DIAGNOSTIC_CONSTEXPR_IF_POSSIBLE /* */
#else
#define DIAGNOSTIC_CONSTEXPR_IF_POSSIBLE constexpr
#endif

namespace quick_lint_js {
namespace {
constexpr std::uint16_t parse_code_string(const char* code_string) noexcept {
  QLJS_CONSTEXPR_ASSERT(code_string[0] == 'E');
  QLJS_CONSTEXPR_ASSERT('0' <= code_string[1] && code_string[1] <= '9');
  QLJS_CONSTEXPR_ASSERT('0' <= code_string[2] && code_string[2] <= '9');
  QLJS_CONSTEXPR_ASSERT('0' <= code_string[3] && code_string[3] <= '9');
  QLJS_CONSTEXPR_ASSERT('0' <= code_string[4] && code_string[4] <= '9');
  QLJS_CONSTEXPR_ASSERT(code_string[5] == '\0');
  return static_cast<std::uint16_t>((code_string[1] - '0') * 1000 +  //
                                    (code_string[2] - '0') * 100 +   //
                                    (code_string[3] - '0') * 10 +    //
                                    (code_string[4] - '0') * 1);
}

std::array<char, 5> diag_code_to_string(std::uint16_t diag_code) noexcept {
  QLJS_ASSERT(diag_code <= 9999);
  return std::array<char, 5>{
      'E',
      static_cast<char>('0' + ((diag_code / 1000) % 10)),
      static_cast<char>('0' + ((diag_code / 100) % 10)),
      static_cast<char>('0' + ((diag_code / 10) % 10)),
      static_cast<char>('0' + ((diag_code / 1) % 10)),
  };
}

// Convert a QLJS_DIAG_TYPE user into a diagnostic_info.
class Diagnostic_Info_Builder {
 public:
  QLJS_WARNING_PUSH
  QLJS_WARNING_IGNORE_GCC("-Wconversion")
  constexpr explicit Diagnostic_Info_Builder(const char* code_string,
                                             Diagnostic_Severity sev) {
    this->info_.severity = sev;
    this->info_.code = parse_code_string(code_string);
  }
  QLJS_WARNING_POP

  // Each of Args must be a diagnostic_message_arg_info.
  template <class... Args>
  constexpr Diagnostic_Info_Builder add(const Translatable_Message& message,
                                        const Args&... arg_infos) {
    this->info_.message_formats[this->current_message_index_] = message;

    std::size_t current_arg_index = 0;
    Diagnostic_Message_Args& args =
        this->info_.message_args[this->current_message_index_];
    ((args[current_arg_index++] = arg_infos), ...);

    ++this->current_message_index_;
    return *this;
  }

  constexpr Diagnostic_Info build() noexcept { return this->info_; }

 private:
  Diagnostic_Info info_{};
  int current_message_index_ = 0;
};

template <class Diag>
struct Info_For_Diagnostic;

#define MAKE_ARGS(...) MAKE_ARGS_N(QLJS_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
#define MAKE_ARGS_N(...) MAKE_ARGS_N_(__VA_ARGS__)
#define MAKE_ARGS_N_(count, ...) MAKE_ARGS_##count(__VA_ARGS__)

#define MAKE_ARGS_1(arg0)         \
  Diagnostic_Message_Arg_Info(    \
      offsetof(Diag_Class, arg0), \
      get_diagnostic_message_arg_type<decltype(Diag_Class::arg0)>())
#define MAKE_ARGS_2(arg0, arg1) MAKE_ARGS_1(arg0), MAKE_ARGS_1(arg1)
#define MAKE_ARGS_3(arg0, arg1, arg2) MAKE_ARGS_2(arg0, arg1), MAKE_ARGS_1(arg2)

#define MESSAGE(message_format, ...) \
  .add(message_format, MAKE_ARGS(__VA_ARGS__))

#define QLJS_DIAG_TYPE(name, code, severity, struct_body, format_call)       \
  template <>                                                                \
  struct Info_For_Diagnostic<name> {                                         \
    using Diag_Class = name;                                                 \
                                                                             \
    static DIAGNOSTIC_CONSTEXPR_IF_POSSIBLE Diagnostic_Info get() noexcept { \
      return Diagnostic_Info_Builder(code, severity) format_call.build();    \
    }                                                                        \
  };
QLJS_X_DIAG_TYPES
#undef QLJS_DIAG_TYPE
}

// If you see an error with the following lines, translation-table-generated.h
// is probably out of date. Run tools/update-translator-sources to rebuild this
// file.
DIAGNOSTIC_CONSTEXPR_IF_POSSIBLE const Diagnostic_Info
    all_diagnostic_infos[] = {
#define QLJS_DIAG_TYPE(name, code, severity, struct_body, format_call) \
  Info_For_Diagnostic<name>::get(),
        QLJS_X_DIAG_TYPES
#undef QLJS_DIAG_TYPE
};

const Diagnostic_Info& get_diagnostic_info(Diag_Type type) noexcept {
  return all_diagnostic_infos[static_cast<std::ptrdiff_t>(type)];
}

std::array<char, 5> Diagnostic_Info::code_string() const noexcept {
  return diag_code_to_string(this->code);
}

QLJS_WARNING_PUSH
// GCC thinks that all_diagnostic_infos[i].code is not null-terminated, but it
// is.
QLJS_WARNING_IGNORE_GCC("-Wstringop-overflow")

std::optional<Diag_Type> diag_type_from_code_slow(
    std::string_view code) noexcept {
  for (int i = 0; i < Diag_Type_Count; ++i) {
    // TODO(strager): Parse the incoming code instead of stringifying each code
    // in the table.
    auto diag_code_string = all_diagnostic_infos[i].code_string();
    std::string_view diag_code_string_view(diag_code_string.data(),
                                           diag_code_string.size());
    if (diag_code_string_view == code) {
      return static_cast<Diag_Type>(i);
    }
  }
  return std::nullopt;
}

QLJS_WARNING_POP
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
