#pragma once

#include <clang/Lex/Lexer.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Tooling/Tooling.h>

struct Context;

class MacroCallback : public clang::PPCallbacks {
public:

    explicit MacroCallback(clang::SourceManager& sm, clang::LangOptions& opts, Context* ctx);

    void MacroExpands(const clang::Token& token, const clang::MacroDefinition&, clang::SourceRange,
                      const clang::MacroArgs*) final;

private:
    static clang::StringRef spell(clang::SourceRange range, const clang::SourceManager& sm);
    static clang::StringRef spell(const clang::Token* token, const clang::SourceManager& sm);

    clang::SourceManager& _sm;
    clang::LangOptions& _opts;
    Context* context_;
};
