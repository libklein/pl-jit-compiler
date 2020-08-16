
#ifndef PLJIT_DEAD_CODE_ELIMINATION_HPP
#define PLJIT_DEAD_CODE_ELIMINATION_HPP

#include "pljit/optimization/optimization_pass.hpp"

namespace pljit::optimization::passes {

class dead_code_elimination : public optimization_pass {
    public:
    dead_code_elimination() = default;

    void optimize(semantic_analysis::FunctionNode& node) override;
};

} // namespace pljit::optimization::passes

#endif //PLJIT_DEAD_CODE_ELIMINATION_HPP
