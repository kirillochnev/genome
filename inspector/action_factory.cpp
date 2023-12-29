#include "ast_callback.hpp"
#include "action_factory.hpp"

#include <memory>

#include "macro_callback.hpp"
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

using namespace clang;
using namespace tooling;
using namespace ast_matchers;

namespace {

    class Action : public ASTFrontendAction {
    public:
        explicit Action(Context* ctx) : 
                _ast_callback(ctx),
                context_(ctx) {
                static const auto class_matcher = cxxRecordDecl(isDefinition(), unless(isExpansionInSystemHeader())).bind("c");
                finder_.addMatcher(class_matcher, &_ast_callback);
                static const auto enum_matcher = enumDecl(unless(isExpansionInSystemHeader())).bind("e");
                finder_.addMatcher(enum_matcher, &_ast_callback);
        }

        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& compiler, StringRef) override {
            // register macro handler
            auto callback = std::make_unique<MacroCallback>(compiler.getSourceManager(),
                                                            compiler.getLangOpts(), context_);
            compiler.getPreprocessor().addPPCallbacks(std::move(callback));
            // forward work to match finder
            return finder_.newASTConsumer();
        }

    private:
        MatchFinder finder_;
        AstCallback _ast_callback;
        Context* context_;
    };
}

std::unique_ptr<FrontendActionFactory> newFrontendActionFactory(Context* ctx) {
    class ActionFactory : public tooling::FrontendActionFactory {
    public:
        explicit ActionFactory(Context* ctx) :
            context_(ctx) {
        }

        std::unique_ptr<FrontendAction> create() override {
            return std::make_unique<Action>(context_);
        }

    private:
        Context* context_;
    };
    return std::make_unique<ActionFactory>(ctx);
}
