# SPDX-License-Identifier: LGPL-2.1-or-later

BEGIN {
    print "static const char* const ip_protocol_names[] = { "
}

{
    # get numeric value
    cmd = "cpp -dM -include netinet/in.h < /dev/null | grep -w " $1 " | awk '{print $3}'"
    cmd | getline val
    close(cmd)

    if (!seen[val]++) {
        printf "        [%s] = \"%s\",\n", $1, tolower($1)
    }
}

END {
    print "};"
}
