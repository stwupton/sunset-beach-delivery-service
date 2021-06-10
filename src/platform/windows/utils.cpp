#pragma once

template<typename T>
void safeRelease(T **toRelease) {
  T *toReleaseIn = (*toRelease);
  if (toReleaseIn != NULL) {
    toReleaseIn->Release();
    toReleaseIn = NULL;
  }
}