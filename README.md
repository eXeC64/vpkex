vpkex
=====

`vpkex` is a command line tool to extract resources from Valve's VPK archive.

Usage: `vpkex <archive> [file]`

`archive` must be a `_dir.vpk` vpk file. If `file` is given it will be extracted
from archive and output to `stdout`. Otherwise, the list of files in the archive
will be listed to `stdout`.
