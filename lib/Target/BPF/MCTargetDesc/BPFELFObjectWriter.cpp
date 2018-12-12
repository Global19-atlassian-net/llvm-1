//===-- BPFELFObjectWriter.cpp - BPF ELF Writer ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"
#include <cstdint>

using namespace llvm;

namespace {

class BPFELFObjectWriter : public MCELFObjectTargetWriter {
public:
  BPFELFObjectWriter(uint8_t OSABI);
  ~BPFELFObjectWriter() override = default;

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;

  bool needsRelocateWithSymbol(const MCSymbol &Sym,
                               unsigned Type) const override;
};

} // end anonymous namespace

// Avoid section relocations because the BPF backend can only handle 
// section relocations with values (offset into the section containing
// the symbol being relocated).  Forcing a relocation with a symbol
// will result in the symbol's index being used in the .o file instead.
bool BPFELFObjectWriter::needsRelocateWithSymbol(const MCSymbol &Sym,
                                                 unsigned Type) const {
  return true;
}

BPFELFObjectWriter::BPFELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit*/ true, OSABI, ELF::EM_BPF,
                              /*HasRelocationAddend*/ false) {}

unsigned BPFELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
                                          const MCFixup &Fixup,
                                          bool IsPCRel) const {
  // determine the type of the relocation
  switch ((unsigned)Fixup.getKind()) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case FK_PCRel_4:
  case FK_SecRel_4:
  case FK_Data_4:
    return ELF::R_BPF_64_32;
  case FK_SecRel_8:
  case FK_Data_8:
    return ELF::R_BPF_64_64;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createBPFELFObjectWriter(uint8_t OSABI) {
  return llvm::make_unique<BPFELFObjectWriter>(OSABI);
}
