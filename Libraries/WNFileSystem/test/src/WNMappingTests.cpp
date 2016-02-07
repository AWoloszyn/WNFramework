// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WNFileSystem/inc/WNMapping.h"
#include "WNFileSystem/inc/WNFactory.h"
#include "WNTesting/inc/WNTestHarness.h"

using mapping_test = ::testing::TestWithParam<wn::file_system::mapping_type>;

TEST_P(mapping_test, creation) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);
  }
}

TEST_P(mapping_test, create_file) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);

    wn::file_system::result r;
    const wn::file_system::file_ptr fp = mp->create_file("temp.txt", r);

    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(r, wn::file_system::result::ok);

    fp->close();

    EXPECT_FALSE(fp->is_open());
  }
}

TEST_P(mapping_test, create_directory) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);

    const wn::file_system::result r = mp->create_directory("temp");

    EXPECT_EQ(r, wn::file_system::result::ok);
  }
}

TEST_P(mapping_test, exists_file) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);

    wn::file_system::result r;
    const wn::file_system::file_ptr fp = mp->create_file("temp.txt", r);

    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(r, wn::file_system::result::ok);

    fp->close();

    EXPECT_FALSE(fp->is_open());
    EXPECT_TRUE(mp->exists_file("temp.txt"));
    EXPECT_FALSE(mp->exists_file("notafile.txt"));
  }
}

TEST_P(mapping_test, exists_directory) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);

    const wn::file_system::result r = mp->create_directory("temp");

    EXPECT_EQ(r, wn::file_system::result::ok);
    EXPECT_TRUE(mp->exists_directory("temp"));
    EXPECT_FALSE(mp->exists_directory("notafile.txt"));
  }
}

TEST_P(mapping_test, recursive_create_directory) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);
    const wn::file_system::result r =
        mp->recursive_create_directory("temp/foo/bar/baz");
    EXPECT_EQ(r, wn::file_system::result::ok);

    EXPECT_TRUE(mp->exists_directory("temp"));
    EXPECT_TRUE(mp->exists_directory("temp/foo"));
    EXPECT_TRUE(mp->exists_directory("temp/foo/bar"));
    EXPECT_TRUE(mp->exists_directory("temp/foo/bar/baz"));
    EXPECT_FALSE(mp->exists_directory("notafile.txt"));
  }
}

TEST_P(mapping_test, initialize_files) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);
    wn::file_system::result r = mp->initialize_files({
        {"a.txt", "a"}, {"a/b.txt", "a slash b"}, {"a/c.txt", "a slash c"},
        {"b/c/d.txt", "b slash c slash d"}, {"q.txt", "q"},
    });
    EXPECT_EQ(r, wn::file_system::result::ok);

    EXPECT_TRUE(mp->exists_directory("a"));
    EXPECT_TRUE(mp->exists_directory("b/c"));
    EXPECT_TRUE(mp->exists_file("a/b.txt"));
    EXPECT_TRUE(mp->exists_file("a.txt"));
    EXPECT_TRUE(mp->exists_file("b/c/d.txt"));
    EXPECT_TRUE(mp->exists_file("q.txt"));

    wn::file_system::file_ptr file = mp->open_file("b/c/d.txt", r);
    EXPECT_EQ(r, wn::file_system::result::ok);
    ASSERT_NE(file, nullptr);
    EXPECT_EQ(file->typed_range<char>(),
        wn::containers::string_view("b slash c slash d"));
  }
}

TEST_P(mapping_test, open_file) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);

    wn::file_system::result r;
    wn::file_system::file_ptr fp = mp->create_file("temp.txt", r);

    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(r, wn::file_system::result::ok);

    fp->close();

    EXPECT_FALSE(fp->is_open());

    fp = mp->open_file("temp.txt", r);

    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(r, wn::file_system::result::ok);

    fp->close();

    EXPECT_FALSE(fp->is_open());
  }
}

TEST_P(mapping_test, delete_file) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);

    wn::file_system::result r;
    const wn::file_system::file_ptr fp = mp->create_file("temp.txt", r);

    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(r, wn::file_system::result::ok);

    fp->close();

    EXPECT_FALSE(fp->is_open());

    r = mp->delete_file("temp.txt");

    EXPECT_EQ(r, wn::file_system::result::ok);
    EXPECT_FALSE(mp->exists_file("temp.txt"));
  }
}

TEST_P(mapping_test, delete_directory) {
  wn::testing::allocator allocator;

  {
    wn::file_system::factory f;
    const wn::file_system::mapping_ptr mp =
        f.make_mapping(GetParam(), &allocator);

    ASSERT_NE(mp, nullptr);

    wn::file_system::result r = mp->create_directory("temp");

    EXPECT_EQ(r, wn::file_system::result::ok);

    wn::file_system::file_ptr fp = mp->create_file("temp/temp1.txt", r);

    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(r, wn::file_system::result::ok);

    r = mp->create_directory("temp/temp2");

    EXPECT_EQ(r, wn::file_system::result::ok);

    fp = mp->create_file("temp/temp2/temp2.txt", r);

    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(r, wn::file_system::result::ok);

    fp->close();

    r = mp->delete_directory("temp");

    EXPECT_EQ(r, wn::file_system::result::ok);
    EXPECT_FALSE(mp->exists_directory("temp"));
  }
}

INSTANTIATE_TEST_CASE_P(all_mappings, mapping_test,
    ::testing::Values(wn::file_system::mapping_type::scratch,
                            wn::file_system::mapping_type::memory_backed));
