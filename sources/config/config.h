#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

#include <string>
#include <map>
#include <fstream>

#include "common/types.h"
#include "common/error.h"


namespace Config {

enum ConfigErrorCode {
    CONF_NO_ERR = 0,
    CONF_FILE_NOT_FOUND_ERR = 20000,
};

class Category {
 private:
    // TODO: use list
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
    FieldMap        Fields_;
    CategoryMap     SubCategories_;

 private:
    static void           WriteToFile(const Category& subcat, std::ofstream* out, const std::string& catprefix = "");
    static bool           IsField(const std::string& str);
    static bool           IsCategory(const std::string& str);
    static void           ParseField(const std::string& str, Category* cat);
    static Category*      ParseLine(const std::string& str, Category* rootCategory, Category* currentCategoryLevel, bool isEmptyLine);
    static Category*      SwitchCurrentCategory(const std::string& str, Category* root_category);

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

    USize                 CountSubcategories() const;
    USize                 CountFields() const;

    SubcategoryRange      GetSubcatoryIterRange();
    SubcategoryConstRange GetSubcatoryIterRange() const;

    FieldsRange           GetFieldsIterRange();
    FieldsConstRange      GetFieldsIterRange() const;

    static Category       ParseFromINI(const std::string& filepath, Error *err);
    static void           DumpToINI(const Category& configObj, const std::string& filepath, Error *err);
};

}  // namespace Config


#endif  // CONFIG_CONFIG_H_
