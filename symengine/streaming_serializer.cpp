//
// Created by saleh on 12/14/24.
//

#include <symengine/serialize-cereal.h>
#include <symengine/basic.h>
#include <symengine/streaming_serializer.h>


SymEngine::streaming_serializer::streaming_serializer(
    const std::string &filename)
    : file(filename, std::ios::binary),
        count(0)
{

    if (!file) {
        throw std::ios_base::failure("Failed to open file.");
    }

    ser = std::make_unique<
        RCPBasicAwareOutputArchive<cereal::PortableBinaryOutputArchive>
    >(file);

    unsigned short major = SYMENGINE_MAJOR_VERSION;
    unsigned short minor = SYMENGINE_MINOR_VERSION;

    save_cereal(major, minor);
}

void SymEngine::streaming_serializer::save(const RCP<const Basic> &expr)
{
    save_cereal(expr);
    count++;
}

void SymEngine::streaming_serializer::save(const vec_basic &expr)
{
    for (const auto &e : expr) {
        // dont write `expr` in one go. The whole idea is to be able to
        // write indefinite number of expressions
        save_cereal(e);
    }
    count += expr.size();
}

SymEngine::streaming_serializer::~streaming_serializer()
{
    ser.reset();
    file.flush();
    // seek to the end of the file explicitly and write the count
    file.seekp(0, std::ios::end);
    file.write(reinterpret_cast<const char *>(&count), sizeof(size_t));
    file.close();
}