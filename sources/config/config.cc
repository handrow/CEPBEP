#include "config/config.h"
#include "config/utils.h"

namespace Config {

std::string trim(const std::string& str, char delimiter) {
    usize start = 0;
    usize end = str.size() - 1;
    while (start < str.size() && str[start] == delimiter)
        start++;
    while (end > 0 && str[end] == delimiter)
        end--;
    return str.substr(start, end - (start - 1));
}

bool                  Category::HasField(const std::string& fname) const {
    FieldsConstIter element = __fields.find(fname);
    return element != __fields.end();
}

bool                  Category::HasCategory(const std::string& cname) const {
    SubcategoryConstIter element = __subs.find(cname);
    return element != __subs.end();
}

void Category::SetField(const std::string& fname, const std::string& fvalue) {
    __fields.insert(Field(fname, fvalue));
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
    __subs.insert(std::make_pair(cname, subcat));
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

usize Category::IsField(const std::string& str) {
    if (str[0] != '[')
        return 1;
    if (str[str.size() - 1] != ']')
        return 1;
    return 0;
}

void Category::AddField(const std::string& str, Category* cat) {
    usize i = str.find("=");
    cat->SetField(trim(str.substr(0, i - 1), ' '),
                    trim(str.substr(i + 1), ' '));
}

Category* Category::Pars(const std::string& str, Category& cat, Category* now) {
    if (str != "") {
        if (IsField(str)) {
            AddField(str, now);
        } else {
            now = UseNewCat(str.substr(1, str.size() - 2), cat);
        }
    }
    return now;
}

Category* Category::UseNewCat(const std::string& str, Category& cat) {
    Category* sub_cat = &cat;
    std::string rmdr = str;

    for (usize i = rmdr.find(":"); i != std::string::npos; i = rmdr.find(":")) {
        std::string sub = rmdr.substr(0, i);

        SubcategoryIter it = sub_cat->__subs.find(sub);
        if (it != sub_cat->__subs.end()) {
            sub_cat = &(it->second);
        } else {
            sub_cat = &(sub_cat->__subs.insert(std::make_pair(sub, Category())).first->second);
        }
        rmdr = rmdr.substr(sub.size() + 1);
    }
    sub_cat = &(sub_cat->__subs.insert(std::make_pair(rmdr, Category())).first->second);
    return sub_cat;
}

Category       Category::ParseFromINI(const std::string& filepath, Error *err) {
    std::ifstream file(filepath);
    Category cat;
    Category* now = &cat;
    if (!file.is_open())
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File open error");
    else
        *err = Error(CONF_NO_ERR, "No error");
    if (err->IsError())
        return cat;
    while (true) {
        std::string str;
        if (!getline(file, str))
            return cat;
        now = Pars(str.substr(0, str.find(';')), cat, now);
    }
}

void Category::WriteToFile(const Category& subcat, std::ofstream& out, std::string &path) const {
    FieldsConstRange fields_range = subcat.GetFieldsIterRange();
    out << std::endl;
    if (path.size() > 0) {
        out << "[" << path << "]" << std::endl;
    }
    for (FieldsConstIter start = fields_range.first; start != fields_range.second; start++) {
        out << start->first << " = " << start->second << std::endl;
    }
    SubcategoryConstRange subs_range = subcat.GetSubcatoryIterRange();
    for (SubcategoryConstIter start = subs_range.first; start != subs_range.second; start++) {
        if (path.size() > 0)
            path = path + ':';
        for (usize i = 0; i < start->first.size(); i++)
            path = path + start->first[i];
        subcat.WriteToFile(start->second, out, path);
        for (usize count = path.size(); path[count - 1] != ':' && count > 1; ) {
            path.pop_back();
            count = path.size();
        }
        path.pop_back();
    }
}

void           Category::DumpToINI(const Category& config_obj, const std::string& filepath, Error *err) {
    std::ofstream out(filepath);
    std::string path;

    if (!out.is_open()) {
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File creation error");
        return;
    }
    *err = Error(CONF_NO_ERR, "File creation error");
    config_obj.WriteToFile(config_obj, out, path);
}

}  // namespace Config
