# LLVM_IR_cyc_c
A simple program to extract Cyclomatic complexity out of LLVM IR (.bc / .ll) files

Compile using:
```clang++ -g -O3 CCcalculator.cpp `llvm-config --cxxflags --ldflags --libs` -o CCcalculator```

Run:
`./CCcalculator bitcodefile.bc`
