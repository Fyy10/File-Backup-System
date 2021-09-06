#include "listen_file_change.hpp"

int main(int argc, char *argv[]) {
    Watcher watch(argv[1]);
    watch.handle_events();
    return 0;
}