#!/usr/bin/python3

import cgi, cgitb
cgitb.enable()

form = cgi.FieldStorage()

print("Content-Type: text/html")
print("")

cgi.print_form(form)
cgi.print_environ()