#include "Zipline.h"

#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/FileSystem.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

void tools::zipline::Assembler::ConstructJob(Compilation &C,
                                             const JobAction &JA,
                                             const InputInfo &Output,
                                             const InputInfoList &Inputs,
                                             const ArgList &Args,
                                             const char *LinkingOutput) const {
  ArgStringList CmdArgs;
  const InputInfo &II = Inputs[0];

  // gnu base64
  // base64 [input] > [output]
  Args.AddAllArgValues(CmdArgs, options::OPT_Wa_COMMA, options::OPT_Xassembler);
  CmdArgs.push_back(II.getFilename());

  std::string Exec =
      Args.MakeArgString(getToolChain().GetProgramPath("base64"));
  auto Cmd = std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Args.MakeArgString(Exec), CmdArgs,
                                         Inputs, Output);
  Cmd->setRedirectFiles(
    {std::nullopt, std::string(Output.getFilename()), std::nullopt});
  C.addCommand(std::move(Cmd));
}

void tools::zipline::Linker::build7zArgs(const JobAction &JA,
                                          const InputInfo &Output,
                                          const InputInfoList &Inputs,
                                          const ArgList &Args,
                                          ArgStringList &CmdArgs) const {
  // 'a' for archive
  CmdArgs.push_back("a");
  // output file
  StringRef OutFile = Output.getFilename();
  if (!OutFile.ends_with(".7z"))
    CmdArgs.push_back(
      Args.MakeArgString(OutFile + ".7z"));
  else
    CmdArgs.push_back(OutFile.data());
  // input files
  AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);
}

void tools::zipline::Linker::buildZipArgs(const JobAction &JA,
                                          const InputInfo &Output,
                                          const InputInfoList &Inputs,
                                          const ArgList &Args,
                                          ArgStringList &CmdArgs) const {
  // output file
  StringRef OutFile = Output.getFilename();
  if (!OutFile.ends_with(".zip"))
    CmdArgs.push_back(
      Args.MakeArgString(OutFile + ".zip"));
  else
    CmdArgs.push_back(OutFile.data());
  // input files
  AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);
}

void tools::zipline::Linker::buildTarArgs(const JobAction &JA,
                                          const InputInfo &Output,
                                          const InputInfoList &Inputs,
                                          const ArgList &Args,
                                          ArgStringList &CmdArgs) const {
  // arguments and output file
  CmdArgs.push_back("-czf");
  StringRef OutFile = Output.getFilename();
  if (!OutFile.ends_with(".tar.gz"))
    CmdArgs.push_back(
      Args.MakeArgString(OutFile + ".tar.gz"));
  else
    CmdArgs.push_back(OutFile.data());
  // input files
  AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);
}

void tools::zipline::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                          const InputInfo &Output,
                                          const InputInfoList &Inputs,
                                          const ArgList &Args,
                                          const char *LinkingOutput) const {
  ArgStringList CmdArgs;
  std::string Compressor = "7z";
  if (Arg *A = Args.getLastArg(options::OPT_fuse_ld_EQ))
    Compressor = A->getValue();

  if (Compressor == "7z")
  {
    build7zArgs(JA, Output, Inputs, Args, CmdArgs);
  }
  else
  {
    if (Compressor == "zip")
     buildZipArgs(JA, Output, Inputs, Args, CmdArgs);
    if (Compressor == "tar" || Compressor == "gzip")
     buildTarArgs(JA, Output, Inputs, Args, CmdArgs);
    else
     llvm_unreachable("Unsupported compressor name");
  }

  std::string Exec =
      Args.MakeArgString(getToolChain().GetProgramPath(Compressor.c_str()));
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::None(), Args.MakeArgString(Exec),
      CmdArgs, Inputs, Output));
}

ZiplineToolChain::~ZiplineToolChain() {}

void ZiplineToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                                 ArgStringList &CC1Args) const {
  using namespace llvm;
  SmallString<16> CustomIncludePath;
  sys::fs::expand_tilde("~/my_include", CustomIncludePath);
  addSystemInclude(DriverArgs, CC1Args, CustomIncludePath.c_str());
}

Tool *ZiplineToolChain::buildAssembler() const {
  return new tools::zipline::Assembler(*this);
}

Tool *ZiplineToolChain::buildLinker() const {
  return new tools::zipline::Linker(*this);
}
