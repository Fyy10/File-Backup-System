#ifndef LISTEN_FILE_CHANGE_HPP
#define LISTEN_FILE_CHANGE_HPP

#include <string>
#include <sys/inotify.h>
#include <map>
#include <list>
#include "core.hpp"

#define MAX_EVENTS 128 // Maximum number of events to process
#define LEN_NAME 64 // Maximum length of filename, including '\0'
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) //size of one event
#define BUF_SIZE    ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME ))

using namespace std;

struct watch_roots
{
    string source_root;
    string target_root;
    FileFilter ff;
};

// a wrapper of Watcher, use watch_roots* as input
void *listen_file_change(void*);

class Watcher
{
public:
    // source_root and target_root
    Watcher(const char* source_root, const char* target_root, FileFilter = FileFilter()) : Watcher(string(source_root), string(target_root)) {}
    Watcher(const string & source_root, const string & target_root, FileFilter = FileFilter());
    ~Watcher();

    void add_watch(const char*);
    void add_watch(const string);
    void add_watch_recursive(const char*);
    void add_watch_recursive(const string);
    void handle_events();

private:
    // source root
    string source_root;
    // target root
    string target_root;
    // file descripter
    int fd;
    // watch descripters
    list<int> watches;
    // map wd to file path
    map<int, string> wd_to_path;
    // file filter
    FileFilter ff;
};

#endif // LISTEN_FILE_CHANGE_HPP