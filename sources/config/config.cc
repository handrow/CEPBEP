#include "common/string_utils.h"

#include "config/config.h"

namespace Config {

bool                  Category::HasField(const std::string& fname) const {
    FieldsConstIter element = __fields.find(fname);
    return element != __fields.end();
}

bool                  Category::HasCategory(const std::string& cname) const {
    SubcategoryConstIter element = __subs.find(cname);
    return element != __subs.end();
}

void Category::SetField(const std::string& fname, const std::string& fvalue) {
    __fields[fname] = fvalue;
}

void Category::RemoveField(const std::string& fname) {
    __fields.erase(fname);
}

Category::Field& Category::GetFieldRef(const std::string& fname) {
    return *__fields.find(fname);
}

const Category::Field& Category::GetFieldRef(const std::string& fname) const {
    return *__fields.find(fname);
}

std::string         Category::GetFieldValue(const std::string& fname) const {
    return __fields.find(fname)->second;
}

void Category::SetSubcategory(const std::string& cname, const Category& subcat) {
    __subs[cname] = subcat;
}

void Category::RemoveSubcategory(const std::string& fname) {
    __subs.erase(fname);
}

Category& Category::GetSubcategoryRef(const std::string& fname) {
    return __subs.find(fname)->second;
}

const Category& Category::GetSubcategoryRef(const std::string& fname) const {
    return __subs.find(fname)->second;
}

usize               Category::CountSubcategories() const {
    return __subs.size();
}
usize               Category::CountFields() const {
    return __fields.size();
}

Category::SubcategoryRange    Category::GetSubcatoryIterRange() {
    return SubcategoryRange(__subs.begin(), __subs.end());
}

Category::SubcategoryConstRange Category::GetSubcatoryIterRange() const {
    return SubcategoryConstRange(__subs.begin(), __subs.end());
}

Category::FieldsRange         Category::GetFieldsIterRange() {
    return FieldsRange(__fields.begin(), __fields.end());
}

Category::FieldsConstRange    Category::GetFieldsIterRange() const {
    return FieldsConstRange(__fields.begin(), __fields.end());
}


/// PARSE

namespace {

static const char* COMMENT_SYM = ";";
static const char* CAT_OPEN_SYM = "[";
static const char* CAT_CLOSE_SYM = "]";
static const char* CAT_PATH_DELIM_SYM = ":";
static const char* ASSGINATION_SYM = "=";

}  // namespace

bool Category::IsField(const std::string& str) {
    if (str[0] != *CAT_OPEN_SYM)
        return 1;
    if (str[str.size() - 1] != *CAT_CLOSE_SYM)
        return 1;
    return 0;
}

void Category::ParseField(const std::string& str, Category* cat) {
    const usize delim_pos = str.find(*ASSGINATION_SYM);
    cat->SetField(Trim(str.substr(0, delim_pos), ' '),
                  Trim(str.substr(delim_pos + 1), ' '));
}

Category* Category::ParseLine(const std::string& str, Category* root_category, Category* current_category_level) {
    if (str.size() != 0) {
        if (IsField(str)) {
            ParseField(str, current_category_level);
        } else {
            return SwitchCurrentCategory(str, root_category);
        }
    } else {
        current_category_level = root_category;
    }
    return current_category_level;
}

Category* Category::SwitchCurrentCategory(const std::string& str, Category* root_category) {
    Category* sub_cat = root_category;
    std::string rmdr = str.substr(1, str.size() - 2);  // deleting '[' and ']' from the beginning and end of the line("[server]" -> "server")

    for (usize i = rmdr.find(*CAT_PATH_DELIM_SYM);
               i != std::string::npos;
               i = rmdr.find(*CAT_PATH_DELIM_SYM)) {

        std::string sub = rmdr.substr(0, i);
        std::pair<std::string, Category> pair(sub, Category());
        sub_cat = &(sub_cat->__subs.insert(pair)
                                   .first /* get iterator */
                                   ->second /* get category ref */);
        rmdr = rmdr.substr(sub.size() + 1);
    }
    sub_cat = &(sub_cat->__subs.insert(std::make_pair(rmdr, Category())).first->second);

    return sub_cat;
}

Category       Category::ParseFromINI(const std::string& filepath, Error *err) {
    std::ifstream file(filepath);
    Category root_category;
    Category* current_category_level = &root_category;
    if (!file.is_open()) {
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File open failed");
        return root_category;
    }
    while (true) {
        std::string str;
        if (!getline(file, str))
            return root_category;
        current_category_level = ParseLine(str.substr(0, str.find(*COMMENT_SYM)/*remove comment*/), &root_category, current_category_level);
    }
}

void Category::WriteToFile(const Category& subcat, std::ofstream* out, const std::string& catpath) {
    const bool is_global_space = catpath.empty();

    (*out) << std::endl;
    if (!is_global_space) {
        (*out) << CAT_OPEN_SYM
               << catpath
               << CAT_CLOSE_SYM << std::endl;
    }

    FieldsConstRange fields = subcat.GetFieldsIterRange();
    for (FieldsConstIter it = fields.first;
                         it != fields.second; ++it) {
        (*out) << it->first << " "
               << ASSGINATION_SYM << " "
               << it->second << std::endl;
    }

    SubcategoryConstRange subs = subcat.GetSubcatoryIterRange();
    for (SubcategoryConstIter it = subs.first;
                              it != subs.second; ++it) {
        std::string subpath = catpath
                            + (is_global_space ? "" : CAT_PATH_DELIM_SYM)
                            + it->first;

        subcat.WriteToFile(it->second, out, subpath);
    }
}

void           Category::DumpToINI(const Category& config_obj, const std::string& filepath, Error *err) {
    std::ofstream out(filepath);

    if (!out.is_open()) {
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File creation error");
        return;
    }
    WriteToFile(config_obj, &out);
}

}  // namespace Config
