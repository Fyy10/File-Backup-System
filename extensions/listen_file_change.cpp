#include "listen_file_change.hpp"
#include <string>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>
#include <string.h>

using namespace std;

Watcher::Watcher(const string & target) {
    this->target = target;

    // create file descripter
    fd = inotify_init();
    if (fd < 0) {
        cout << "Error creating file descripter" << endl;
        exit(1);
    }

    add_watch_recursive(target);
}

Watcher::~Watcher() {
    for (list<int>::const_iterator iter = watches.begin(); iter != watches.end(); iter++)
    {
        inotify_rm_watch(fd, *iter);
    }
    close(fd);
}

void Watcher::add_watch(const char* target)
{
    add_watch(string(target));
}

void Watcher::add_watch_recursive(const char* target)
{
    add_watch_recursive(string(target));
}

void Watcher::add_watch(const string target)
{
    string path = target;
    if (path.rfind('/') == path.length() - 1) path.pop_back();
    // add watch (only watch directories)
    int wd = inotify_add_watch(fd, path.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE | IN_DELETE_SELF);
    if (wd < 0) {
        cout << "Error adding watches" << endl;
        exit(2);
    }

    // note: remember to remove wd from watches and the map if the file/dir is deleted (prevent removing watch twice)
    watches.push_back(wd);
    wd_to_path[wd] = path;

    cout << "Listening " << wd_to_path[wd] << endl;
}

void Watcher::add_watch_recursive(const string target)
{
    // recursively add watches in a directory (target should be a directory)
    string path = target;
    if (path.rfind('/') == path.length() - 1) path.pop_back();

    struct stat s;
    lstat(path.c_str(), &s);

    if (S_ISDIR(s.st_mode))
    {
        // add watch to current dir
        add_watch(path);

        // return value for readdir()
        struct dirent *filename;
        // return value for opendir()
        DIR *dir;

        dir = opendir(path.c_str());

        while ((filename = readdir(dir)) != NULL)
        {
            // in case of '.' and '..'
            if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0) continue;

            struct stat s;
            string next_path = path + '/' + filename->d_name;
            lstat(next_path.c_str(), &s);
            if (S_ISDIR(s.st_mode)) add_watch_recursive(next_path);
        }
    }
}

void Watcher::handle_events() {
    // read the buffer and handle the events
    while (true)
    {
        int i = 0, length;
        // events buffer
        char buffer[BUF_SIZE];

        // read buffer
        length = read(fd, buffer, BUF_SIZE);

        // process events
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            if (event->len)
            {
                const string event_path = wd_to_path[event->wd] + '/' + event->name;
                if (event->mask & IN_CREATE)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        cout << "Directory " << event_path << " created" << endl;
                        add_watch(event_path);
                    }
                    else cout << "File " << event_path << " created" << endl;
                }
                else if (event->mask & IN_DELETE)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        cout << "Directory " << event_path << " deleted" << endl;
                    }
                    else cout << "File " << event_path << " deleted" << endl;
                }
                else if (event->mask & IN_MODIFY)
                {
                    if (event->mask & IN_ISDIR) cout << "Directory " << event_path << " modified" << endl;
                    else cout << "File " << event_path << " modified" << endl;
                }
                else if (event->mask & IN_DELETE_SELF)
                {
                    watches.remove(event->wd);
                    wd_to_path.erase(event->wd);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
}