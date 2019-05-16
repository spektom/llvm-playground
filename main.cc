#include "codegen.h"

int main(int argc, char **argv) {
  Codegen codegen(argc, argv);
  std::unique_ptr<CodeContext> context =
      codegen.CreateContext("MyModule", OptimizationLevel::O2);
}
