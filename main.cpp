#define MAX_EVENTS  1024     // number of events at time
#define LEN_NAME    40
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))

#include <iostream>
#include <fstream>
#include <string>
#include <sys/inotify.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
    const string log_path = "../log.txt"; 
    char buffer[BUF_LEN];

    int inotify_descriptor = inotify_init();
    if (inotify_descriptor < 0)
    {
        cerr << "Couldn't initialize inotify\n";
    }
    char *path = argv[1];
    
    int watch_descriptor = inotify_add_watch(inotify_descriptor, path,
        IN_CREATE | IN_MODIFY | IN_DELETE);

    if (watch_descriptor == -1)
    {
        cerr << "Could not watch to " << path << "\n";
        return 1;
    }
    else
    {
        cout << "Watching changes in dir: " << path << "\n";
    }

    while(true)
    {
        ofstream output(log_path, ios::app);
        if (!output.is_open())
        {
            cerr << "Could not open log file\n";
        }

        int i = 0;
        int length = read(inotify_descriptor, buffer, BUF_LEN);
        if (length < 0)
        {
            cerr << "Could not read events\n";
        }

        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];

            if (event->len)
            {
                if (event->mask & IN_CREATE)
                {
                    if (event->mask & IN_ISDIR)
                        output << "The directory " << event->name << " was created.\n";
                    else
                        output << "The file " << event->name << " was created\n";
                }

                if (event->mask & IN_DELETE)
                {
                    if (event->mask & IN_ISDIR)
                        output << "The directory " << event->name << " was deleted.\n";
                    else
                        output << "The file " << event->name << " was deleted\n";
                }

                if (event->mask & IN_MODIFY) 
                {
                    if (event->mask & IN_ISDIR)
                        output << "The directory " << event->name << " was modified.\n";
                    else
                        output << "The file " << event->name << " was modified\n";
                }
                
                i += EVENT_SIZE + event->len;
                
            }
        }
        output.close();
    }
    
    inotify_rm_watch(inotify_descriptor, watch_descriptor);
    close(inotify_descriptor);
    
    return 0;
}
