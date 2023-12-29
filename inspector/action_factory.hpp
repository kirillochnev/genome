#pragma once

#include <memory>
#include <clang/Tooling/Tooling.h>

struct Context;

std::unique_ptr<clang::tooling::FrontendActionFactory> newFrontendActionFactory(Context* context);
