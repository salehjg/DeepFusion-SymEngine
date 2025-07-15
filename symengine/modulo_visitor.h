//
// Created by saleh on 7/15/25.
//

#pragma once

#include <symengine/basic.h>
#include <symengine/visitor.h>
#include <symengine/functions.h>

namespace SymEngine
{
class ModuloVisitor : public BaseVisitor<ModuloVisitor>
{
private:
    map_basic_basic m_mReplacements;

public:
    /**
     * Traverses the expression to resolve all instances of modulo function
     * with numeric arguments, like %(3,2) and NOT LIKE %(i0, 5), etc.
     * We assume that the modulo instances are FuncionSymbol("%)'s with two
     * args.
     * @param b The expr to visit.
     * @return The resolved expr.
     */
    SymEngine::RCP<const SymEngine::Basic> Apply(const SymEngine::Basic &b);

    void bvisit(const FunctionSymbol &x);

    void bvisit(const SymEngine::Basic &x);
};
} // namespace SymEngine
