#include "ast_callback.hpp"
#include "context.hpp"
#include <iostream>

using namespace clang;
using namespace ast_matchers;


namespace {
    std::string toString(AccessSpecifier access) {
        switch (access) {
            case clang::AS_public:
                return "kPublic";
            case clang::AS_protected:
                return "kProtected";
            case clang::AS_private:
                return "kPrivate";
            default:
                return "kNone";
        }
    }
    auto getKey(const CXXRecordDecl* c, const SourceManager& sm) {
        return c->getLocation().printToString(sm);
    }

    void handleClass(Context& ctx, const CXXRecordDecl* c, const Context::ReflectRequest* request,
                     const SourceManager& sm, const LangOptions& opts) {
        const auto key = getKey(c, sm);
        if (request == nullptr) {
            auto it = ctx.requests.find(key);// ctx.requests.find(getLocation(c, sm, opts));
            if (it == ctx.requests.end()) {
                return;
            }
            request = &it->second;
        }
 
        const auto qualified_name = c->getQualifiedNameAsString();
        if (ctx.result.find(qualified_name) != ctx.result.end()) {
            return;
        }
        auto& class_info = ctx.result[qualified_name];
        class_info.file = request->file;
        class_info.name = qualified_name;
        if (c->getNameAsString() != request->name) {
            std::cout << "qualified name of [" << request->name << "] is [" << qualified_name << "]"
                << "|            [" << c->getNameAsString() << "]" << std::endl;
        }
        for (auto&& decl : c->getPrimaryContext()->decls()) {
            if (const auto* field_in = dyn_cast<FieldDecl>(decl)) {
                auto& field_out = class_info.fields.emplace_back();
                field_out.name = field_in->getName();
                field_out.access = toString(field_in->getAccess());
                continue;
            }

            if (const auto* nested_class = dyn_cast<CXXRecordDecl>(decl)) {
                if (nested_class->isThisDeclarationADefinition()) {
                    std::cout << "--------------  Handling nested class -----------------" << std::endl;
                    handleClass(ctx, nested_class, request, sm, opts);
                    std::cout << "--------------  End of handling nested class -----------------" << std::endl;

                }
            }
        }
    }

    void handleEnum(const EnumDecl* e, Context::ClassInfo* class_info, const SourceManager& sm, const LangOptions& opts) {

    }
}


void AstCallback::run(const MatchFinder::MatchResult& result) {
    if (const auto* c = result.Nodes.getNodeAs<CXXRecordDecl>("c")) {
        handleClass(*_ctx, c, nullptr, *result.SourceManager, result.Context->getLangOpts());
    }
    if (const auto* e = result.Nodes.getNodeAs<EnumDecl>("e")) {
        handleEnum(e, nullptr, *result.SourceManager, result.Context->getLangOpts());
    }
}
