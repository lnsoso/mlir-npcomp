//===- BasicpyOps.cpp - Core numpy dialect ops --------------------*- C++
//-*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "npcomp/Dialect/Basicpy/BasicpyOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/FunctionImplementation.h"
#include "mlir/IR/OpImplementation.h"
#include "npcomp/Dialect/Basicpy/BasicpyDialect.h"

namespace mlir {
namespace NPCOMP {
namespace Basicpy {

//===----------------------------------------------------------------------===//
// SlotObjectMakeOp
//===----------------------------------------------------------------------===//

static ParseResult parseSlotObjectMakeOp(OpAsmParser &parser,
                                         OperationState *result) {
  llvm::SmallVector<OpAsmParser::OperandType, 4> operandTypes;
  if (parser.parseOperandList(operandTypes, OpAsmParser::Delimiter::Paren) ||
      parser.parseOptionalAttrDict(result->attributes) ||
      parser.parseArrowTypeList(result->types)) {
    return failure();
  }

  if (result->types.size() != 1 ||
      !result->types.front().isa<SlotObjectType>()) {
    return parser.emitError(parser.getNameLoc(),
                            "custom assembly form requires SlotObject result");
  }
  auto slotObjectType = result->types.front().cast<SlotObjectType>();
  result->addAttribute("className", slotObjectType.getClassName());
  return parser.resolveOperands(operandTypes, slotObjectType.getSlotTypes(),
                                parser.getNameLoc(), result->operands);
}

static void printSlotObjectMakeOp(OpAsmPrinter &p, SlotObjectMakeOp op) {
  // If the argument types do not match the result type slots, then
  // print the generic form.
  auto canCustomPrint = ([&]() -> bool {
    auto type = op.result().getType().dyn_cast<SlotObjectType>();
    if (!type)
      return false;
    auto args = op.slots();
    auto slotTypes = type.getSlotTypes();
    if (args.size() != slotTypes.size())
      return false;
    for (unsigned i = 0, e = args.size(); i < e; ++i) {
      if (args[i].getType() != slotTypes[i])
        return false;
    }
    return true;
  })();
  if (!canCustomPrint) {
    p.printGenericOp(op);
    return;
  }

  p << op.getOperationName() << "(";
  p.printOperands(op.slots());
  p << ")";
  p.printOptionalAttrDict(op.getAttrs(), {"className"});

  // Not really a symbol but satisfies same rules.
  p.printArrowTypeList(op.getOperation()->getResultTypes());
}

//===----------------------------------------------------------------------===//
// SlotObjectGetOp
//===----------------------------------------------------------------------===//

static ParseResult parseSlotObjectGetOp(OpAsmParser &parser,
                                        OperationState *result) {
  OpAsmParser::OperandType object;
  IntegerAttr indexAttr;
  Type indexType = parser.getBuilder().getIndexType();
  if (parser.parseOperand(object) || parser.parseLSquare() ||
      parser.parseAttribute(indexAttr, indexType, "index",
                            result->attributes) ||
      parser.parseRSquare()) {
    return failure();
  }
  Type objectType;
  if (parser.parseColonType(objectType) ||
      parser.resolveOperand(object, objectType, result->operands)) {
    return failure();
  }

  auto castObjectType = objectType.dyn_cast<SlotObjectType>();
  if (!castObjectType) {
    return parser.emitError(parser.getNameLoc(),
                            "illegal object type on custom assembly form");
  }
  auto index = indexAttr.getValue().getZExtValue();
  auto slotTypes = castObjectType.getSlotTypes();
  if (index >= slotTypes.size()) {
    return parser.emitError(parser.getNameLoc(),
                            "out of bound index on custom assembly form");
  }
  result->addTypes({slotTypes[index]});
  return success();
}

static void printSlotObjectGetOp(OpAsmPrinter &p, SlotObjectGetOp op) {
  // If the argument types do not match the result type slots, then
  // print the generic form.
  auto canCustomPrint = ([&]() -> bool {
    auto type = op.object().getType().dyn_cast<SlotObjectType>();
    if (!type)
      return false;
    auto index = op.index().getZExtValue();
    if (index >= type.getSlotCount())
      return false;
    if (op.result().getType() != type.getSlotTypes()[index])
      return false;
    return true;
  })();
  if (!canCustomPrint) {
    p.printGenericOp(op);
    return;
  }

  p << op.getOperationName() << " ";
  p.printOperand(op.object());
  p << "[" << op.index() << "]";
  p.printOptionalAttrDict(op.getAttrs(), {"index"});
  p << " : ";
  p.printType(op.object().getType());
}

#define GET_OP_CLASSES
#include "npcomp/Dialect/Basicpy/BasicpyOps.cpp.inc"
} // namespace Basicpy
} // namespace NPCOMP
} // namespace mlir
