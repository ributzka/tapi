#include "VTable.h"

// VTables
void test1::Simple::run() {}
void test2::Simple::run() {}
void test4::Sub::run() {}
void test5::Sub::run() {}
void test6::Sub::run() {}
void test11::Sub::run3() {}

template class test12::Simple<int>;

// Weak-Defined RTTI
__attribute__((visibility("hidden"))) test7::Sub a;
