//
// Created by saleh on 12/14/24.
//

#ifndef STREAMING_DESERIALIZER_H
#define STREAMING_DESERIALIZER_H


#include <symengine/basic.h>
#include <fstream>
#include <memory>

// The forward declaration of cereal::PortableBinaryOutputArchive
namespace cereal {
    class PortableBinaryInputArchive;
}

namespace SymEngine {

    // The forward declaration of RCPBasicAwareOutputArchive template
    // The header file containing it should not be published.
    template <class Archive>
    class RCPBasicAwareInputArchive;

    class streaming_deserializer {
    protected:
        std::unique_ptr<
            RCPBasicAwareInputArchive<cereal::PortableBinaryInputArchive>
        >deser;
        std::ifstream file;
        size_t count, index;
    public:
        streaming_deserializer(const std::string &filename);

        size_t size() const;

        size_t current_index() const;

        /**
         * This is to expose cereal serialization functions to the user, without
         * needing the non-published headers.
         * @tparam Types
         * @param args
         */
        template <class ... Types>
        inline void load_cereal(Types && ... args);

        bool load_next(RCP<const Basic> &outExpr);

        vec_basic load_all();

        ~streaming_deserializer();
    };
}
#endif //STREAMING_DESERIALIZER_H
