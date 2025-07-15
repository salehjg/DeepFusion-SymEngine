//
// Created by saleh on 7/12/25.
//
#include "diff_memaccess.h"
#include "modulo_visitor.h"

void ::DiffVisitor2::bvisit(const Add &self)
{
    if (m_bDebug)
        std::cout << Indent() << "bvisit Add: " << self.__str__() << "\n";
    m_iNestingLevel++;
    DetailedExprs result;

    for (const auto &a : self.get_args()) {
        if (m_bDebug)
            std::cout << Indent() << "bvisit Add, accepting arg: " << a->
                __str__() << "\n";
        a->accept(*this);
        PrintDetailedExprs(m_oResult_, "Result after visiting arg");
        result = AggrigateTwoDetailedExprs(result, m_oResult_, true, false);
    }
    m_oResult_ = result;
    PrintDetailedExprs(m_oResult_, "Result after visiting Add args");
    m_iNestingLevel--;
}

void ::DiffVisitor2::bvisit(const Mul &self)
{
    if (m_bDebug)
        std::cout << Indent() << "bvisit Mul: " << self.__str__() << "\n";
    m_iNestingLevel++;
    const auto &args = self.get_args();
    DetailedExprs toBeSummed;
    for (size_t i = 0; i < args.size(); ++i) {
        DetailedExprs prod;
        for (size_t j = 0; j < args.size(); ++j) {
            if (i == j) {
                args[j]->accept(*this);
                if (m_bDebug)
                    std::cout << Indent() << "bvisit Mul, i=" << i << ", j=" <<
                        j << ", arg[j]: " << args[j]->__str__() << "\n";
                PrintDetailedExprs(m_oResult_, "Result after visiting arg[j]");
                prod = AggrigateTwoDetailedExprs(prod, m_oResult_, false, true);
            } else {
                if (m_bDebug)
                    std::cout << Indent() << "bvisit Mul, i=" << i << ", j=" <<
                        j << ", arg[j]: " << args[j]->__str__() << "\n";
                prod = AggrigateTwoDetailedExprs(
                    prod, {{args[j], {}}}, false, true);
            }
        }
        // If the differentiated term is zero, skip
        //if (eq(*prod[i], *zero))
        //    continue;
        toBeSummed = AggrigateTwoDetailedExprs(toBeSummed, prod, true, false);
    }
    m_oResult_ = toBeSummed;
    m_iNestingLevel--;
}

void ::DiffVisitor2::bvisit(const Pow &self)
{
    if (m_bDebug)
        std::cout << Indent() << "bvisit Pow: " << self.__str__() << "\n";
    m_iNestingLevel++;

    // d/dx (f^g) = f^(g-1) * (f' * g  +   g' * f * log(f))
    auto f = self.get_base();
    auto g = self.get_exp();

    // Differentiate f
    f->accept(*this);
    auto df = m_oResult_;

    // Differentiate g
    g->accept(*this);
    auto dg = m_oResult_;

    auto pow_fg = pow(f, sub(g, one)); // f^(g-1)

    // Check if all pieces in df and dg are zero
    bool df_zero = true, dg_zero = true;
    for (const auto &piece : df) {
        if (!eq(*piece.first, *zero)) {
            df_zero = false;
            break;
        }
    }
    for (const auto &piece : dg) {
        if (!eq(*piece.first, *zero)) {
            dg_zero = false;
            break;
        }
    }

    DetailedExprs term1, term2;

    // f^(g-1) * g' * f * log(f)
    if (!dg_zero) {
        for (const auto &piece : dg) {
            if (!eq(*piece.first, *zero)) {
                auto term = mul(mul(pow_fg, f), mul(piece.first, log(f)));
                term1.emplace_back(term, piece.second);
            }
        }
    }

    // f^(g-1) * g * f'
    if (!df_zero) {
        for (const auto &piece : df) {
            if (!eq(*piece.first, *zero)) {
                auto term = mul(pow_fg, mul(g, piece.first));
                term2.emplace_back(term, piece.second);
            }
        }
    }

    m_oResult_ = AggrigateTwoDetailedExprs(term1, term2, true, false);

    PrintDetailedExprs(m_oResult_, "Result after visiting Pow");
    m_iNestingLevel--;
}

void ::DiffVisitor2::bvisit(const Derivative &self)
{
    if (m_bDebug)
        std::cout << Indent() << "bvisit Derivative: " << self.__str__() <<
            "\n";
    m_iNestingLevel++;
    throw std::runtime_error("Not implemented");
    /*
    // If this is d/d(wrt) of Derivative(expr, wrt), return Derivative(expr, 2*wrt)
    auto arg = self.get_arg();
    auto symbols = self.get_symbols();
    bool found = false;
    for (auto &s : symbols) {
        if (eq(*s, *wrt)) {
            found = true;
            break;
        }
    }
    if (found) {
        multiset_basic newsyms = symbols;
        newsyms.insert(wrt);
        result_ = Derivative::create(arg, newsyms);
    } else {
        // Derivative is the derivative of the derivative
        arg->accept(*this);
        RCP<const Basic> t = result_;
        for (auto &s : symbols) {
            t = Derivative::create(t, {s});
        }
        result_ = t;
    }
    */
    m_iNestingLevel--;
}

void ::DiffVisitor2::bvisit(const MemAccess &self)
{
    if (m_bDebug)
        std::cout << Indent() << "bvisit MemAccess: " << self.__str__() << "\n";
    m_iNestingLevel++;
    auto selfArgs = self.get_args();
    auto wrtArgs = m_pWrt->get_args();
    if (self.get_tensor_id() != m_pWrt->get_tensor_id()) {
        m_oResult_ = {{zero, {}}};
        PrintDetailedExprs(m_oResult_, "Early exit1");
        m_iNestingLevel--;
        return;
    }
    if (selfArgs.size() != wrtArgs.size()) {
        m_oResult_ = {{zero, {}}};
        PrintDetailedExprs(m_oResult_, "Early exit2");
        m_iNestingLevel--;
        return;
    }
    for (size_t i = 0; i < selfArgs.size(); ++i) {
        if (is_a_Number(*selfArgs[i])) {
            if (!eq(*selfArgs[i], *wrtArgs[i])) {
                m_oResult_ = {{zero, {}}};
                PrintDetailedExprs(m_oResult_, "Early exit3");
                m_iNestingLevel--;
                return;
            }
        }
    }
    if (eq(self, *m_pWrt)) {
        m_oResult_ = {{one, {}}};
        PrintDetailedExprs(m_oResult_, "Early exit4");
        m_iNestingLevel--;
        return;
    }
    if (m_vStackSymIndices.empty()) {
        // No loop variables in the stack, so no matches possible
        m_oResult_ = {{zero, {}}};
        PrintDetailedExprs(m_oResult_, "Early exit5");
        m_iNestingLevel--;
        return;
    }

    // Find the used SymIndices in `self` that are in the stack
    vec_sym stackSyms;
    std::vector<size_t> stackLBs, stackUBs;
    for (const auto &arg__ : selfArgs) {
        auto freeArgs = free_symbols(*arg__);
        for (auto &arg : freeArgs) {
            if (is_a<Symbol>(*arg)) {
                auto pSym = rcp_static_cast<const Symbol>(arg);
                // find the pSym's index in the stack if it exists
                auto it = std::find_if(m_vStackSymIndices.begin(),
                                       m_vStackSymIndices.end(),
                                       [&pSym](const RCP<const Symbol> &lv) {
                                           return eq(*lv, *pSym);
                                       });
                if (it != m_vStackSymIndices.end()) {
                    // get the index
                    size_t idx = std::distance(m_vStackSymIndices.begin(), it);
                    stackSyms.push_back(pSym);
                    stackLBs.push_back(m_vStackLowerBonds[idx]);
                    stackUBs.push_back(m_vStackUpperBonds[idx]);
                }
            }
        }
    }
    if (stackSyms.empty()) {
        m_oResult_ = {{zero, {}}};
        PrintDetailedExprs(m_oResult_, "Early exit6");
        m_iNestingLevel--;
        return;
    }
    VecMatchesMap vecMatchedMap;
    DynamicLoop(
        stackLBs, stackUBs,
        [this, &self, &stackSyms, &vecMatchedMap](
        const std::vector<size_t> &idx) {
            // Replace SymIndex instances with their Integer values
            map_basic_basic m;
            for (size_t i = 0; i < stackSyms.size(); ++i) {
                m[stackSyms[i]] = integer(idx[i]);
            }
            auto newSelfWithModulos = self.subs(m);
            RCP<const Basic> newSelf = newSelfWithModulos;

            if (m_bResolveModulos) {
                ModuloVisitor visitor;
                newSelf = visitor.Apply(*newSelfWithModulos);
            }

            if (eq(*newSelf, *m_pWrt)) {
                map_basic_basic mm;
                for (int index = 0; index < idx.size(); ++index) {
                    // Store the matched indices
                    mm[stackSyms[index]] = integer(idx[index]);
                }
                vecMatchedMap.push_back(mm);
            }
        }
    );

    if (vecMatchedMap.empty()) {
        m_oResult_ = {{zero, {}}};
    } else {
        // If we have matches, return the first one
        m_oResult_ = {{one, vecMatchedMap}};
    }
    PrintDetailedExprs(m_oResult_, "Final MemAccess Result");
    m_iNestingLevel--;
}

void DiffVisitor2::bvisit(const FunctionSymbol &self)
{
    if (m_bDebug)
        std::cout << Indent() << "bvisit FunctionSymbol: " << self.__str__() <<
            "\n";
    m_iNestingLevel++;
    if (self.get_name() == "Sum") {
        // ... your differentiation logic for Sum ...
        // example:
        auto args = self.get_args();
        size_t vars = (args.size() - 1) / 3;
        // push each loop var with its LB and UB to the stack
        for (size_t i = 0; i < vars; ++i) {
            auto loopVar = rcp_static_cast<const Symbol>(args[1 + i * 3]);
            auto lb = args[2 + i * 3];
            auto ub = args[3 + i * 3];
            auto plb = rcp_static_cast<const Integer>(lb);
            auto pub = rcp_static_cast<const Integer>(ub);

            m_vStackSymIndices.push_back(loopVar);
            m_vStackLowerBonds.emplace_back(plb->as_int());
            m_vStackUpperBonds.emplace_back(pub->as_int());
        }

        auto loopBody = args[0];
        loopBody->accept(*this);
        auto dLoopBody = m_oResult_;

        // pop the pushed stuff
        for (size_t i = 0; i < vars; ++i) {
            m_vStackSymIndices.pop_back();
            m_vStackLowerBonds.pop_back();
            m_vStackUpperBonds.pop_back();
        }

        RCP<const Basic> resolved = zero;
        VecMatchesMap unionMap;
        for (auto &piece : dLoopBody) {
            for (const auto &m : piece.second) {
                // Resolve the matches in the loop body
                auto e = piece.first->subs(m);
                // any entry means a non-zero derivative.
                resolved = add(resolved, e);
                unionMap.push_back(m);
            }
        }

        m_oResult_ = {{resolved, unionMap}};
        PrintDetailedExprs(m_oResult_, "Final Sum Result");
        m_iNestingLevel--;
        return;
    }
    // Otherwise, fallback
    m_oResult_ = {{zero, {}}};
    m_iNestingLevel--;
}

void ::DiffVisitor2::bvisit(const Basic &self)
{
    m_oResult_ = {{zero, {}}};
}

void DiffVisitor2::DynamicLoop(const std::vector<size_t> &lows,
                               const std::vector<size_t> &highs,
                               const std::function<void(
                                   const std::vector<size_t> &)> &job)
{
    size_t N = lows.size();
    std::vector<size_t> idx = lows;

    while (true) {
        // do work
        if (m_bDebug) {
            std::cout << Indent() << "DynamicLoop Iteration Indices: ";
            for (size_t i = 0; i < N; ++i)
                std::cout << idx[i] << " ";
            std::cout << "\n";
        }
        job(idx);
        ssize_t level = N - 1;
        while (level >= 0) {
            ++idx[level];
            if (idx[level] <= highs[level]) {
                break;
            } else {
                idx[level] = lows[level];
                --level;
            }
        }

        if (level < 0)
            break;
    }
}

__always_inline std::string DiffVisitor2::Indent()
{
    if (!m_bDebug)
        return "";
    std::string indent;
    for (size_t i = 0; i < m_iNestingLevel; ++i) {
        indent += "  ";
    }
    return indent;
}

__always_inline void DiffVisitor2::PrintDetailedExprs(const DetailedExprs &de,
    const std::string &msg)
{
    if (!m_bDebug)
        return;
    std::cout << Indent() << "----------------------------------------\n";
    std::cout << Indent() << msg << ": \n";
    for (const auto &pair : de) {
        std::cout << Indent() << "Expr: " << pair.first->__str__() << "\n";
        std::cout << Indent() << "Matches:\n";
        for (const auto &match : pair.second) {
            std::cout << Indent() << "  ";
            for (const auto &m : match) {
                std::cout << m.first->__str__() << " -> " << m.second->__str__()
                    << ", ";
            }
            std::cout << "\n";
        }
    }
}

__always_inline DiffVisitor2::DetailedExprs
DiffVisitor2::AggrigateTwoDetailedExprs(
    const DetailedExprs &de1, const DetailedExprs &de2, bool isAdd, bool isMul)
{
    PrintDetailedExprs(de1, "AggrigateTwoDetailedExprs DE1");
    PrintDetailedExprs(de2, "AggrigateTwoDetailedExprs DE2");
    if (m_iNestingLevel == 1) {
        int a = 1;
    }
    if (de1.empty())
        return de2;
    if (de2.empty())
        return de1;
    if (isAdd) {
        // Union: just concatenate the two vectors
        VecMatchesMap unionOfAllIterations;

        for (auto &piece : de1) {
            unionOfAllIterations.insert(
                unionOfAllIterations.end(),
                piece.second.begin(),
                piece.second.end()
            );
        }
        for (auto &piece : de2) {
            unionOfAllIterations.insert(
                unionOfAllIterations.end(),
                piece.second.begin(),
                piece.second.end()
            );
        }

        // Remove duplicates from unionOfAllIterations without using STL algorithms
        for (size_t i = 0; i < unionOfAllIterations.size(); ++i) {
            for (size_t j = i + 1; j < unionOfAllIterations.size();) {
                if (unordered_eq(unionOfAllIterations[i],
                                 unionOfAllIterations[j])) {
                    unionOfAllIterations.
                        erase(unionOfAllIterations.begin() + j);
                } else {
                    ++j;
                }
            }
        }

        std::map<RCP<const Basic>, std::vector<map_basic_basic>,
                 RCPBasicKeyLess> exprValPerUnifiedIter;
        for (auto &iter : unionOfAllIterations) {
            if (iter.empty())
                continue;
            RCP<const Basic> exprAggr = zero;
            for (auto &piece : de1) {
                // check if iter which is a map_basic_basic is in piece.second's vector of map_basic_basic
                for (auto &c : piece.second) {
                    if (unordered_eq(iter, c)) {
                        // Found a match, add to the map
                        exprAggr = add(exprAggr, piece.first);
                        break;
                    }
                }
            }
            for (auto &piece : de2) {
                // check if iter which is a map_basic_basic is in piece.second's vector of map_basic_basic
                for (auto &c : piece.second) {
                    if (unordered_eq(iter, c)) {
                        // Found a match, add to the map
                        exprAggr = add(exprAggr, piece.first);
                        break;
                    }
                }
            }
            if (not eq(*exprAggr, *zero)) {
                exprValPerUnifiedIter[exprAggr].push_back(iter);
            }
        }

        DetailedExprs result;
        for (const auto &pair : exprValPerUnifiedIter) {
            result.emplace_back(pair.first, pair.second);
        }

        // remove empty maps from unionOfAllIterations
        unionOfAllIterations.erase(
            std::remove_if(unionOfAllIterations.begin(),
                           unionOfAllIterations.end(),
                           [](const map_basic_basic &m) {
                               return m.empty();
                           }),
            unionOfAllIterations.end()
        );

        if (unionOfAllIterations.empty()) {
            // it means that both de1 and de2 have no match maps.
            // both of them are valid for the universe set.
            // but de1 is a vector of pieces, de2 as well.
            // So none of them had any match maps. we have to add all of them
            RCP<const Basic> sum = zero;
            for (const auto &piece : de1) {
                sum = add(sum, piece.first);
            }
            for (const auto &piece : de2) {
                sum = add(sum, piece.first);
            }
            result.emplace_back(sum, std::vector<map_basic_basic>{});
        }

        PrintDetailedExprs(result, "AggrigateTwoDetailedExprs Add Result");
        return result;
    } else {
        if (isMul) {
            DetailedExprs result;
            for (auto &piece1 : de1) {
                for (auto &piece2 : de2) {
                    // take intersection of the two vector of maps
                    std::vector<map_basic_basic> intersection;
                    for (const auto &m1 : piece1.second) {
                        for (const auto &m2 : piece2.second) {
                            if (unordered_eq(m1, m2)) {
                                intersection.push_back(m1);
                                break;
                                // found a match, no need to check further
                            }
                        }
                    }
                    if (piece1.second.empty()) {
                        // no maps in piece1 means that it is valid for the universe set.
                        intersection = piece2.second; // take all from piece2
                    } else if (piece2.second.empty()) {
                        // no maps in piece2 means that it is valid for the universe set.
                        intersection = piece1.second; // take all from piece1
                    }
                    //if (!intersection.empty()) {
                    // If we have an intersection, create a new entry
                    result.emplace_back(
                        mul(piece1.first, piece2.first),
                        intersection
                    );
                    //}
                }
            }
            // now we need to find all pieces with the same expr and merge their maps.
            std::map<RCP<const Basic>, std::vector<map_basic_basic>,
                     RCPBasicKeyLess> exprValPerUnifiedIter;
            for (auto &piece : result) {
                RCP<const Basic> exprAggr = piece.first;
                if (piece.second.empty()) {
                    exprValPerUnifiedIter[exprAggr].emplace_back();
                } else {
                    for (const auto &iter : piece.second) {
                        exprValPerUnifiedIter[exprAggr].push_back(iter);
                    }
                }

            }
            result.clear();
            for (const auto &pair : exprValPerUnifiedIter) {
                result.emplace_back(pair.first, pair.second);
            }

            PrintDetailedExprs(result, "AggrigateTwoDetailedExprs Mul Result");
            return result;
        } else {
            throw std::runtime_error("Unsupported operation for aggregation");
        }
    }
}