#include <llvm/IR/Type.h>

#include <fstream>
#include <sstream>
#include <unistd.h>
#include <vector>

#include "debug_info.h"
#include "module.h"

DebugInfo::DebugInfo(ModuleBuilder &mb)
    : indent_(0), line_no_(0), enabled_(true), mb_(mb), source_file_([&] {
        auto source_dir = std::filesystem::temp_directory_path() /
                          ("llvm-playground-" + std::to_string(getpid()));
        std::filesystem::create_directories(source_dir);
        return source_dir / (mb_.module().getName().str() + ".cc");
      }()),
      dbg_builder_(mb_.module()),
      dbg_file_(dbg_builder_.createFile(source_file_.string(),
                                        source_file_.parent_path().string())),
      dbg_scope_(dbg_file_) {

  dbg_builder_.createCompileUnit(llvm::dwarf::DW_LANG_C_plus_plus, dbg_file_,
                                 "llvm-playground", true, "", 0);
}

DebugInfo::~DebugInfo() {
  std::filesystem::remove_all(source_file_.parent_path());
}

void DebugInfo::AddLine(std::string const &line) {
  source_code_ << std::string(indent_, ' ') << line << "\n";
  line_no_++;
}

llvm::DebugLoc DebugInfo::GetDebugLocation() {
  return llvm::DebugLoc::get(line_no_, 1, dbg_scope_);
}

llvm::DIType *DebugInfo::GetDIType(llvm::Type *type) {
  auto data_layout = mb_.module().getDataLayout();

  switch (type->getTypeID()) {
  case llvm::Type::HalfTyID: {
    return dbg_builder_.createBasicType("half", 16, llvm::dwarf::DW_ATE_float);
  }
  case llvm::Type::FloatTyID: {
    return dbg_builder_.createBasicType("float", 32, llvm::dwarf::DW_ATE_float);
  }
  case llvm::Type::DoubleTyID: {
    return dbg_builder_.createBasicType("double", 64,
                                        llvm::dwarf::DW_ATE_float);
  }
  case llvm::Type::X86_FP80TyID: {
    return dbg_builder_.createBasicType("x86_fp80", 80,
                                        llvm::dwarf::DW_ATE_float);
  }
  case llvm::Type::FP128TyID: {
    return dbg_builder_.createBasicType("fp128", 128,
                                        llvm::dwarf::DW_ATE_float);
  }
  case llvm::Type::PPC_FP128TyID: {
    return dbg_builder_.createBasicType("ppc_fp128", 128,
                                        llvm::dwarf::DW_ATE_float);
  }
  case llvm::Type::IntegerTyID: {
    auto *int_type = llvm::dyn_cast<llvm::IntegerType>(type);
    return dbg_builder_.createBasicType(
        "i" + std::to_string(int_type->getBitWidth()), int_type->getBitWidth(),
        llvm::dwarf::DW_ATE_unsigned);
  }
  case llvm::Type::FunctionTyID: {
    auto *func_type = llvm::dyn_cast<llvm::FunctionType>(type);
    std::vector<llvm::Metadata *> arg_types;
    for (llvm::Type *arg_type : func_type->params()) {
      arg_types.push_back(GetDIType(arg_type));
    }
    return dbg_builder_.createSubroutineType(
        dbg_builder_.getOrCreateTypeArray(arg_types));
  }
  case llvm::Type::StructTyID: {
    auto *struct_type = llvm::dyn_cast<llvm::StructType>(type);
    if (struct_type->isOpaque()) {
      return dbg_builder_.createUnspecifiedType(struct_type->getName());
    }
    std::vector<llvm::Metadata *> decl_types;
    int i = 0;
    for (llvm::Type *arg_type : struct_type->elements()) {
      auto *decl_type = GetDIType(arg_type);
      decl_types.push_back(dbg_builder_.createMemberType(
          dbg_file_, std::to_string(i), dbg_file_, 0,
          data_layout.getTypeSizeInBits(arg_type),
          data_layout.getABITypeAlignment(arg_type),
          data_layout.getStructLayout(struct_type)->getElementOffsetInBits(i),
          llvm::DINode::FlagZero, decl_type));
      i++;
    }
    return dbg_builder_.createStructType(
        dbg_file_, struct_type->hasName() ? struct_type->getName() : "",
        dbg_file_, 0, data_layout.getTypeSizeInBits(type),
        data_layout.getABITypeAlignment(type), llvm::DINode::FlagZero, nullptr,
        dbg_builder_.getOrCreateArray(decl_types));
  }
  case llvm::Type::ArrayTyID: {
    auto *array_type = llvm::dyn_cast<llvm::ArrayType>(type);
    std::vector<llvm::Metadata *> subscripts;
    subscripts.push_back(
        dbg_builder_.getOrCreateSubrange(0, array_type->getNumElements()));
    return dbg_builder_.createArrayType(
        data_layout.getTypeSizeInBits(type),
        data_layout.getABITypeAlignment(type),
        GetDIType(array_type->getElementType()),
        dbg_builder_.getOrCreateArray(subscripts));
  }
  case llvm::Type::PointerTyID: {
    auto *pointer_type = llvm::dyn_cast<llvm::PointerType>(type);
    return dbg_builder_.createPointerType(
        GetDIType(pointer_type->getElementType()),
        data_layout.getTypeSizeInBits(type));
  }
  default:
    std::string name;
    llvm::raw_string_ostream os(name);
    type->print(os);
    return dbg_builder_.createUnspecifiedType(name);
  }
}

void DebugInfo::EnterFunction(llvm::Function *function) {
  auto str = std::stringstream{};

  auto return_type = GetDIType(function->getReturnType());
  str << return_type->getName().str() << " " << function->getName().str()
      << "(";

  for (auto arg = function->arg_begin(); arg != function->arg_end(); arg++) {
    auto arg_type = GetDIType(arg->getType());
    auto arg_idx = arg->getArgNo() + 1;
    auto arg_name = "arg" + std::to_string(arg_idx);
    arg->setName(arg_name);
    str << arg_type->getName().str() << " " << arg_name;
    if (arg + 1 != function->arg_end()) {
      str << ", ";
    }
    auto dbg_arg = dbg_builder_.createParameterVariable(
        dbg_scope_, arg_name, arg_idx, dbg_file_, line_no_, arg_type);
    dbg_builder_.insertDbgValueIntrinsic(
        arg, dbg_arg, dbg_builder_.createExpression(), GetDebugLocation(),
        function->getEntryBlock().getFirstNonPHI());
  }

  auto fn_type = llvm::dyn_cast<llvm::DISubroutineType>(
      GetDIType(function->getFunctionType()));

  auto fn_scope = dbg_builder_.createFunction(
      dbg_scope_, function->getName().str(), function->getName().str(),
      dbg_file_, line_no_, fn_type, line_no_, llvm::DINode::FlagPrototyped,
      llvm::DISubprogram::DISPFlags::SPFlagDefinition |
          llvm::DISubprogram::DISPFlags::SPFlagOptimized);

  function->setSubprogram(fn_scope);
  mb_.ir_builder().SetCurrentDebugLocation(GetDebugLocation());

  str << ") {";
  AddLine(str.str());
  EnterScope();
}

void DebugInfo::ExitFunction(llvm::Value *ret_value) {
  if (ret_value != nullptr) {
    auto str = std::stringstream{};
    str << "return " << ret_value->getName().str() << ";";
    AddLine(str.str());
  }
  LeaveScope();
  AddLine("}");
}

void DebugInfo::ExitModule() {
  {
    auto ofs = std::ofstream(source_file_, std::ios::trunc);
    ofs << source_code_.str();
  }
  dbg_builder_.finalize();
}
