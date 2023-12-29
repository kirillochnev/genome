#include "macro_callback.hpp"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Tooling/Tooling.h"
#include "context.hpp"
#include <string>
#include <iostream>

using namespace clang;


namespace {
    constexpr auto function_like_marco_begin = tok::l_paren;
    constexpr auto function_like_marco_end = tok::r_paren;
    const std::string reflect_macro_name = "EXTRACT_GENOME";
    struct AttrReflect {
        bool withNonPublic = false;
        bool withBase = false;
    };
    std::string toStr(const llvm::StringRef& from) {
        std::string result;
        result.resize(from.size());
        memcpy(result.data(), from.data(), result.size());
        return result;
    }
}

MacroCallback::MacroCallback(SourceManager& sm, LangOptions& opts, Context* ctx):
    _sm(sm),
    _opts(opts),
    context_(ctx) {
}

void MacroCallback::MacroExpands(const Token& token, const MacroDefinition&,
                                 clang::SourceRange, const clang::MacroArgs*) {

    if (_sm.isInSystemHeader(token.getLocation())) {
        return;
    }

    auto text = toStr(spell(&token, _sm));
    if (reflect_macro_name != text) {
        return;
    }
    //std::cout << "Find token [" << reflect_macro_name << "], text is: [" << text << "] looking for ')'" << std::endl;
    auto t = token;

        AttrReflect attr;

        while (true) {
            auto t_opt = Lexer::findNextToken(t.getLocation(), _sm, _opts);
            t = t_opt.value();
            std::string spelling = toStr(spell(&t, _sm));

            if (t.getKind() == function_like_marco_end) {
                //std::cout << ") [" << spelling <<  "] found! looking for class name!" << std::endl;
                break;
            }
            //std::cout << "skipping tokent [" << spelling << "]" << std::endl;

//            if (!(t.getKind() == tok::raw_identifier || t.getKind() == tok::string_literal)) {
//                continue;
//            }
//
//            auto spelling = spell(&t, _sm);
//            if (spelling == "WithNonPublic") {
//                attr.withNonPublic = true;
//            } else if (spelling == "WithBase") {
//                attr.withBase = true;
//            }
        }
        std::string spelling;
        do {
            //if (!spelling.empty()) {
            //    std::cout << "skipping tokent [" << spelling << "]" << std::endl;
            //}

            // skip ')' and keywords 'struct' 'class' or 'enum'
            auto t_opt = Lexer::findNextToken(t.getLocation(), _sm, _opts);
            t = t_opt.value();
            spelling = toStr(spell(&t, _sm));

        } while (spelling == "struct" || spelling == "class" || spelling == "enum" || spelling == "const");

        //std::cout << "class name was found! [" << spelling << "]\n\n\n" << std::endl;
        const auto location = t.getLocation();
        auto& request = context_->requests[location.printToString(_sm)];
        request.name = spelling;
        request.file = _sm.getFilename(location);
//        _ctx->reflect_map[_sm.getFileOffset(t.getLocation())] = attr;

    /* else if (text == "ER_ALIAS") {
        std::string alias;

        // skip '('
        auto t_opt = Lexer::findNextToken(t.getLocation(), _sm, _opts);
        // get the alias token
        t_opt = Lexer::findNextToken(t_opt.getValue().getLocation(), _sm, _opts);
        t = t_opt.getValue();
        alias = spell(&t, _sm).str();
        // skip ')'
        t_opt = Lexer::findNextToken(t.getLocation(), _sm, _opts);
        t = t_opt.getValue();

        auto t_origin = t;
        while (true) {
            // skip all type parts
            t_opt = Lexer::findNextToken(t.getLocation(), _sm, _opts);
            t = t_opt.getValue();

            auto k = t.getKind();
            if (k == tok::coloncolon || k == tok::less || k == tok::greater) {
                continue;
            }

            if (k == tok::raw_identifier) {
                t_origin = t;
            } else {
                break;
            }
        }

        _ctx->alias_map[_sm.getFileOffset(t_origin.getLocation())] = alias;

    } else if (text == "ER_EXCLUDE") {

        auto t_origin = t;
        while (true) {
            // skip all type parts
            auto t_opt = Lexer::findNextToken(t.getLocation(), _sm, _opts);
            t = t_opt.getValue();

            auto k = t.getKind();
            if (k == tok::coloncolon || k == tok::less || k == tok::greater) {
                continue;
            }

            if (k == tok::raw_identifier) {
                t_origin = t;
            } else {
                break;
            }
        }

        _ctx->excludes.insert(_sm.getFileOffset(t_origin.getLocation()));
    }*/
}

StringRef MacroCallback::spell(SourceRange range, const SourceManager& sm) {
    return Lexer::getSourceText(CharSourceRange::getTokenRange(range), sm, LangOptions(), nullptr);
}

StringRef MacroCallback::spell(const Token* token, const SourceManager& sm) {
    SourceLocation begin;
    SourceLocation end;

    if (tok::isStringLiteral(token->getKind())) {
        begin = token->getLocation().getLocWithOffset(1);
        end = token->getEndLoc().getLocWithOffset(-2);
    } else {
        begin = token->getLocation();
        end = token->getEndLoc().getLocWithOffset(-1);
    }

    return spell(SourceRange(begin, end), sm);
}

