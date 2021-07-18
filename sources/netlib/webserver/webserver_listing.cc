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
    u64          date;
    bool         subdir;
};

typedef std::list<DirEntry>  DirEntriesList;

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
        << "td.size_item { text-align: right; width: 10%; } </style>"
        << "</head><body>"
        << "<div class=wrapper>"
        << "<h1>Index of /" << title << "</h1><table>";

    for (DirEntriesList::const_iterator it = entries.begin();
                                        it != entries.end();
                                        ++it) {
        std::string resname = it->filename + (it->subdir == true ? "/" : "");
        str << "<tr>"
            << "<td class=name_item><a href=\"" << resname << "\">" << resname << "</a></td>"
            << "<td class=date_item>" << it->date << "</td>"
            << "<td class=size_item>" << it->size << "</td>"
            << "</tr>";
    }

    str << "</table></div></body></html>";

    return str.str();
}

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

    ss->http_writer.Header().Set("Content-type", "text/html");
    ss->http_writer.Write(GenerateHtmlListing(ss->http_req.uri.path, entries));

    return ss->res_code = 200, __OnHttpResponse(ss);
}

}  // namespace Webserver
