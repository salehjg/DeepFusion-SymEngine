//
// Created by saleh on 12/14/24.
//

#include <symengine/serialize-cereal.h>
#include <symengine/basic.h>
#include <symengine/streaming_deserializer.h>

SymEngine::streaming_deserializer::streaming_deserializer(
    const std::string &filename):
    file(filename, std::ios::binary),
    count(0)
{
    if (!file) {
        throw std::ios_base::failure("Failed to open file.");
    }
    // seek to the end of the file - sizeof(size_t) to read the count
    file.seekg(-static_cast<std::streamoff>(sizeof(size_t)), std::ios::end);
    file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
    file.seekg(0, std::ios::beg);
    deser = std::make_unique<
        RCPBasicAwareInputArchive<cereal::PortableBinaryInputArchive>
    >(file);
    index = 0;
    unsigned short major, minor;
    (*deser)(major, minor);
    if (major != SYMENGINE_MAJOR_VERSION or minor != SYMENGINE_MINOR_VERSION) {
        throw SerializationError(StreamFmt()
                                 << "SymEngine-" << SYMENGINE_MAJOR_VERSION
                                 << "." << SYMENGINE_MINOR_VERSION
                                 << " was asked to deserialize an object "
                                 << "created using SymEngine-" << major << "."
                                 << minor << ".");
    }
}

size_t SymEngine::streaming_deserializer::size() const
{
    return count;
}

size_t SymEngine::streaming_deserializer::current_index() const
{
    return index;
}

template<class ... Types>
void SymEngine::streaming_deserializer::load_cereal(Types &&...args) {
    (*deser)(std::forward<Types>(args)...);
}

bool SymEngine::streaming_deserializer::load_next(RCP<const Basic> &outExpr)
{
    if (index >= count) {
        throw std::runtime_error("Trying to load more than available.");
    }
    (*deser)(outExpr);
    return ++index < count;
}

SymEngine::vec_basic SymEngine::streaming_deserializer::load_all()
{
    vec_basic exprs;
    RCP<const Basic> expr;
    while (load_next(expr)) {
        exprs.push_back(expr);
    }
    return exprs;
}

SymEngine::streaming_deserializer::~streaming_deserializer()
{
    file.close();
}