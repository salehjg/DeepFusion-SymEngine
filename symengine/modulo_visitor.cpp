//
// Created by saleh on 7/15/25.
//

#include "modulo_visitor.h"

SymEngine::RCP<const SymEngine::Basic> SymEngine::ModuloVisitor::Apply(
    const SymEngine::Basic &b)
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
            bool isNum0 = args[0]->get_type_code() == SymEngine::TypeID::SYMENGINE_INTEGER;
            bool isNum1 = args[1]->get_type_code() == SymEngine::TypeID::SYMENGINE_INTEGER;
            if (isNum0 && isNum1) {
                auto nVal0 = down_cast<const Integer &>(*args[0]).as_int();
                auto nVal1 = down_cast<const Integer &>(*args[1]).as_int();
                m_mReplacements[x.rcp_from_this()] = integer(nVal0 % nVal1);
            }
        }
    }
}

void SymEngine::ModuloVisitor::bvisit(const SymEngine::Basic &x)
{
    for (const auto &arg: x.get_args()) {
        arg->accept(*this);
    }
}