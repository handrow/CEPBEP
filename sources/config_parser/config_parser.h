#ifndef CONFIG_PARSER_CONFIG_PARSER_H_
# define CONFIG_PARSER_CONFIG_PARSER_H_

#include "config_utils.h"
#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <cstdio>

namespace ConfigParser {

class Category_v2 {

    void SetField(const std::string& fname, const std::string& fvalue);
    

};

class Category {

 public:
    public:
    typedef Category                                Category;
    typedef std::pair<std::string, std::string>     Field;
    typedef std::map<std::string, std::string>      FieldMap;
    typedef std::list<Category>                     CategoryList;

 private:
    std::string                         __name;
    FieldMap                            __fields;
    std::list<Category>                 __categories;

    std::vector<std::string>            FileReading(std::string file_name);
    std::map<std::string, std::string>  ParserFields(std::vector<std::string> config, size_t start);
    explicit Category(const std::vector<std::string> &conf, int start);

 public:
    explicit Category(const std::string& file_name);
	~Category();
    // Category &operator=(const Category &other);

    FieldMap::iterator             GetFieldIterBegin();
    CategoryList::iterator        GetCategoryIterBegin();

    void                        SetField(const std::string &key, const std::string &value);
    void                        RemoveField(const std::string &key);

    Category&                   GetCategoryRef(const std::string &key);
    const Category              &GetCategoryRef(const std::string &key) const;
    bool                        HasCategory(const std::string &key);
    bool                        HasValue(const std::string &key);

    Field                       field(const std::string &key);
    Field::second_type          &FieldValue(Field &field);
    const Field::first_type     &FieldKey(Field &field);

    std::string                 &Name();
    void                        AddField();
    void                        AddCategory(const Category &new_category);
    void                        RemoveCategory();
    size_t                      CountFields();
    size_t                      CountCategories();
};

void    dump_to_ini(Category conf, std::string path);
Category  parse_from_ini(const std::string &__fileName);

class FieldIterator {
 private:
    Category::FieldMap::iterator __ptr;
 public:
    Category::Field               &operator*();
    FieldIterator               &operator=(const FieldIterator &other);
    FieldIterator               &operator=(const Category::FieldMap::iterator &other);
    std::string                 &Value();
    const std::string           &Key();
    void                        Next();
    void                        Prev();
    bool                        IsEnd();
    bool                        IsRend();
};

class CategoryIterator {
 private:
    Category::CategoryList::iterator __ptr;
 public:
    Category::Category            &operator*();
    CategoryIterator            &operator=(const CategoryIterator &other);
    CategoryIterator            &operator=(const Category::CategoryList::iterator &other);
    std::string                 &Name();
    void                        Next();
    void                        Prev();
    bool                        IsEnd();
    bool                        IsRend();
};

};  // namespace Config_parser

#endif  // CONFIG_PARSER_CONFIG_PARSER_H_