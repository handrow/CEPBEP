#ifndef SOURCES_CONFIG_CONFIG_H_
#define SOURCES_CONFIG_CONFIG_H_

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <utility>


#include "common/types.h"
#include "common/error.h"


namespace Config {

enum ConfigErrorCode {
    CONF_NO_ERR = 0,
    CONF_FILE_NOT_FOUND_ERR = 20000,
};

class Category {
 private:
    typedef std::map<std::string, Category>     CategoryMap;
    typedef std::map<std::string, std::string>  FieldMap;

 public:
    typedef FieldMap::value_type                Field;
    typedef CategoryMap::iterator               SubcategoryIter;
    typedef FieldMap::iterator                  FieldsIter;

    typedef CategoryMap::const_iterator         SubcategoryConstIter;
    typedef FieldMap::const_iterator            FieldsConstIter;

    typedef std::pair<SubcategoryIter, SubcategoryIter> SubcategoryRange;
    typedef std::pair<SubcategoryConstIter, SubcategoryConstIter> SubcategoryConstRange;

    typedef std::pair<FieldsIter, FieldsIter> FieldsRange;
    typedef std::pair<FieldsConstIter, FieldsConstIter> FieldsConstRange;

 private:
    FieldMap        __fields;
    CategoryMap     __subs;
    void WriteToFile(const Category& subcat, std::ofstream& out, std::string& path) const;

 public:
    bool                  HasField(const std::string& fname) const;
    bool                  HasCategory(const std::string& cname) const;

    void                  SetField(const std::string& fname, const std::string& fvalue);
    void                  RemoveField(const std::string& fname);
    Field&                GetFieldRef(const std::string& fname);
    const Field&          GetFieldRef(const std::string& fname) const;

    std::string           GetFieldValue(const std::string& fname) const;

    void                  SetSubcategory(const std::string& cname, const Category& subcat);
    void                  RemoveSubcategory(const std::string& cname);
    Category&             GetSubcategoryRef(const std::string& cname);
    const Category&       GetSubcategoryRef(const std::string& cname) const;

    usize                 CountSubcategories() const;
    usize                 CountFields() const;

    SubcategoryRange      GetSubcatoryIterRange();
    SubcategoryConstRange GetSubcatoryIterRange() const;

    FieldsRange           GetFieldsIterRange();
    FieldsConstRange      GetFieldsIterRange() const;

    static Category       ParseFromINI(const std::string& filepath, Error *err);
    static void           DumpToINI(const Category& config_obj, const std::string& filepath, Error *err);

 private:
    static usize IsField(const std::string& str);
    static void AddField(const std::string& str, Category* cat);
    static Category* Pars(const std::string& str, Category& cat, Category* now);
    static Category* UseNewCat(const std::string& str, Category& cat);
};

}  // namespace Config


#endif  // SOURCES_CONFIG_CONFIG_H_
