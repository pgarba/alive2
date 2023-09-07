// Copyright (c) 2018-present The Alive2 Authors.
// Distributed under the MIT license that can be found in the LICENSE file.

#include "util/compiler.h"
#include "util/parallel.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;

bool fifo::init() {
#ifndef _WIN32
  ENSURE(parallel::init());
  auto fifo_filename = getenv("ALIVE_JOBSERVER_FIFO");
  if (!fifo_filename)
    return false;
  pipe_fd = open(fifo_filename, O_RDWR);
  if (pipe_fd < 0)
    return false;
  return true;
#endif
}

void fifo::getToken() {
#ifndef _WIN32
  ENSURE(read(pipe_fd, &token, 1) == 1);
#endif
}

void fifo::putToken() {
#ifndef _WIN32
  ENSURE(write(pipe_fd, &token, 1) == 1);
#endif
}

#ifndef _WIN32
tuple<pid_t, ostream *, int> fifo::limitedFork() {
  assert(pipe_fd != -1);
  return parallel::limitedFork();
}
#endif

void fifo::finishChild(bool is_timeout) {
  parallel::finishChild(is_timeout);
}

void fifo::finishParent() {
  /*
   * finishParent() basically just blocks -- we'll give up our
   * parallel execution token until it returns
   */
  putToken();
  parallel::finishParent();
  getToken();
}
