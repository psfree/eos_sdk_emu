#! /bin/bash

FILE="$1"
OUT_FILE="$2"
define="^#define EOS_.*_LATEST .+$"
struct="^EOS_STRUCT\((.*), ($"

awk '
function ltrim(s) { sub(/^[ \t\r\n]+/, "", s); return s }
function rtrim(s) { sub(/[ \t\r\n]+$/, "", s); return s }
function trim(s)  { return rtrim(ltrim(s)); }
BEGIN{version=0}
{
  if ( $0 ~ /'"$define"'/ ) {
    version=gensub(/^#define.*LATEST ([0-9]+)/, "\\1", 0);
    version=trim(version);
    print gensub(/LATEST ([0-9]+)/, "00\\1 \\1", 0);
  } else if ( $0 ~ /^EOS_STRUCT/ && $0 !~ /Callback/ ) {
    print gensub(/\((.*),/, "(\\100"version",", 0);
  } else if ( $0 ~ /^#include "eos_common.h"/ ) {
    getline
  } else {
    print
  }
}
' "$FILE" >"$OUT_FILE"

egrep "^EOS_STRUCT.*00." "$OUT_FILE" | sed -r 's/EOS_STRUCT\(([^,]+)([0-9]{3,3}),.*/#define \1 \1\2/'

echo
echo "#include <$OUT_FILE>"
echo

egrep "^#define EOS_.*00. " "$OUT_FILE" | sed -r 's/#define (EOS_[a-zA-Z_0-9]+_)([0-9]+).*/#define \1LATEST \1\2/'
