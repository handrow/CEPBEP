#include "common/string_utils.h"

#include "config/config.h"

namespace Config {

bool                  Category::HasField(const std::string& fname) const {
    FieldsConstIter element = Fields_.find(fname);
    return element != Fields_.end();
}

bool                  Category::HasCategory(const std::string& cname) const {
    SubcategoryConstIter element = SubCategories_.find(cname);
    return element != SubCategories_.end();
}

void Category::SetField(const std::string& fname, const std::string& fvalue) {
    Fields_[fname] = fvalue;
}

void Category::RemoveField(const std::string& fname) {
    Fields_.erase(fname);
}

Category::Field& Category::GetFieldRef(const std::string& fname) {
    return *Fields_.find(fname);
}

const Category::Field& Category::GetFieldRef(const std::string& fname) const {
    return *Fields_.find(fname);
}

std::string         Category::GetFieldValue(const std::string& fname) const {
    return Fields_.find(fname)->second;
}

void Category::SetSubcategory(const std::string& cname, const Category& subcat) {
    SubCategories_[cname] = subcat;
}

void Category::RemoveSubcategory(const std::string& fname) {
    SubCategories_.erase(fname);
}

Category& Category::GetSubcategoryRef(const std::string& fname) {
    return SubCategories_.find(fname)->second;
}

const Category& Category::GetSubcategoryRef(const std::string& fname) const {
    return SubCategories_.find(fname)->second;
}

USize               Category::CountSubcategories() const {
    return SubCategories_.size();
}
USize               Category::CountFields() const {
    return Fields_.size();
}

Category::SubcategoryRange    Category::GetSubcatoryIterRange() {
    return SubcategoryRange(SubCategories_.begin(), SubCategories_.end());
}

Category::SubcategoryConstRange Category::GetSubcatoryIterRange() const {
    return SubcategoryConstRange(SubCategories_.begin(), SubCategories_.end());
}

Category::FieldsRange         Category::GetFieldsIterRange() {
    return FieldsRange(Fields_.begin(), Fields_.end());
}

Category::FieldsConstRange    Category::GetFieldsIterRange() const {
    return FieldsConstRange(Fields_.begin(), Fields_.end());
}


/// PARSE

namespace {

static const char* COMMENT_SYM = ";";
static const char* CAT_OPEN_SYM = "[";
static const char* CAT_CLOSE_SYM = "]";
static const char* CAT_PATH_DELIM_SYM = ":";
static const char* ASSGINATION_SYM = "=";

}  // namespace

bool Category::IsCategory(const std::string& str) {
    USize tokBegin = str.find_first_of(CAT_OPEN_SYM);
    USize tokEnd = str.find_last_of(CAT_CLOSE_SYM);

    if (tokBegin == std::string::npos || tokEnd == std::string::npos)
        return false;

    tokBegin += 1;

    std::string raw_name = Trim(Trim(str.substr(tokBegin, tokEnd - tokBegin), ' '), '\t');

    if (raw_name.empty())
        return false;

    const std::string restricted_syms = "[]";

    for (std::string::iterator it = raw_name.begin();
                               it != raw_name.end(); ++it) {
        if (restricted_syms.find_first_of(*it) != std::string::npos)
            return false;
    }

    return true;
}

bool Category::IsField(const std::string& str) {
    std::string trimmed = Trim(Trim(str, ' '), '\t');
    USize assignSymPos = trimmed.find_first_of(ASSGINATION_SYM);

    if (assignSymPos == std::string::npos || assignSymPos == 0)
        return false;

    const std::string restricted_syms = "[]";

    for (std::string::iterator it = trimmed.begin(),
                               end = trimmed.begin() + assignSymPos;
                               it != end; ++it) {
        if (restricted_syms.find_first_of(*it) != std::string::npos)
            return false;
    }

    return true;
}

void Category::ParseField(const std::string& str, Category* cat) {
    const USize delimPos = str.find(*ASSGINATION_SYM);
    cat->SetField(Trim(str.substr(0, delimPos), ' '),
                  Trim(str.substr(delimPos + 1), ' '));
}

Category* Category::ParseLine(const std::string& str, Category* rootCategory, Category* currentCategoryLvl, bool isEmptyLine) {
    if (!isEmptyLine) {
        if (IsCategory(str)) {
            return SwitchCurrentCategory(str, rootCategory);
        } else if (IsField(str)) {
            ParseField(str, currentCategoryLvl);
        } else if (!str.empty()) {
            // TODO: Undefiend line, throw error
        }
    } else {
        currentCategoryLvl = rootCategory;
    }
    return currentCategoryLvl;
}

Category* Category::SwitchCurrentCategory(const std::string& str, Category* rootCategory) {
    Category* sub_cat = rootCategory;
    std::string rmdr = str.substr(1, str.size() - 2);  // deleting '[' and ']' from the beginning and end of the line("[server]" -> "server")

    for (USize i = rmdr.find(*CAT_PATH_DELIM_SYM);
               i != std::string::npos;
               i = rmdr.find(*CAT_PATH_DELIM_SYM)) {

        std::string sub = rmdr.substr(0, i);
        std::pair<std::string, Category> pair(sub, Category());
        sub_cat = &(sub_cat->SubCategories_.insert(pair)
                                   .first /* get iterator */
                                   ->second /* get category ref */);
        rmdr = rmdr.substr(sub.size() + 1);
    }
    sub_cat = &(sub_cat->SubCategories_.insert(std::make_pair(rmdr, Category())).first->second);

    return sub_cat;
}

Category       Category::ParseFromINI(const std::string& filepath, Error *err) {
    std::ifstream file(filepath.c_str());
    Category rootCategory;
    Category* currentCategoryLvl = &rootCategory;
    if (!file.is_open()) {
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File open failed");
        return rootCategory;
    }
    while (true) {
        std::string line;
        if (!getline(file, line))
            return rootCategory;
        
        line = Trim(Trim(line, ' '), '\t');

        bool isEmpty = line.empty();

        line = line.substr(0, line.find(*COMMENT_SYM));
        line = Trim(Trim(line, ' '), '\t');

        currentCategoryLvl = ParseLine(line,
                                           &rootCategory,
                                           currentCategoryLvl,
                                           isEmpty);
    }
}

void Category::WriteToFile(const Category& subcat, std::ofstream* out, const std::string& catpath) {
    const bool isGlobalScope = catpath.empty();

    (*out) << std::endl;
    if (!isGlobalScope) {
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
                            + (isGlobalScope ? "" : CAT_PATH_DELIM_SYM)
                            + it->first;

        subcat.WriteToFile(it->second, out, subpath);
    }
}

void           Category::DumpToINI(const Category& config_obj, const std::string& filepath, Error *err) {
    std::ofstream out(filepath.c_str());

    if (!out.is_open()) {
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File creation error");
        return;
    }
    WriteToFile(config_obj, &out);
}

}  // namespace Config
