
#!/usr/bin/python

# import os

# a = os.getenv('AUTH_TYPE')

# print "Content-type:text/html\r\n\r\n"
# print '<html>'
# print '<head>'
# print '<title>Hello World - First CGI Program</title>'
# print '</head>'
# print '<body>'
# print '<h2>Hello World! This'
# print a
# print 'is my first CGI program</h2>'
# print '</body>'
# print '</html>'

import cgi, os

form = cgi.FieldStorage()

fileitem = form['file1']

path = "./Website/uploads/"

isExist = os.path.exists(path)

if not isExist:
    os.makedirs(path)

if fileitem.filename:
    fn = os.path.basename(fileitem.filename)
    open(path + fn, 'wb').write(fileitem.file.read())
    message = "The file '" + fn + "' was uploaded successfully with python"

else:
    message = "No file was uploaded"

print ("""\
<html lang="en">
<head>
	<title>19 WebServ | Upload </title>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.2.0-beta1/dist/css/bootstrap.min.css" rel="stylesheet"
	integrity="sha384-0evHe/X+R7YkIZDRvuzKMRqM+OrBnVFBL6DOitfPri4tjfHxaWutUpFmBp4vmVor" crossorigin="anonymous">
	<link href="https://fonts.googleapis.com/css?family=Open+Sans:300,400,700" rel="stylesheet">
	<link rel="stylesheet" href="../style.css">
</head>

<body>
	<br><h2>%s</h2>
</body>
</html>
""" % message)
