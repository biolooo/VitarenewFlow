set -eu
cd "$(dirname "$0")"
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined test.c -o home-loading-test
for scenario in 1 2 3 4 5 6 7 8 9 10 11 12; do ./home-loading-test "$scenario"; done
