// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#include <cstddef>
#include <gmock/gmock.h>
#include <optional>
#include <quick-lint-js/cli/cli-location.h>
#include <quick-lint-js/container/padded-string.h>
#include <quick-lint-js/diag-collector.h>
#include <quick-lint-js/diag-matcher.h>
#include <quick-lint-js/diag/diagnostic-types.h>
#include <quick-lint-js/fe/lex.h>
#include <quick-lint-js/fe/source-code-span.h>
#include <quick-lint-js/port/unreachable.h>
#include <vector>

namespace quick_lint_js {
class Offsets_Matcher::Span_Impl
    : public testing::MatcherInterface<const Source_Code_Span &> {
 public:
  explicit Span_Impl(Padded_String_View code,
                     CLI_Source_Position::Offset_Type begin_offset,
                     CLI_Source_Position::Offset_Type end_offset)
      : code_(code), begin_offset_(begin_offset), end_offset_(end_offset) {}

  void DescribeTo(std::ostream *out) const override {
    *out << "has begin-end offset " << this->begin_offset_ << '-'
         << this->end_offset_;
  }

  void DescribeNegationTo(std::ostream *out) const override {
    *out << "doesn't have begin-end offset " << this->begin_offset_ << '-'
         << this->end_offset_;
  }

  bool MatchAndExplain(const Source_Code_Span &span,
                       testing::MatchResultListener *listener) const override {
    auto span_begin_offset = narrow_cast<CLI_Source_Position::Offset_Type>(
        span.begin() - this->code_.data());
    auto span_end_offset = narrow_cast<CLI_Source_Position::Offset_Type>(
        span.end() - this->code_.data());
    bool result = span_begin_offset == this->begin_offset_ &&
                  span_end_offset == this->end_offset_;
    *listener << "whose begin-end offset (" << span_begin_offset << '-'
              << span_end_offset << ") "
              << (result ? "equals" : "doesn't equal") << " "
              << this->begin_offset_ << '-' << this->end_offset_;
    return result;
  }

 private:
  Padded_String_View code_;
  CLI_Source_Position::Offset_Type begin_offset_;
  CLI_Source_Position::Offset_Type end_offset_;
};

Offsets_Matcher::Offsets_Matcher(Padded_String_View input,
                                 CLI_Source_Position::Offset_Type begin_offset,
                                 CLI_Source_Position::Offset_Type end_offset)
    : code_(input), begin_offset_(begin_offset), end_offset_(end_offset) {}

Offsets_Matcher::Offsets_Matcher(Padded_String_View input,
                                 CLI_Source_Position::Offset_Type begin_offset,
                                 String8_View text)
    : code_(input),
      begin_offset_(begin_offset),
      end_offset_(begin_offset + text.size()) {}

Offsets_Matcher::Offsets_Matcher(Offsets_Matcher &&) = default;

Offsets_Matcher &Offsets_Matcher::operator=(Offsets_Matcher &&) = default;

Offsets_Matcher::~Offsets_Matcher() = default;

/*implicit*/ Offsets_Matcher::operator testing::Matcher<
    const Source_Code_Span &>() const {
  return testing::Matcher<const Source_Code_Span &>(
      new Span_Impl(this->code_, this->begin_offset_, this->end_offset_));
}

class Span_Matcher::Span_Impl
    : public testing::MatcherInterface<const Source_Code_Span &> {
 public:
  explicit Span_Impl(const Char8 *expected) : expected_(expected) {}

  void DescribeTo(std::ostream *out) const override {
    *out << "begins at " << static_cast<const void *>(this->expected_);
  }

  void DescribeNegationTo(std::ostream *out) const override {
    *out << "doesn't begin at " << static_cast<const void *>(this->expected_);
  }

  bool MatchAndExplain(const Source_Code_Span &span,
                       testing::MatchResultListener *listener) const override {
    bool result = span.begin() == this->expected_;
    *listener << "whose span (at " << static_cast<const void *>(span.begin())
              << ") " << (result ? "begins" : "doesn't begin") << " at "
              << static_cast<const void *>(this->expected_);
    return result;
  }

 private:
  const Char8 *expected_;
};

Span_Matcher::Span_Matcher(const Char8 *expected) : expected_(expected) {}

Span_Matcher::operator testing::Matcher<const Source_Code_Span &>() const {
  return testing::Matcher<const Source_Code_Span &>(
      new Span_Impl(this->expected_));
}

class Source_Code_Span_Matcher::Span_Impl
    : public testing::MatcherInterface<const Source_Code_Span &> {
 public:
  explicit Span_Impl(Source_Code_Span expected) : expected_(expected) {}

  void DescribeTo(std::ostream *out) const override {
    *out << "begins at " << static_cast<const void *>(this->expected_.begin())
         << " and ends at " << static_cast<const void *>(this->expected_.end());
  }

  void DescribeNegationTo(std::ostream *out) const override {
    *out << "doesn't begin at "
         << static_cast<const void *>(this->expected_.begin()) << " and end at "
         << static_cast<const void *>(this->expected_.end());
  }

  bool MatchAndExplain(const Source_Code_Span &span,
                       testing::MatchResultListener *listener) const override {
    bool result = same_pointers(span, this->expected_);
    *listener << "whose span (from " << static_cast<const void *>(span.begin())
              << " to " << static_cast<const void *>(span.end()) << ") "
              << (result ? "equals" : "doesn't equal") << " expected (from "
              << static_cast<const void *>(this->expected_.begin()) << " to "
              << static_cast<const void *>(this->expected_.begin()) << ")";
    return result;
  }

 private:
  Source_Code_Span expected_;
};

Source_Code_Span_Matcher::Source_Code_Span_Matcher(Source_Code_Span expected)
    : expected_(expected) {}

Source_Code_Span_Matcher::operator testing::Matcher<const Source_Code_Span &>()
    const {
  return testing::Matcher<const Source_Code_Span &>(
      new Span_Impl(this->expected_));
}

Source_Code_Span Diag_Matcher_Arg::get_span(const void *error_object) const
    noexcept {
  const void *member_data =
      reinterpret_cast<const char *>(error_object) + this->member_offset;
  switch (this->member_type) {
  case Diagnostic_Arg_Type::Source_Code_Span:
    return *static_cast<const Source_Code_Span *>(member_data);

  case Diagnostic_Arg_Type::Char8:
  case Diagnostic_Arg_Type::enum_kind:
  case Diagnostic_Arg_Type::invalid:
  case Diagnostic_Arg_Type::statement_kind:
  case Diagnostic_Arg_Type::String8_View:
  case Diagnostic_Arg_Type::variable_kind:
    QLJS_ASSERT(false && "invalid arg type");
    break;
  }
  QLJS_UNREACHABLE();
}

template <class State, class Field>
class Diag_Fields_Matcher_Impl_Base
    : public testing::MatcherInterface<const Diag_Collector::Diag &> {
 public:
  explicit Diag_Fields_Matcher_Impl_Base(State s) : state_(std::move(s)) {}

  void DescribeTo(std::ostream *out) const override {
    *out << "has type " << this->state_.type;
    this->describe_fields_to(out);
  }

  void DescribeNegationTo(std::ostream *out) const override {
    *out << "doesn't have type " << this->state_.type;
    this->describe_fields_to(out);
  }

  void describe_fields_to(std::ostream *) const {
    // TODO(strager)
  }

  bool MatchAndExplain(const Diag_Collector::Diag &error,
                       testing::MatchResultListener *listener) const override {
    bool type_matches = error.type() == this->state_.type;
    if (!type_matches) {
      *listener << "whose type (" << error.type() << ") isn't "
                << this->state_.type;
      return false;
    }

    bool result = true;
    bool is_first_field = true;
    for (const Field &f : this->state_.fields) {
      if (!is_first_field) {
        *listener << " and ";
      }
      bool matches = this->field_matches(error, f, listener);
      result = result && matches;
      is_first_field = false;
    }
    return result;
  }

 protected:
  virtual bool field_matches(const Diag_Collector::Diag &error, const Field &f,
                             testing::MatchResultListener *listener) const = 0;

  State state_;
};

Diag_Matcher::Diag_Matcher(Diag_Type type) : state_{type, std::nullopt, {}} {}

Diag_Matcher::Diag_Matcher(Padded_String_View input, Diag_Type type,
                           Field field_0)
    : state_{type, input, {field_0}} {}

Diag_Matcher::Diag_Matcher(Padded_String_View input, Diag_Type type,
                           Field field_0, Field field_1)
    : state_{type, input, {field_0, field_1}} {}

Diag_Matcher::Diag_Matcher(Padded_String_View input, Diag_Type type,
                           Field field_0, Field field_1, Field field_2)
    : state_{type, input, {field_0, field_1, field_2}} {}

class Diag_Matcher::Impl
    : public Diag_Fields_Matcher_Impl_Base<Diag_Matcher::State,
                                           Diag_Matcher::Field> {
 public:
  using Base =
      Diag_Fields_Matcher_Impl_Base<Diag_Matcher::State, Diag_Matcher::Field>;

  using Base::Diag_Fields_Matcher_Impl_Base;

 protected:
  bool field_matches(const Diag_Collector::Diag &error, const Field &f,
                     testing::MatchResultListener *listener) const override {
    QLJS_ASSERT(this->state_.input.has_value());
    Source_Code_Span span = f.arg.get_span(error.data());
    auto span_begin_offset = narrow_cast<CLI_Source_Position::Offset_Type>(
        span.begin() - this->state_.input->data());
    auto span_end_offset = narrow_cast<CLI_Source_Position::Offset_Type>(
        span.end() - this->state_.input->data());
    auto expected_end_offset = f.begin_offset + f.text.size();

    bool span_matches = span_begin_offset == f.begin_offset &&
                        span_end_offset == expected_end_offset;
    *listener << "whose ." << f.arg.member_name << " (" << span_begin_offset
              << "-" << span_end_offset << ") "
              << (span_matches ? "equals" : "doesn't equal") << " "
              << f.begin_offset << "-" << expected_end_offset;
    return span_matches;
  }
};

/*implicit*/ Diag_Matcher::operator testing::Matcher<
    const Diag_Collector::Diag &>() const {
  return testing::Matcher<const Diag_Collector::Diag &>(new Impl(this->state_));
}

Diag_Spans_Matcher::Diag_Spans_Matcher(Diag_Type type, Field field_0)
    : state_{type, {field_0}} {}

Diag_Spans_Matcher::Diag_Spans_Matcher(Diag_Type type, Field field_0,
                                       Field field_1)
    : state_{type, {field_0, field_1}} {}

class Diag_Spans_Matcher::Impl
    : public Diag_Fields_Matcher_Impl_Base<Diag_Spans_Matcher::State,
                                           Diag_Spans_Matcher::Field> {
 public:
  using Base = Diag_Fields_Matcher_Impl_Base<Diag_Spans_Matcher::State,
                                             Diag_Spans_Matcher::Field>;

  using Base::Diag_Fields_Matcher_Impl_Base;

 protected:
  bool field_matches(const Diag_Collector::Diag &error, const Field &f,
                     testing::MatchResultListener *listener) const override {
    Source_Code_Span span = f.arg.get_span(error.data());
    bool span_matches = same_pointers(span, f.expected);
    *listener << "whose ." << f.arg.member_name << " (`"
              << out_string8(span.string_view()) << "` @"
              << reinterpret_cast<const void *>(span.begin()) << ") "
              << (span_matches ? "equals" : "doesn't equal") << " `"
              << out_string8(f.expected.string_view()) << "` @"
              << reinterpret_cast<const void *>(f.expected.begin());
    return span_matches;
  }
};

/*implicit*/ Diag_Spans_Matcher::operator testing::Matcher<
    const Diag_Collector::Diag &>() const {
  return testing::Matcher<const Diag_Collector::Diag &>(new Impl(this->state_));
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
