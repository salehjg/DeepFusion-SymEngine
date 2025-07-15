//
// Created by saleh on 7/15/25.
//

#include "modulo_visitor.h"

SymEngine::RCP<const SymEngine::Basic>
SymEngine::ModuloVisitor::Apply(const SymEngine::Basic &b)
{
    m_mReplacements.clear();
    b.accept(*this);
    return b.xreplace(m_mReplacements);
}

void SymEngine::ModuloVisitor::bvisit(const FunctionSymbol &x)
{
    if (x.get_name() == "%") {
        if (x.get_args().size() == 2) {
            auto args = x.get_args();
            try {
                auto nVal0 = static_cast<long>(eval_double(*args[0]));
                auto nVal1 = static_cast<long>(eval_double(*args[1]));
                m_mReplacements[x.rcp_from_this()] = integer(nVal0 % nVal1);
            } catch (const SymEngineException &e) {
                return;
            }
        }
    }
}

void SymEngine::ModuloVisitor::bvisit(const SymEngine::Basic &x)
{
    for (const auto &arg : x.get_args()) {
        arg->accept(*this);
    }
}