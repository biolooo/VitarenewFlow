set -eu
cd "$(dirname "$0")"
SOURCE=../../launcher/source
test -f "$SOURCE/twl_repair.c" || SOURCE=.
cc -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -fsanitize=undefined -fno-sanitize-recover=all -Imock -I"$SOURCE" test.c "$SOURCE/twl_advice.c" -o twl-test
./twl-test
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Imock -I"$SOURCE" input_test.c "$SOURCE/twl_advice.c" -o input-test
./input-test
