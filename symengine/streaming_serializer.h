//
// Created by saleh on 12/14/24.
//

#ifndef STREAMINGSERIALIZER_H
#define STREAMINGSERIALIZER_H

#include <symengine/basic.h>
#include <fstream>
#include <memory>

// The forward declaration of cereal::PortableBinaryOutputArchive
 namespace cereal {
    class PortableBinaryOutputArchive;
 }

namespace SymEngine {

    // The forward declaration of RCPBasicAwareOutputArchive template
    // The header file containing it should not be published.
    template <class Archive>
    class RCPBasicAwareOutputArchive;

    class streaming_serializer {
    protected:
        std::unique_ptr<
            RCPBasicAwareOutputArchive<cereal::PortableBinaryOutputArchive>
        >ser;
        std::ofstream file;
        size_t count;

    public:
        streaming_serializer(const std::string &filename);

        /**
         * This is to expose cereal serialization functions to the user, without
         * needing the non-published headers.
         * @tparam Types
         * @param args
         */
        template <class ... Types>
        inline void save_cereal(Types && ... args);

        void save(const RCP<const Basic> &expr);

        void save(const vec_basic &expr);

        ~streaming_serializer();
    };

    template <class ... Types>
    void streaming_serializer::save_cereal(Types &&...args)
    {
        (*ser)(std::forward<Types>(args)...);
    }
}

#endif //STREAMINGSERIALIZER_H
