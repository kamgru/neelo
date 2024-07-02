clang-format -i main.c
clang-format -i game.c

clang -Wall -Wextra -pedantic -lX11 -lXrandr main.c game.c
