use CGI;

my $q = CGI->new;

my $lightweight_fh = $q->upload('file1');
my $filename = $q->param('file1');

my $dir = './Website/uploads/';

my $msg;

mkdir($dir) unless (-d $dir);

# undef may be returned if it's not a valid file handle
if (defined $lightweight_fh) {
	# Upgrade the handle to one compatible with IO::Handle:
	my $io_handle = $lightweight_fh->handle;

	open (OUTFILE, '>', $dir . $filename);
	while ($bytesread = $io_handle->read($buffer, 1024)) {
		print OUTFILE $buffer;
	}
	$msg = 'The file \'' . $filename . '\' was uploaded successfully with perl';
}
else {
	$msg = 'No file was uploaded';
}

print '<html lang="en">
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
	<br><h2>' . $msg . '</h2></body></html>';