#ifndef CONFIG_PARSER_CONFIG_PARSER_H_
# define CONFIG_PARSER_CONFIG_PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cstdio>

namespace Ft {
namespace ConfigParser {

class Config {

 private:
    std::string&                        __name;
    std::map<std::string, std::string> __fields;
    std::map<std::string, Config>      __categories;

 public:
    typedef Config                                Category;
    typedef std::pair<std::string, std::string>   Field;
    typedef std::map<std::string, std::string>    FieldType;
    typedef std::map<std::string, Config>         CategoryType;

    explicit Config();
	~Config();
    Config &operator=(const Config &other);

    FieldType::iterator             GetFieldIterBegin();
    CategoryType::iterator        GetCategoryIterBegin();

    void                        SetField(const std::string &key, const std::string &value);
    void                        RemoveField(const std::string &key);

    Category                    Category(const std::string &key);
    bool                        HasCategory(const std::string &key);
    bool                        HasValue(const std::string &key);

    Field                       Field(const std::string &key);
    Field::second_type          &FieldValue(Field &field);
    const Field::first_type     &FieldKey(Field &field);

    std::string                 &name();
    void                        AddField();
    void                        AddCategory();
    void                        RemoveCategory();
    size_t                      CountFields();
    size_t                      CountCategories();
};

void    dump_to_ini(Config conf, std::string path);
Config  parse_from_ini(const std::string &__fileName);

class FieldIterator {
 private:
    Config::FieldType::iterator __ptr;
 public:
    Config::Field               &operator*();
    FieldIterator               &operator=(const FieldIterator &other);
    FieldIterator               &operator=(const Config::FieldType::iterator &other);
    std::string                 &Value();
    const std::string           &Key();
    void                        Next();
    void                        Prev();
    bool                        IsEnd();
    bool                        IsRend();
};

class CategoryIterator {
 private:
    Config::CategoryType::iterator __ptr;
 public:
    Config::Category            &operator*();
    CategoryIterator            &operator=(const CategoryIterator &other);
    CategoryIterator            &operator=(const Config::CategoryType::iterator &other);
    std::string                 &Name();
    void                        Next();
    void                        Prev();
    bool                        IsEnd();
    bool                        IsRend();
};

};  // namespace Ft

};  // namespace Config_parser

#endif  // CONFIG_PARSER_CONFIG_PARSER_H_