#include "ninotify.h"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string.h>
#include <map>
#include <stdlib.h>
#include <unistd.h>

#include <sys/inotify.h>
#include <ev.h>

#include <v8.h>
#include <node.h>

using namespace v8;
using namespace std;
using namespace node;

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

static Handle<Object> niTarget;

struct niPair {
    string              watched_file;
    string              events_file;
};

struct {
    int                 fd;
    fd_set              rfds;
    ev_io               read_watcher;
    map<int, niPair>    ni_pairs;
} static niStruct;

static void Initialize(Handle<Object> target) {
    HandleScope scope;

    target->Set(String::NewSymbol("addWatch"), FunctionTemplate::New(AddWatch)->GetFunction());
    target->Set(String::NewSymbol("removeWatch"), FunctionTemplate::New(RemoveWatch)->GetFunction());
    target->Set(String::NewSymbol("removeAllWatches"), FunctionTemplate::New(RemoveAllWatches)->GetFunction());

    target->Set(String::New("IN_ACCESS"), Integer::New(IN_ACCESS));
    target->Set(String::New("IN_MODIFY"), Integer::New(IN_MODIFY));
    target->Set(String::New("IN_ATTRIB"), Integer::New(IN_ATTRIB));
    target->Set(String::New("IN_CLOSE_WRITE"), Integer::New(IN_CLOSE_WRITE));
    target->Set(String::New("IN_CLOSE_NOWRITE"), Integer::New(IN_CLOSE_NOWRITE));
    target->Set(String::New("IN_OPEN"), Integer::New(IN_OPEN));
    target->Set(String::New("IN_MOVED_FROM"), Integer::New(IN_MOVED_FROM));
    target->Set(String::New("IN_MOVED_TO"), Integer::New(IN_MOVED_TO));
    target->Set(String::New("IN_CREATE"), Integer::New(IN_CREATE));
    target->Set(String::New("IN_DELETE"), Integer::New(IN_DELETE));
    target->Set(String::New("IN_DELETE_SELF"), Integer::New(IN_DELETE_SELF));

    niTarget = target;
    niStruct.fd = -1;
}

static Handle<Value> AddWatch(const Arguments& args) {
    HandleScope scope;

    set<string> excluded;

    String::Utf8Value dir_path_(args[0]);
    string dir_path(*dir_path_);

    uint32_t mask = args[1]->ToUint32()->Int32Value();

    Local<Array> excluded_ = Local<Array>::Cast(args[2]->ToObject());

    for (uint32_t i = 0; i < excluded_->Length(); i++) {
        Local<Value> elem = excluded_->Get(i);
        if (elem->IsString()) {
            String::Utf8Value dir_path_name_(elem);
            excluded.insert(string(*dir_path_name_));
        }
    }

    String::Utf8Value events_file_(args[3]);
    string events_file(*events_file_);

    vector<int> wds;
    AddInotifyWatch(dir_path, mask, excluded, wds, events_file);

    return Undefined();
}

static Handle<Value> RemoveWatch(const Arguments& args) {
    HandleScope scope;

    RemoveInotifyWatch();

    return Undefined();
}

static Handle<Value> RemoveAllWatches(const Arguments& args) {
    HandleScope scope;

    RemoveAllInotifyWatches();

    return Undefined();
}

static void Callback(EV_P_ ev_io *w, int revents) {
    HandleScope scope;

    timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;
    int r = select(niStruct.fd + 1, &niStruct.rfds, NULL, NULL, 0);
    if (r < 0) {
        perror("select");
    } else if (FD_ISSET(niStruct.fd, &niStruct.rfds)) {
        char buf[BUF_LEN];
        int len, i = 0;

        len = read(niStruct.fd, buf, BUF_LEN);
        if (len < 0) {
            if (errno == EINTR) {
                printf("> errno: %d\n", errno);
            } else {
                perror("read");
            }
        }

        while (i < len) {
            struct inotify_event *event;
            event = (struct inotify_event *) &buf[i];
            string file_path = event->len ? niStruct.ni_pairs[event->wd].watched_file + "/" + event->name : niStruct.ni_pairs[event->wd].watched_file;
            ofstream events_file;
            events_file.open(niStruct.ni_pairs[event->wd].events_file.c_str(), ios::out | ios::app);
            events_file << "ninotify " << event->wd << " " << event->mask << " " << file_path << "\n";
            events_file.close();
            i += EVENT_SIZE + event->len;
        }
    }
}

void InitInotify() {
    niStruct.fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);

    FD_ZERO(&niStruct.rfds);
    FD_SET(niStruct.fd, &niStruct.rfds);

    ev_io_init(&niStruct.read_watcher, Callback, niStruct.fd, EV_READ);
    ev_io_start(EV_DEFAULT_UC_ &niStruct.read_watcher);
}

void AddInotifyWatch(string& dir_path, int mask, set<string>& excluded, vector<int>& wds, string& events_file) {
    if (niStruct.fd == -1) {
        InitInotify();
    }

    vector<string> dirs;
    dirs.push_back(dir_path);

    for (int i = 0; i < dirs.size(); i++) {
        int wd = inotify_add_watch(niStruct.fd, dirs[i].c_str(), mask);
        niPair data;
        data.events_file = events_file;
        data.watched_file = dirs[i];
        niStruct.ni_pairs[wd] = data;
        wds.push_back(wd);
    }
}

void RemoveInotifyWatch() {
    // TODO: implement
}

void RemoveAllInotifyWatches() {
    // TODO: implement
    ev_io_stop(EV_A_ &niStruct.read_watcher);
}

static int GetDirs(string& dir_path, vector<string>& sub_dirs, set<string>& excluded, bool recursive) {
    int dirs_count = 0;

    char* c_dir_path = new char[dir_path.size()+1];
    strcpy(c_dir_path, dir_path.c_str());

    DIR *dp;
    struct dirent *de;
    struct stat dir_stat;
    string full_dir_path, dir_name;

    dp = opendir(c_dir_path);
    if (dp != NULL) {
        de = readdir(dp);
        if (de != NULL) { // "."
            do {
                if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
                    dir_name = string(de->d_name);
                    full_dir_path = dir_path + "/" + dir_name;
                    lstat(full_dir_path.c_str(), &dir_stat);
                    if (S_ISDIR(dir_stat.st_mode)) {
                        if (excluded.find(dir_name) == excluded.end()) {
                            sub_dirs.push_back(full_dir_path);
                            dirs_count++;
                            if (recursive) {
                                dirs_count += GetDirs(full_dir_path, sub_dirs, excluded, recursive);
                            }
                        }
                    }
                }
            } while((de = readdir(dp)) != NULL);
        } else {
            cout << "error reading dir: " << c_dir_path << "\n";
        }
        closedir(dp);
    } else {
        cout << "error opening dir: " << c_dir_path << "\n";
    }

    return dirs_count;
}

extern "C" void init(Handle<Object> target) {
    Initialize(target);
}

