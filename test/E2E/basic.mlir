// RUN: npcomp-opt <%s -pass-pipeline=e2e-lowering-pipeline | FileCheck %s --dump-input=fail

// This is the simplest case, which is easy to stare at for debugging
// purposes.

// CHECK-LABEL: func @rank1
func @rank1(%arg0: tensor<?xf32>, %arg1: tensor<?xf32>) -> tensor<?xf32> {
  %0 = "tcf.add"(%arg0, %arg1) : (tensor<?xf32>, tensor<?xf32>) -> tensor<?xf32>
  return %0 : tensor<?xf32>
}
