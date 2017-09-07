//===- unittests/SDKDB/SDKDB_v1.cpp - SDKDB Test ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "tapi/Core/Registry.h"
#include "tapi/SDKDB/SDKDBFile.h"
#include "tapi/SDKDB/SDKDB_v1.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
#define DEBUG_TYPE "sdkdb-test"

using namespace llvm;
using namespace tapi::internal;

namespace {

Registry setupRegistry() {
  Registry registry;
  auto writer = make_unique<YAMLWriter>();
  writer->add(make_unique<sdkdb::v1::YAMLDocumentHandler>());
  registry.add(std::move(writer));
  return registry;
}

TEST(SDKDB, SymbolsOnly) {
  Registry registry = setupRegistry();

  auto db = make_unique<SDKDBFile>();
  db->setFileType(SDKDB_V1);
  db->setInstallName("/usr/lib/libtest.dylib");
  db->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addGlobalSymbol("sym2", /*isPublic=*/false, AvailabilityInfo(true));
  db->addGlobalSymbol(
      "sym3", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 1, 0), PackedVersion(2, 2, 0), false));
  EXPECT_FALSE(db->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "symbols:         \n"
                         "  - name:            sym1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "  - name:            sym2\n"
                         "    access:          private\n"
                         "    availability:    n/a\n"
                         "  - name:            sym3\n"
                         "    access:          public\n"
                         "    availability:    1.1..2.2\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, ClassesOnly) {
  Registry registry = setupRegistry();

  auto db = make_unique<SDKDBFile>();
  db->setFileType(SDKDB_V1);
  db->setInstallName("/usr/lib/libtest.dylib");
  db->addObjectiveCClass(
      "Class1", "NSObject", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "classes:         \n"
                         "  - name:            Class1\n"
                         "    super-class:     NSObject\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, ClassesOnlyWithMethods) {
  Registry registry = setupRegistry();

  auto db = make_unique<SDKDBFile>();
  db->setFileType(SDKDB_V1);
  db->setInstallName("/usr/lib/libtest.dylib");
  auto *class1 = db->addObjectiveCClass(
      "Class1", "NSObject", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      class1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      class1, "sel2", /*isInstanceMethod=*/true, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      class1, "sel1", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(3, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      class1, "sel2", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(4, 0, 0), PackedVersion(), false));
  db->addObjectiveCClass(
      "Class2", "NSObject", /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "classes:         \n"
                         "  - name:            Class1\n"
                         "    super-class:     NSObject\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "    methods:         \n"
                         "      - name:            sel1\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    3\n"
                         "      - name:            sel1\n"
                         "        kind:            instance\n"
                         "        access:          public\n"
                         "        availability:    1\n"
                         "      - name:            sel2\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    4\n"
                         "      - name:            sel2\n"
                         "        kind:            instance\n"
                         "        access:          private\n"
                         "        availability:    2\n"
                         "  - name:            Class2\n"
                         "    super-class:     NSObject\n"
                         "    access:          private\n"
                         "    availability:    2\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, CategoriesOnly) {
  Registry registry = setupRegistry();

  auto db = make_unique<SDKDBFile>();
  db->setFileType(SDKDB_V1);
  db->setInstallName("/usr/lib/libtest.dylib");
  db->addObjectiveCCategory(
      "Category1", "Class1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "categories:      \n"
                         "  - name:            Category1\n"
                         "    extends:         Class1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, CategoriesOnlyWithMethods) {
  Registry registry = setupRegistry();

  auto db = make_unique<SDKDBFile>();
  db->setFileType(SDKDB_V1);
  db->setInstallName("/usr/lib/libtest.dylib");
  auto *cat1 = db->addObjectiveCCategory(
      "Category1", "Class1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      cat1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      cat1, "sel2", /*isInstanceMethod=*/true, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      cat1, "sel1", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(3, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      cat1, "sel2", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(4, 0, 0), PackedVersion(), false));
  db->addObjectiveCCategory(
      "Category2", "Class1", /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "categories:      \n"
                         "  - name:            Category1\n"
                         "    extends:         Class1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "    methods:         \n"
                         "      - name:            sel1\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    3\n"
                         "      - name:            sel1\n"
                         "        kind:            instance\n"
                         "        access:          public\n"
                         "        availability:    1\n"
                         "      - name:            sel2\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    4\n"
                         "      - name:            sel2\n"
                         "        kind:            instance\n"
                         "        access:          private\n"
                         "        availability:    2\n"
                         "  - name:            Category2\n"
                         "    extends:         Class1\n"
                         "    access:          private\n"
                         "    availability:    2\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, ProtocolsOnly) {
  Registry registry = setupRegistry();

  auto db = make_unique<SDKDBFile>();
  db->setFileType(SDKDB_V1);
  db->setInstallName("/usr/lib/libtest.dylib");
  db->addObjectiveCProtocol(
      "Protocol1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCProtocol(
      "Protocol2", /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "protocols:       \n"
                         "  - name:            Protocol1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "  - name:            Protocol2\n"
                         "    access:          private\n"
                         "    availability:    2\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, ProtocolsOnlyWithMethods) {
  Registry registry = setupRegistry();

  auto db = make_unique<SDKDBFile>();
  db->setFileType(SDKDB_V1);
  db->setInstallName("/usr/lib/libtest.dylib");
  auto *proto1 = db->addObjectiveCProtocol(
      "Protocol1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      proto1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      proto1, "sel2", /*isInstanceMethod=*/true, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      proto1, "sel1", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(3, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      proto1, "sel2", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(4, 0, 0), PackedVersion(), false));
  db->addObjectiveCProtocol(
      "Protocol2", /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "protocols:       \n"
                         "  - name:            Protocol1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "    methods:         \n"
                         "      - name:            sel1\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    3\n"
                         "      - name:            sel1\n"
                         "        kind:            instance\n"
                         "        access:          public\n"
                         "        availability:    1\n"
                         "      - name:            sel2\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    4\n"
                         "      - name:            sel2\n"
                         "        kind:            instance\n"
                         "        access:          private\n"
                         "        availability:    2\n"
                         "  - name:            Protocol2\n"
                         "    access:          private\n"
                         "    availability:    2\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, Everything) {
  Registry registry = setupRegistry();

  auto db = make_unique<SDKDBFile>();
  db->setFileType(SDKDB_V1);
  db->setInstallName("/usr/lib/libtest.dylib");

  db->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addGlobalSymbol("sym2", /*isPublic=*/false, AvailabilityInfo(true));
  db->addGlobalSymbol(
      "sym3", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 1, 0), PackedVersion(2, 2, 0), false));

  auto *class1 = db->addObjectiveCClass(
      "Class1", "NSObject", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      class1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      class1, "sel2", /*isInstanceMethod=*/true, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      class1, "sel1", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(3, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      class1, "sel2", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(4, 0, 0), PackedVersion(), false));
  db->addObjectiveCClass(
      "Class2", "NSObject", /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));

  auto *cat1 = db->addObjectiveCCategory(
      "Category1", "Class1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      cat1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      cat1, "sel2", /*isInstanceMethod=*/true, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      cat1, "sel1", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(3, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      cat1, "sel2", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(4, 0, 0), PackedVersion(), false));
  db->addObjectiveCCategory(
      "Category2", "Class1", /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));

  auto *proto1 = db->addObjectiveCProtocol(
      "Protocol1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      proto1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      proto1, "sel2", /*isInstanceMethod=*/true, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      proto1, "sel1", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(3, 0, 0), PackedVersion(), false));
  db->addObjectiveCMethod(
      proto1, "sel2", /*isInstanceMethod=*/false, /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(4, 0, 0), PackedVersion(), false));
  db->addObjectiveCProtocol(
      "Protocol2", /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db->takeError());

  SmallString<2048> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "symbols:         \n"
                         "  - name:            sym1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "  - name:            sym2\n"
                         "    access:          private\n"
                         "    availability:    n/a\n"
                         "  - name:            sym3\n"
                         "    access:          public\n"
                         "    availability:    1.1..2.2\n"
                         "classes:         \n"
                         "  - name:            Class1\n"
                         "    super-class:     NSObject\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "    methods:         \n"
                         "      - name:            sel1\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    3\n"
                         "      - name:            sel1\n"
                         "        kind:            instance\n"
                         "        access:          public\n"
                         "        availability:    1\n"
                         "      - name:            sel2\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    4\n"
                         "      - name:            sel2\n"
                         "        kind:            instance\n"
                         "        access:          private\n"
                         "        availability:    2\n"
                         "  - name:            Class2\n"
                         "    super-class:     NSObject\n"
                         "    access:          private\n"
                         "    availability:    2\n"
                         "categories:      \n"
                         "  - name:            Category1\n"
                         "    extends:         Class1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "    methods:         \n"
                         "      - name:            sel1\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    3\n"
                         "      - name:            sel1\n"
                         "        kind:            instance\n"
                         "        access:          public\n"
                         "        availability:    1\n"
                         "      - name:            sel2\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    4\n"
                         "      - name:            sel2\n"
                         "        kind:            instance\n"
                         "        access:          private\n"
                         "        availability:    2\n"
                         "  - name:            Category2\n"
                         "    extends:         Class1\n"
                         "    access:          private\n"
                         "    availability:    2\n"
                         "protocols:       \n"
                         "  - name:            Protocol1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "    methods:         \n"
                         "      - name:            sel1\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    3\n"
                         "      - name:            sel1\n"
                         "        kind:            instance\n"
                         "        access:          public\n"
                         "        availability:    1\n"
                         "      - name:            sel2\n"
                         "        kind:            class\n"
                         "        access:          private\n"
                         "        availability:    4\n"
                         "      - name:            sel2\n"
                         "        kind:            instance\n"
                         "        access:          private\n"
                         "        availability:    2\n"
                         "  - name:            Protocol2\n"
                         "    access:          private\n"
                         "    availability:    2\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, SimpleMerge) {
  Registry registry = setupRegistry();

  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  db1->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  db2->addGlobalSymbol("sym2", /*isPublic=*/false, AvailabilityInfo(true));
  db2->addGlobalSymbol(
      "sym3", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 1, 0), PackedVersion(2, 2, 0), false));

  db1->merge(std::move(*db2));
  EXPECT_FALSE(db1->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db1.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "symbols:         \n"
                         "  - name:            sym1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "  - name:            sym2\n"
                         "    access:          private\n"
                         "    availability:    n/a\n"
                         "  - name:            sym3\n"
                         "    access:          public\n"
                         "    availability:    1.1..2.2\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, UpdateMerge) {
  Registry registry = setupRegistry();

  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  db1->addGlobalSymbol("sym1", /*isPublic=*/false, AvailabilityInfo());

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  db2->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));

  db1->merge(std::move(*db2));
  EXPECT_FALSE(db1->takeError());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  auto err = registry.writeFile(os, db1.get());
  EXPECT_FALSE(err);

  const char *expected = "--- !tapi-sdkdb-v1\n"
                         "install-name:    /usr/lib/libtest.dylib\n"
                         "access:          public\n"
                         "symbols:         \n"
                         "  - name:            sym1\n"
                         "    access:          public\n"
                         "    availability:    1\n"
                         "...\n";
  EXPECT_STREQ(expected, buffer.c_str());
}

TEST(SDKDB, ConflictMerge) {
  Registry registry = setupRegistry();

  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  db1->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  db2->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(4, 0, 0), PackedVersion(), false));

  db1->merge(std::move(*db2));

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  logAllUnhandledErrors(db1->takeError(), os, "");

  const char *error =
      "Failed to combine availability i:2 o:0 u:0 and i:4 o:0 u:0 for sym1\n";

  EXPECT_STREQ(error, buffer.c_str());
}

TEST(SDKDB, VerifySuccess) {
  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  db1->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  db1->addGlobalSymbol("sym2", /*isPublic=*/false, AvailabilityInfo(true));
  db1->addGlobalSymbol("sym3", /*isPublic=*/true, AvailabilityInfo(true));
  auto *class1 = db1->addObjectiveCClass(
      "Class1", "NSObject", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db1->addObjectiveCMethod(
      class1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  auto *cat1 = db1->addObjectiveCCategory(
      "Category1", "Class1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db1->addObjectiveCMethod(
      cat1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  auto *proto1 = db1->addObjectiveCProtocol(
      "Protocol1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db1->addObjectiveCMethod(
      proto1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db1->takeError());

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  // Same symbol.
  db2->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  // From private to public.
  db2->addGlobalSymbol("sym2", /*isPublic=*/true, AvailabilityInfo(true));
  // From no availablity to availablity.
  db2->addGlobalSymbol(
      "sym3", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  // Entirely new symbol.
  db2->addGlobalSymbol(
      "sym4", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  // Check objc metadata verificaion.
  class1 = db2->addObjectiveCClass(
      "Class1", "NSObject", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db2->addObjectiveCMethod(
      class1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  cat1 = db2->addObjectiveCCategory(
      "Category1", "Class1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db2->addObjectiveCMethod(
      cat1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  proto1 = db2->addObjectiveCProtocol(
      "Protocol1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db2->addObjectiveCMethod(
      proto1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db2->takeError());

  auto errors = db2->verifySDKDBFile(db1.get());

  EXPECT_FALSE(errors);
}

TEST(SDKDB, VerifyAPIRegression) {
  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  db1->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db1->takeError());

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  db2->addGlobalSymbol(
      "sym1", /*isPublic=*/false,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db2->takeError());

  auto err = db2->verifySDKDBFile(db1.get());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  logAllUnhandledErrors(std::move(err), os, "");

  const char *error = "API sym1 becomes SPI\n";

  EXPECT_STREQ(error, buffer.c_str());
}

TEST(SDKDB, VerifyAvailabilityRegression) {
  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  db1->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db1->takeError());

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  db2->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(true));
  EXPECT_FALSE(db2->takeError());

  auto err = db2->verifySDKDBFile(db1.get());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  logAllUnhandledErrors(std::move(err), os, "");

  const char *error =
      "AvailabilityInfo is different for sym1: i:2 o:0 u:0 and i:0 o:0 u:1\n";

  EXPECT_STREQ(error, buffer.c_str());
}

TEST(SDKDB, VerifyAPIMissing) {
  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  db1->addGlobalSymbol(
      "sym1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(2, 0, 0), PackedVersion(), false));
  EXPECT_FALSE(db1->takeError());

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  EXPECT_FALSE(db2->takeError());

  auto err = db2->verifySDKDBFile(db1.get());

  SmallString<1024> buffer;
  raw_svector_ostream os(buffer);
  logAllUnhandledErrors(std::move(err), os, "");

  const char *error = "missing C Symbol sym1\n";

  EXPECT_STREQ(error, buffer.c_str());
}

TEST(SDKDB, VerifyCategory) {
  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  auto *class1 = db1->addObjectiveCClass(
      "Class1", "NSObject", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db1->addObjectiveCMethod(
      class1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db1->addObjectiveCMethod(
      class1, "sel2", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db1->categoryMerge();
  EXPECT_FALSE(db1->takeError());

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  class1 = db2->addObjectiveCClass(
      "Class1", "NSObject", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db1->addObjectiveCMethod(class1, "sel2", /*isInstanceMethod=*/true,
                           /*isPublic=*/false, AvailabilityInfo());
  auto cat1 = db2->addObjectiveCCategory(
      "Category1", "Class1", /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  // API moved to category.
  db2->addObjectiveCMethod(
      cat1, "sel1", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  // API only annotated in category.
  db2->addObjectiveCMethod(
      cat1, "sel2", /*isInstanceMethod=*/true, /*isPublic=*/true,
      AvailabilityInfo(PackedVersion(1, 0, 0), PackedVersion(), false));
  db2->categoryMerge();
  EXPECT_FALSE(db2->takeError());

  auto errors = db2->verifySDKDBFile(db1.get());

  EXPECT_FALSE(errors);
}

TEST(SDKDB, VerifyRemoveUnavailable) {
  auto db1 = make_unique<SDKDBFile>();
  db1->setFileType(SDKDB_V1);
  db1->setInstallName("/usr/lib/libtest.dylib");
  db1->addGlobalSymbol("sym1", /*isPublic=*/true, AvailabilityInfo(true));
  EXPECT_FALSE(db1->takeError());

  auto db2 = make_unique<SDKDBFile>();
  db2->setFileType(SDKDB_V1);
  db2->setInstallName("/usr/lib/libtest.dylib");
  EXPECT_FALSE(db2->takeError());

  auto err = db2->verifySDKDBFile(db1.get());

  EXPECT_FALSE(err);
}

} // end anonymous namespace.
