#! /bin/sh
#
# provide $(MAKE) with values for linux KVERSION and INCLUDE.
# 
# a file ./+linux-info may be specified containing assignments to
# variables `kver' and `linux' about the kernel version and source tree
# directory.

set -e

prog=linux-info

if test -f +linux-info; then
    . +linux-info
else
    # kernel version to be used
    kver=

    # linux source directory to be used
    linux=/dev/null

    # remove when properly filled in the above values
fi

wd=`pwd`
if test -d "$linux"; then
    cd $linux
    if test -f MAINTAINERS && test -d include/linux && test -d arch; then :
    else
	echo "$prog: Seems not to be a linux source directory: $linux" >&2
	exit 1
    fi
else
    echo "$prog: Not a directory: $linux" >&2
    exit 1
fi

# wd is `$linux'



# print some information for the developer
uts_release=`cat include/linux/version.h | sed -n '/#.*fine *UTS_RELEASE/s,.*UTS_REL.* ",,p' | sed 's, *" *$,,'`


case $1 in
    --kversion)
	echo $kver
	;;
    -I)
	echo $linux/include
	;;
    *)
cat <<EOF >&2

  ,--------------------------------------------------------------.
  | Using kernel source tree                                     |
  |
  |   $linux
  |
  |
  | The module will be compiled for kernel 
  |
  |   $kver,
  |
  |
  | UTS_RELEASE:
  |
  |   $uts_release
  |
  |                                                              |
  \`--------------------------------------------------------------'


EOF
;;
esac




#
# arch-tag: M.T. Mo Jan 31 18:48:41 CET 2005
#