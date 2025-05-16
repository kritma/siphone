args := `pkg-config --cflags --libs --static libpjproject openssl raylib`

build:
    clang main.c util.c {{args}} -o siphone

run:
    source .env && ./siphone

build-dev:
    bear -- just build