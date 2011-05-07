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

void InitInotify();

void AddInotifyWatch(string& path, int mask, vector<int>& wds, string& events_file);

void RemoveInotifyWatch();

void RemoveAllInotifyWatches();

