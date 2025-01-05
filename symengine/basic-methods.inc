class EvalRealDoubleVisitorFinal;

#define SYMENGINE_INCLUDE_METHODS_BASE()                                       \
    virtual void accept(Visitor &v) const = 0;                                 \
    virtual SymEngine::vec_basic accept2(Visitor2 &v) const = 0;                \
    virtual std::string accept3(Visitor3 &v) const = 0;                         \
    virtual size_t accept4(Visitor4 &v) const = 0;                         \
    virtual void accept(EvalRealDoubleVisitorFinal &v) const = 0;

#define SYMENGINE_INCLUDE_METHODS_DERIVED()                                    \
    void accept(Visitor &v) const override;                                    \
    SymEngine::vec_basic accept2(Visitor2 &v) const override;                   \
    std::string accept3(Visitor3 &v) const override;                            \
    size_t accept4(Visitor4 &v) const override;                            \
    void accept(EvalRealDoubleVisitorFinal &v) const override;
