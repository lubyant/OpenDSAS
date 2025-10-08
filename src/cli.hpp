#ifndef SRC_CLI_HPP_
#define SRC_CLI_HPP_
#include <argparse/argparse.hpp>

namespace dsas {

enum class CliStatus{
    Root,
    Cast,
    Cal
};

CliStatus parse_args(int argc, char *argv[]);

}  // namespace dsas

#endif