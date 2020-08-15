
#ifndef PLJIT_PLJIT_HPP
#define PLJIT_PLJIT_HPP

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "pljit/semantic_analysis/symbol_table.hpp"
#include <string_view>

namespace pljit {
    namespace semantic_analysis {
        class FunctionNode;
    } // namespace semantic_analysis

    class Function {
        std::mutex compilation_mutex;
        source_management::source_code source_code;
        std::unique_ptr<semantic_analysis::FunctionNode> ast;
        semantic_analysis::symbol_table symbol_table;
        bool compilation_failed = false;

#ifndef NDEBUG
        unsigned int compilation_passed = 0;
#endif

        void compile();
        std::optional<int64_t> call_impl(std::initializer_list<int64_t>);

        public:
        explicit Function(std::string source);

        template<class... Args>
        std::optional<int64_t> operator()(Args... args) {
            static_assert(std::conjunction_v<std::is_convertible<Args, int64_t>...>, "PL supports only int64_t parameters.");
            // std::forward is not superfluous, we may pass arbitrary types convertible to int64_t.
            return call_impl({std::forward<Args>(args)...});
        }
        ~Function();
    };

    class function_handle;

    class Pljit {
        std::vector<std::unique_ptr<Function>> registered_functions;

        public:
        function_handle register_function(std::string source);

        Function& get(unsigned id) {
            return *registered_functions[id];
        }
    };

    class function_handle {
        friend Pljit;
        Pljit* pljit;
        unsigned function_id;

        explicit function_handle(Pljit* pljit, unsigned function_id) : pljit(pljit), function_id(function_id) {};

        public:
        template<class... Args>
        std::optional<int64_t> operator()(Args... args) {
            static_assert(std::conjunction_v<std::is_convertible<Args, int64_t>...>, "PL supports only int64_t parameters.");
            // std::forward is not superfluous, we may pass arbitrary types convertible to int64_t.
            return pljit->get(function_id)(std::forward<Args>(args)...);
        }
    };

} // namespace pljit

#endif //PLJIT_PLJIT_HPP
