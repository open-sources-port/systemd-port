# SPDX-License-Identifier: LGPL-2.1-or-later

BEGIN {
    print "static const char* const errno_names[] = {"
}

!/(EDEADLOCK|EWOULDBLOCK|ENOTSUP)/ {
    # get numeric value of the macro
    cmd = "echo " $1 " | xargs -I{} sh -c 'echo $(({}))'"
    cmd | getline val
    close(cmd)

    # skip duplicate numeric values
    if (!(val in seen)) {
        seen[val] = 1
        printf "        [%s] = \"%s\",\n", $1, $1
    }
}

END {
    print "};"
}

