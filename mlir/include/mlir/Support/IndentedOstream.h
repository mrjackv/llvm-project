//===- IndentedOstream.h - raw ostream wrapper to indent --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// raw_ostream subclass that keeps track of indentation for textual output
// where indentation helps readability.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_SUPPORT_INDENTEDOSTREAM_H_
#define MLIR_SUPPORT_INDENTEDOSTREAM_H_

#include "mlir/Support/LLVM.h"
#include "llvm/Support/raw_ostream.h"

namespace mlir {

/// raw_ostream subclass that simplifies indention a sequence of code.
class raw_indented_ostream : public raw_ostream {
public:
  explicit raw_indented_ostream(llvm::raw_ostream &os, int indentSize = 2) : indentSize(indentSize), os(os) {
    SetUnbuffered();
  }

  /// Simple RAII struct to use to indentation around entering/exiting region.
  struct DelimitedScope {
    explicit DelimitedScope(raw_indented_ostream &os, StringRef open = "",
                            StringRef close = "", bool indent = true)
        : os(os), open(open), close(close), indent(indent) {
      os << open;
      if (indent)
        os.indent();
    }
    ~DelimitedScope() {
      if (indent)
        os.unindent();
      os << close;
    }

    raw_indented_ostream &os;

  private:
    StringRef open, close;
    bool indent;
  };

  /// Returns the underlying (unindented) raw_ostream.
  raw_ostream &getOStream() const { return os; }

  /// Returns DelimitedScope.
  DelimitedScope scope(StringRef open = "", StringRef close = "",
                       bool indent = true) {
    return DelimitedScope(*this, open, close, indent);
  }

  /// Prints a string re-indented to the current indent. Re-indents by removing
  /// the leading whitespace from the first non-empty line from every line of
  /// the string, skipping over empty lines at the start. Prefixes each line
  /// with extraPrefix after the indentation.
  raw_indented_ostream &printReindented(StringRef str,
                                        StringRef extraPrefix = "");

  /// Increases the indent and returning this raw_indented_ostream.
  raw_indented_ostream &indent() {
    currentIndent += indentSize;
    return *this;
  }

  /// Decreases the indent and returning this raw_indented_ostream.
  raw_indented_ostream &unindent() {
    currentIndent = std::max(0, currentIndent - indentSize);
    return *this;
  }

  /// Emits whitespace and sets the indentation for the stream.
  raw_indented_ostream &indent(int with) {
    os.indent(with);
    atStartOfLine = false;
    currentIndent = with;
    return *this;
  }

private:
  void write_impl(const char *ptr, size_t size) override;

  /// Return the current position within the stream, not counting the bytes
  /// currently in the buffer.
  uint64_t current_pos() const override { return os.tell(); }

  /// Constant indent added/removed.
  int indentSize;

  /// Tracker for current indentation.
  int currentIndent = 0;

  /// The leading whitespace of the string being printed, if reindent is used.
  int leadingWs = 0;

  /// The extra prefix to be printed, if reindent is used.
  StringRef currentExtraPrefix;

  /// Tracks whether at start of line and so indent is required or not.
  bool atStartOfLine = true;

  /// The underlying raw_ostream.
  raw_ostream &os;
};

} // namespace mlir
#endif // MLIR_SUPPORT_INDENTEDOSTREAM_H_
