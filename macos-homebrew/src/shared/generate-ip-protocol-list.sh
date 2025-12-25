#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-2.1-or-later

set -eu
set -o pipefail

${1:?} -dM -include netinet/in.h -I../include - </dev/null | \
       awk '/^#define[ \t]+IPPROTO_[^ \t]+[ \t]+[^ \t]/ {
              name=$2
              val=$3
              # skip duplicate numeric values
              if (!seen[val]++) print name
       }' | sed -e 's/IPPROTO_//'
