#include "listen_file_change.hpp"
#include <string>
#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>

using namespace std;

Watcher::Watcher(const string & target) {
    this->target = target;

    // create file descripter
    fd = inotify_init();
    if (fd < 0) cout << "Error creating file descripter" << endl;

    // add watch
    wd = inotify_add_watch(fd, target.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
    if (wd < 0) cout << "Error adding watches" << endl;
}

Watcher::~Watcher() {
    inotify_rm_watch(fd, wd);
    close(fd);
}

void Watcher::listen() {
    cout << "Listening on " << target << endl;
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
                if (event->mask & IN_CREATE)
                {
                    if (event->mask & IN_ISDIR) cout << "Directory " << event->name << " created" << endl;
                    else cout << "File " << event->name << " created" << endl;
                }
                else if (event->mask & IN_DELETE)
                {
                    if (event->mask & IN_ISDIR) cout << "Directory " << event->name << " deleted" << endl;
                    else cout << "File " << event->name << " deleted" << endl;
                }
                else if (event->mask & IN_MODIFY)
                {
                    if (event->mask & IN_ISDIR) cout << "Directory " << event->name << " modified" << endl;
                    else cout << "File " << event->name << " modified" << endl;
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
}