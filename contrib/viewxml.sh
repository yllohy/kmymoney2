# A bash script to view a temporary copy of a KMyMoney XML file in your favourite editor
# (works with some gnucash files too!)

# Usage:- viewxml [filename]

# Save this script somewhere in your path and remember to apply execute permissions (chmod a+x viewxml)
# Set the following variables as required
TMPDIR=/tmp # a temporary directory for storing the file copy
EDITOR=kate # your editor of choice
WIPE='rm -f' # command to get rid of the temporary file copy (could be replaced with a shredder or something)
#

declare -i TYPE

if [ -z $1 ]; then
  FILE=`kdialog --getopenfilename . '*.*'`;
else
  FILE=$1;
fi

TYPE=0 # default type, gzipped file
read -n 14 <$FILE HEAD
if [ "$HEAD" = "-----BEGIN PGP" ]; then
  TYPE=1; # encrypted file
elif [ "$HEAD" = "<?xml version=" ]; then
  TYPE=2;
fi

BASENAME=`basename $FILE`

case $TYPE in
  0) echo $BASENAME is gzipped
     cp $FILE $TMPDIR/$BASENAME.gz
     gunzip $TMPDIR/$BASENAME.gz;;
  1) echo $BASENAME is encrypted
     gpg -d $FILE >$TMPDIR/$BASENAME;;
  2) echo $BASENAME is plaintext
     cp $FILE $TMPDIR/$BASENAME;;
esac

$EDITOR $TMPDIR/$BASENAME

$WIPE $TMPDIR/$BASENAME


