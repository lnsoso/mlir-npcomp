// RUN: npcomp-opt <%s | FileCheck %s --dump-input=fail

func @f(%arg0: tensor<?xf32>, %arg1: tensor<?xf32>) {
  // CHECK: "tcf.add"
  %0 = "tcf.add"(%arg0, %arg1) : (tensor<?xf32>, tensor<?xf32>) -> tensor<?xf32>
  return
}
