#!/bin/sh
case "$QUERY_STRING" in
 bzadmin|bzflag|bzfs)
  wget -O - http://cvs.sourceforge.net/viewcvs.py/*checkout\*/bzflag/bzflag/man/bzfs.6s?rev=HEAD \
  | man2html \
  | sed -e "s~^using the manual pages.<BR>$~using the manual pages from http://cvs.sourceforge.net/viewcvs.py/*checkout\*/bzflag/bzflag/man/$QUERY_STRING.6s<BR>~"
  ;;
 *)
  echo 'Content-type: text/html'
  echo ''
  echo 'Generate BZFlag man pages from SourceForge sources on the fly.'
  echo '<ul>'
  echo '<li><a href="?bzadmin">bzadmin</a>'
  echo '<li><a href="?bzflag">bzflag</a>'
  echo '<li><a href="?bzfs">bzfs</a>'
  echo '</ul>'
  ;;
esac
