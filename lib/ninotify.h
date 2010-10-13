#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <set>
#include <string.h>

#include <ev.h>
#include <v8.h>

using namespace v8;
using namespace std;

static void Initialize(Handle<Object> target);

static Handle<Value> AddWatch(const Arguments& args);

static Handle<Value> RemoveWatch(const Arguments& args);

static Handle<Value> RemoveAllWatches(const Arguments& args);

static void Callback(EV_P_ ev_io *w, int revents);

static int GetDirs(string& dir_path, vector<string>& sub_dirs, set<string>& excluded, bool recursive);

void InitInotify();

void AddInotifyWatch(string& dir_path, int mask, set<string>& excluded, vector<int>& wds, string& events_file);

void RemoveInotifyWatch();

void RemoveAllInotifyWatches();

