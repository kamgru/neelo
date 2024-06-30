clang-format -i main.c
clang-format -i game.c

clang -Wall -Wextra -pedantic -lX11 main.c game.c
