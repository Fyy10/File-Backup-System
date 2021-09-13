#include "listen_file_change.hpp"
#include <string>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>
#include <string.h>
#include <pthread.h>

using namespace std;

void * listen_file_change(void * roots)
{
    struct watch_roots* source_target = (struct watch_roots*)roots;
    Watcher watcher(source_target->source_root, source_target->target_root);
    delete source_target;
    watcher.handle_events();
    return (void*)0;
}

Watcher::Watcher(const string & source_root, const string & target_root) {
    // assume absolute path
    this->source_root = source_root;
    this->target_root = target_root;
    // remove the suffix '/'
    if (this->source_root.rfind('/') == this->source_root.length() - 1) this->source_root.pop_back();
    if (this->target_root.rfind('/') == this->target_root.length() - 1) this->target_root.pop_back();

    // create file descripter
    fd = inotify_init();
    if (fd < 0) {
        cout << "Error creating file descripter" << endl;
        exit(1);
    }

    add_watch_recursive(source_root);
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
                // use rfind to preserve the source dir name
                // Example:
                // source_root: /home/jeff/data
                // target_root: /home/jeff/data_backup
                // event_path: /home/jeff/data/xxx
                // target_path: /home/jeff/data_backup/data/xxx
                const string target_path = target_root + (event_path.c_str() + source_root.rfind('/'));
                if (event->mask & IN_CREATE)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        cout << "Directory " << event_path << " created" << endl;
                        add_watch(event_path);
                        cout << "Copy directory " << event_path << " to " << target_path << endl;
                    }
                    else
                    {
                        cout << "File " << event_path << " created" << endl;
                        cout << "Copy file " << event_path << " to " << target_path << endl;
                    }
                }
                else if (event->mask & IN_DELETE)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        cout << "Directory " << event_path << " deleted" << endl;
                        cout << "Remove direcory " << target_path << endl;
                    }
                    else
                    {
                        cout << "File " << event_path << " deleted" << endl;
                        cout << "Remove file " << target_path << endl;
                    }
                }
                else if (event->mask & IN_MODIFY)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        cout << "Directory " << event_path << " modified" << endl;
                        cout << "Update directory " << event_path << " to " << target_path << endl;
                    }
                    else
                    {
                        cout << "File " << event_path << " modified" << endl;
                        cout << "Update file " << event_path << " to " << target_path << endl;
                    }
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