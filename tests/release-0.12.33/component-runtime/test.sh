set -eu
cd "$(dirname "$0")"
SOURCE=../../launcher/source
test -f "$SOURCE/component_catalog.c" || SOURCE=.
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Imock -I"$SOURCE" test.c "$SOURCE/component_catalog.c" -o runtime-test
./runtime-test
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -I"$SOURCE" input_test.c -o input-test
./input-test
