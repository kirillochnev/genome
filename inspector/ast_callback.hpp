#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

struct Context;

class AstCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    explicit AstCallback(Context* ctx) : _ctx(ctx) {
    }

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) final;

private:
    Context* _ctx;
};
