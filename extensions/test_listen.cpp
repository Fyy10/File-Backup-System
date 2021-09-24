#include "listen_file_change.hpp"

int main(int argc, char *argv[]) {
    string passwd = "123456";
    Watcher watch(argv[1], argv[2], passwd);
    watch.handle_events();
    return 0;
}