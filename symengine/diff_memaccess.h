//
// Created by saleh on 7/12/25.
//

#pragma once

#include <symengine/basic.h>
#include <symengine/symbol.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/derivative.h>
#include "functions.h" // for MemAccess

using namespace SymEngine;

class DiffVisitor2 : public BaseVisitor<DiffVisitor2>
{
    // The MemAccess to differentiate with respect to
    RCP<const MemAccess> m_pWrt;
    using VecMatchesMap = std::vector<map_basic_basic>;
    using DetailedExprs = std::vector<std::pair<
        RCP<const Basic>, VecMatchesMap>>;
    DetailedExprs m_oResult_;
    int m_iNestingLevel = 0;
    vec_sym m_vStackSymIndices;
    std::vector<size_t> m_vStackLowerBonds;
    std::vector<size_t> m_vStackUpperBonds;
    const bool m_bDebug;
    const bool m_bResolveModulos;

public:
    DiffVisitor2(const RCP<const MemAccess> &wrt_, bool resolveModulos,
                 bool debug = false)
        : m_pWrt(wrt_), m_bResolveModulos(resolveModulos), m_bDebug(debug)
    {
    }

    RCP<const Basic> Apply(RCP<const Basic> &b)
    {
        m_oResult_.clear();
        m_iNestingLevel = 0;
        m_vStackSymIndices.clear();
        m_vStackLowerBonds.clear();
        m_vStackUpperBonds.clear();
        b->accept(*this);
        if (m_oResult_.empty()) {
            throw SymEngineException(
                "DiffVisitor2: There is no piecewise expr in the derivative!"
            );
        }
        if (m_oResult_.size() > 1) {
            throw SymEngineException(
                "DiffVisitor2: There are more than one piecewise expr in the derivative!"
            );
        }
        return m_oResult_[0].first; // there is one piece, return its expr.
    }

    void bvisit(const Add &self);
    void bvisit(const Mul &self);
    void bvisit(const Pow &self);
    void bvisit(const Derivative &self);
    void bvisit(const MemAccess &self);
    void bvisit(const FunctionSymbol &self);
    void bvisit(const Basic &self);

private:
    void DynamicLoop(
        const std::vector<size_t> &lows,
        const std::vector<size_t> &highs,
        // lambda of the job to run for each combination
        const std::function<void(const std::vector<size_t> &)> &job
    );
    __always_inline std::string Indent();
    __always_inline void PrintDetailedExprs(
        const DetailedExprs &de,
        const std::string &msg
    );
    __always_inline DetailedExprs AggrigateTwoDetailedExprs(
        const DetailedExprs &de1,
        const DetailedExprs &de2,
        bool isAdd,
        bool isMul
    );
};