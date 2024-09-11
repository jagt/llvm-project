#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ZIPLINE_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ZIPLINE_H

#include "Gnu.h"

#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {
namespace tools {

/// Zipline - A demo toolchain
namespace zipline {
struct LLVM_LIBRARY_VISIBILITY Assembler : public Tool {
  Assembler(const ToolChain &TC) : Tool("zipeline::toBase64", "toBase64", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

struct LLVM_LIBRARY_VISIBILITY Linker : public Tool {
  Linker(const ToolChain &TC) : Tool("zipeline::zipper", "zipper", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;

private:
  void build7zArgs(const JobAction&, const InputInfo&, const InputInfoList&,
                    const llvm::opt::ArgList&, llvm::opt::ArgStringList&) const;
  void buildZipArgs(const JobAction&, const InputInfo&, const InputInfoList&,
                    const llvm::opt::ArgList&, llvm::opt::ArgStringList&) const;
  void buildTarArgs(const JobAction&, const InputInfo&, const InputInfoList&,
                    const llvm::opt::ArgList&, llvm::opt::ArgStringList&) const;
};
} // end namespace zipline
} // end namespace tools

namespace toolchains {
struct LLVM_LIBRARY_VISIBILITY ZiplineToolChain : public Generic_ELF {
  ZiplineToolChain(const Driver &D, const llvm::Triple &Triple,
                   const llvm::opt::ArgList &Args)
    : Generic_ELF(D, Triple, Args) {}

  ~ZiplineToolChain() override;

  // Disable the integrated assembler. Otherwise Clang will emit
  // object file on x86_64 platforms by default.
  bool IsIntegratedAssemblerDefault() const override { return false; }
  bool useIntegratedAs() const override { return false; }

  void
  AddClangSystemIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                            llvm::opt::ArgStringList &CC1Args) const override;

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
};
} // end namespace toolchains
} // end namespace driver
} // end namespace clang
#endif