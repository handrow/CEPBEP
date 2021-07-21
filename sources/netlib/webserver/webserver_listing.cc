#include <string>
#include <list>
#include <sys/stat.h>

#include <dirent.h>

#include "netlib/webserver/webserver.h"

namespace Webserver {
namespace {

struct DirEntry {
    std::string  filename;
    usize        size;
    time_t       date;
    bool         subdir;
};

typedef std::list<DirEntry>  DirEntriesList;

static const char* __FMT_SIZE_STR[] = {
    "B",
    "KiB",
    "MiB",
    "GiB",
    "TiB",
    "PiB",
};

enum {
    __FMT_SIZE_B = 0,
    __FMT_SIZE_KIB,
    __FMT_SIZE_MIB,
    __FMT_SIZE_GIB,
    __FMT_SIZE_TIB,
    __FMT_SIZE_PIB,
};

std::string  FileSizeToStr(usize bytes) {
    int fsi = __FMT_SIZE_B;
    float fsz = static_cast<float>(bytes);

    while (fsz >= 1000.0f && fsi < __FMT_SIZE_PIB) {
        fsz /= 1024.0f;
        fsi += 1;
    }

    std::stringstream  ss;
    std::string        units = __FMT_SIZE_STR[fsi];

    if (fsz >= 100.0f)
        ss.precision(0);
    else 
        ss.precision(1);

    ss << std::fixed << fsz << " ";

    ss << std::setfill(' ') << std::setw(3) << std::left;
    ss << units;
    return ss.str();
}

std::string  GenerateHtmlListing(const std::string& title, const DirEntriesList& entries) {
    std::stringstream str;

    str << "<html><head>"
        << "<style> body { margin: 0; }"
        << ".wrapper { margin: 0 0 0 15px; min-width: 512px; max-width: 960px; }"
        << "h1 { display: block; margin: 20px 0px; }"
        << "table { border-color: black; border-width: 1px 0px; border-style: solid; font-family: monospace;"
        << "white-space: nowrap; display: block; padding: 20px 0px; }"
        << "td { display: table-cell; }"
        << "td.name_item { padding-right: 24px; width: 65%; }"
        << "td.date_item { padding-right: 24px; width: 20%; }"
        << "pre { display: inline; }"
        << "td.size_item { text-align: right; width: 10%; } </style>"
        << "</head><body>"
        << "<div class=wrapper>"
        << "<h1>Index of " << title << "</h1><table>";

    for (DirEntriesList::const_iterator it = entries.begin();
                                        it != entries.end();
                                        ++it) {
        std::string resname = it->filename + (it->subdir == true ? "/" : "");
        str << "<tr>"
            << "<td class=name_item><a href=\"" << resname << "\">" << resname << "</a></td>"
            << "<td class=date_item>" << FormatTimeToStr("%d-%b-%Y %k:%M", it->date) << "</td>"
            << "<td class=size_item><pre>" << (!it->subdir ? FileSizeToStr(it->size) : "") << "</pre></td>"
            << "</tr>";
    }

    str << "</table></div></body></html>";

    return str.str();
}

struct DirentryCompType {
    bool  operator()(const DirEntry& de1, const DirEntry& de2) const {
        return de1.subdir > de2.subdir;
    }
};

struct DirentryCompName {
    bool  operator()(const DirEntry& de1, const DirEntry& de2) const {
        return de1.filename < de2.filename;
    }
};

}  // namespace

void  HttpServer::__SendDirectoryListing(const std::string& resource_path, SessionCtx* ss) {
    DirEntriesList  entries;
    struct dirent   entry;
    struct dirent*  is_end;
    DIR*            dp = NULL;

    dp = opendir(resource_path.c_str());
    if (dp == NULL)
        return ss->res_code = 500, __OnHttpError(ss);

    while (readdir_r(dp, &entry, &is_end), is_end != NULL) {
        struct stat     st;
        DirEntry        dentry;

        dentry.filename = std::string(entry.d_name, entry.d_namlen);
        if (dentry.filename == ".." || dentry.filename == ".")
            continue;

        std::string path = resource_path + dentry.filename;
        int res = stat(path.c_str(), &st);
        if (res < 0)
            continue;
        
        dentry.date = st.st_mtimespec.tv_sec;
        dentry.subdir = bool(S_ISDIR(st.st_mode));
        dentry.size = st.st_size;
        
        entries.push_back(dentry);
    }

    entries.sort(DirentryCompName());
    entries.sort(DirentryCompType());

    ss->http_writer.Header().Set("Content-type", "text/html");
    ss->http_writer.Write(GenerateHtmlListing(ss->http_req.uri.path, entries));

    return ss->res_code = 200, __OnHttpResponse(ss);
}

}  // namespace Webserver
