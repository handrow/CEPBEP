#include <fstream>
#include <iostream>

#include "gtest/gtest.h"
#include "config/config.h"

TEST(Config_Tests, has_field) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.HasField("server_name") == true);
        EXPECT_TRUE(cat.HasField("listen") == true);

        EXPECT_TRUE(cat.HasField("root") == false);
    }
}

TEST(Config_Tests, has_category) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.HasCategory("server1") == true);
        EXPECT_TRUE(cat.HasCategory("server2") == true);
        EXPECT_TRUE(cat.HasCategory("location") == false);
    }
}

TEST(Config_Tests, set_field) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.HasField("root") == false);
        cat.SetField("root", "../CEPBEP");
        EXPECT_TRUE(cat.HasField("root") == true);
    }
}

TEST(Config_Tests, remove_field) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.HasField("listen") == true);
        cat.RemoveField("listen");
        EXPECT_TRUE(cat.HasField("listen") == false);
    }
}

TEST(Config_Tests, get_field_ref) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.GetFieldRef("server_name").first == "server_name");
        EXPECT_TRUE(cat.GetFieldRef("server_name").second == "youpi");
    }
}

TEST(Config_Tests, get_field_value) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.GetFieldValue("server_name") == "youpi");
    }
}

TEST(Config_Tests, set_subcategory) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.HasCategory("server3") == false);
        Category New;
        cat.SetSubcategory("server3", New);
        EXPECT_TRUE(cat.HasCategory("server3") == true);
    }
}

TEST(Config_Tests, remove_subcategory) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.HasCategory("server2") == true);
        cat.RemoveSubcategory("server2");
        EXPECT_TRUE(cat.HasCategory("server2") == false);
    }
}

TEST(Config_Tests, get_subcategory_ref) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.GetSubcategoryRef("server1").HasCategory("location") == true);
        EXPECT_TRUE(cat.GetSubcategoryRef("server1").HasCategory("location /post_body") == true);
    }
}

TEST(Config_Tests, count_subcategories) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.CountSubcategories() == 2);
        Category New;
        cat.SetSubcategory("server3", New);
        EXPECT_TRUE(cat.CountSubcategories() == 3);
    }
}

TEST(Config_Tests, count_fields) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(cat.CountFields() == 2);
        cat.SetField("root", "../CEPBEP");
        EXPECT_TRUE(cat.CountFields() == 3);
    }
}

TEST(Config_Tests, get_subcatory_iter_range) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);


        EXPECT_TRUE(err.IsOk());

        Category::SubcategoryRange srange = cat.GetSubcategoryRef("server1").GetSubcatoryIterRange();
        std::vector<std::string> checking;
        usize count = 0;
        checking.push_back("location");
        checking.push_back("location *.bla");
        checking.push_back("location /directory");
        checking.push_back("location /post_body");

        for (Category::SubcategoryIter start = srange.first; start != srange.second; start++) {
            EXPECT_TRUE(start->first == checking[count++]);
        }
    }
}

TEST(Config_Tests, get_fields_iter_range) {
    using namespace Config;
    {
        Error err(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);


        EXPECT_TRUE(err.IsOk());

        Category cat2 = cat.GetSubcategoryRef("server1").GetSubcategoryRef("location /directory");
        Category::FieldsRange frange = cat2.GetFieldsIterRange();
        std::vector<std::string> checking;
        usize count = 0;
        checking.push_back("allow_methods");
        checking.push_back("index");
        checking.push_back("root");

        for (Category::FieldsIter start = frange.first; start != frange.second; start++) {
            EXPECT_TRUE(start->first == checking[count++]);
        }
    }
}

TEST(Config_Tests, dump_to_ini) {
    using namespace Config;
    {
        Error err(0, "No error");
        Error err_ini(0, "No error");
        Category cat = Category::ParseFromINI("../config_examples/default.ini", &err);
        Category::DumpToINI(cat, "../config_examples/Test.ini", &err_ini);

        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(err_ini.IsOk());

        std::ifstream verifiable("../config_examples/Test.ini");
        std::ifstream testing("../config_examples/Checking_file.ini");
        std::string sverifiable;
        std::string stesting;

        EXPECT_TRUE(verifiable.is_open() && testing.is_open());

        while (getline(verifiable, sverifiable) && getline(testing, stesting)) {
            EXPECT_TRUE(sverifiable == stesting);
        }
    }
}
