
#pragma once

#define DELETE_COPY_AND_COPY_ASSIGN_CONSTRUCTOR(className)\
  className(const className&) = delete;\
  className &operator=(const className&) = delete

#define DEFAULT_COPY_AND_COPY_ASSIGN_CONSTRUCTOR(className)\
  className(const className&) = default;\
  className &operator=(const className&) = default

// A macro to disallow operator=
// This should be used in the private: declarations for a class.
#define DEFAULT_MOVE_AND_MOVE_ASSIGN_CONSTRUCTOR(className)\
  className &operator=(className &&) = default;\
  className(className&&) = default

#define ALLOW_MOVE_SEMANTICS_ONLY(className)\
  DEFAULT_MOVE_AND_MOVE_ASSIGN_CONSTRUCTOR(className);\
  DELETE_COPY_AND_COPY_ASSIGN_CONSTRUCTOR(className)

#define IS_NUMBER_POWER_OF_2(value) ((value != 0) && !(value & (value - 1)))
