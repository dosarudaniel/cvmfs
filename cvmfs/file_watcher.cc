/**
 * This file is part of the CernVM File System.
 */

#include "file_watcher.h"

#include <unistd.h>

#include "logging.h"

namespace file_watcher {

EventHandler::EventHandler() {}

EventHandler::~EventHandler() {}

FileWatcher::FileWatcher()
    : handler_map_(),
      control_pipe_(),
      started_(false) {}

FileWatcher::~FileWatcher() {
  if (started_) {
    write(control_pipe_[1], "q", 1);
    int ret = pthread_join(thread_, NULL);
    if (ret != 0) {
      LogCvmfs(kLogCvmfs, kLogDebug, "Could not join monitor thread: %d\n", ret);
    }

    close(control_pipe_[1]);
    close(control_pipe_[0]);
  }

  for (HandlerMap::iterator it = handler_map_.begin();
       it != handler_map_.end(); ++it) {
    delete it->second;
  }
}

void FileWatcher::RegisterHandler(const std::string& file_path,
                                  EventHandler* handler) {
  handler_map_[file_path] = handler;
}

bool FileWatcher::Start() {
  if (!started_) {
    int ret = pipe(control_pipe_);
    if (ret != 0) {
      LogCvmfs(kLogCvmfs, kLogDebug, "Could not create control pipe: %d\n", ret);
      return false;
    }

    ret = pthread_create(&thread_, NULL, &FileWatcher::BackgroundThread, this);
    if (ret != 0) {
      LogCvmfs(kLogCvmfs, kLogDebug, "Could not create monitor thread: %d\n", ret);
      return false;
    }

    // Before returning, wait for a start signal in the control pipe
    // from the background thread.
    char buffer[1];
    read(control_pipe_[0], buffer, 1);

    started_ = true;
    return true;
  } else {
    return false;
  }
}

void* FileWatcher::BackgroundThread(void* d) {
  FileWatcher* watcher = reinterpret_cast<FileWatcher*>(d);

  // Use the control pipe to signal readiness to the main thread,
  // before continuing with the event loop.
  write(watcher->control_pipe_[1], "s", 1);

  if (!watcher->RunEventLoop(watcher->handler_map_,
                             watcher->control_pipe_[0])) {
    LogCvmfs(kLogCvmfs, kLogDebug, "Error running event loop.");
  }

  pthread_exit(NULL);
}

}  // namespace file_watcher
