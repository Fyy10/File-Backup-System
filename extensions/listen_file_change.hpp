#ifndef LISTEN_FILE_CHANGE_HPP
#define LISTEN_FILE_CHANGE_HPP

#include <string>
#include <sys/inotify.h>

#define MAX_EVENTS 1024 // Maximum number of events to process
#define LEN_NAME 128
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) //size of one event
#define BUF_SIZE    ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME ))

using namespace std;

class Watcher
{
public:
    // the path of directory or file
    Watcher(const char* target) : Watcher(string(target)) {}
    Watcher(const string & target);
    ~Watcher();

    string target;
    void listen();

private:
    // file descripter
    int fd;
    // watch descripter
    int wd;
};

#endif // LISTEN_FILE_CHANGE_HPP