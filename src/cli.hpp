#ifndef SRC_CLI_HPP_
#define SRC_CLI_HPP_
#include <argparse/argparse.hpp>

namespace dsas {

enum class CliState { root, cast, cal };

CliState parse_args(int argc, char *argv[]);

}  // namespace dsas

#endif