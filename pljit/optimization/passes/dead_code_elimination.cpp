
#include "dead_code_elimination.hpp"
#include "pljit/semantic_analysis/AST.hpp"

using namespace pljit::optimization::passes;

void dead_code_elimination::optimize(pljit::semantic_analysis::FunctionNode& node) {
    unsigned i = 0;
    for (; i < node.get_number_of_statements(); ++i) {
        if (node.get_statement(i)->getType() == semantic_analysis::ASTNode::ReturnStatement) {
            ++i;
            break;
        }
    }

    while (i < node.get_number_of_statements()) {
        node.removeStatement(node.get_number_of_statements());
    }
}
