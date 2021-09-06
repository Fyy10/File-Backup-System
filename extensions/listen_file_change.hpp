#ifndef LISTEN_FILE_CHANGE_HPP
#define LISTEN_FILE_CHANGE_HPP

#include <string>
#include <sys/inotify.h>
#include <map>
#include <list>

#define MAX_EVENTS 128 // Maximum number of events to process
#define LEN_NAME 64 // Maximum length of filename, including '\0'
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
    void add_watch(const char*);
    void add_watch(const string);
    void add_watch_recursive(const char*);
    void add_watch_recursive(const string);
    void handle_events();

private:
    // file descripter
    int fd;
    // watch descripters
    list<int> watches;
    // map wd to file path
    map<int, string> wd_to_path;
};

#endif // LISTEN_FILE_CHANGE_HPP