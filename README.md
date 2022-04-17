# Text File Comparator

File Comparator program IFCOMP, is text file comparator for IBM
OS/VS compatable systems. IFCOMP accepts as input two text files
and produces listing of differences in pseudo-update form.
IFCOMP is very useful in monitoring changes made to software at
the source code level.

```
Document ID             19820000365
Document Type           Other - NASA Tech Brief
Authors                 Kotler, R. S. (Intermetrics, Inc.)
Date Acquired           August 10, 2013
Publication Date        May 1, 1983
Publication Information Publication: NASA Tech Briefs
                        Volume: 7
                        Issue: 2
                        ISSN: 0145-319X
Subject Category        MATHEMATICS AND INFORMATION SCIENCES
Report/Patent Number    MSC-20276
Distribution Limits     Public
Copyright               Work of the US Gov. Public Use Permitted.
```

[https://ntrs.nasa.gov/citations/19820000365](https://ntrs.nasa.gov/citations/19820000365)

# Build

On Mac:
```
$ brew install cmocka
$ make
```

On Ubuntu:
```
$ sudo apt install libcmocka-dev
$ make
```

Run tests:
```
$ make test
```
