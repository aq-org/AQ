// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_LOGGING_LOGGING_H_
#define AQ_LOGGING_LOGGING_H_

#include <iostream>
#include <string>

#define LOGGING_ERROR(message)                                               \
  {                                                                          \
    std::cerr << "[ERROR] " << __FUNCTION__ << ": " << message << std::endl; \
    exit(EXIT_FAILURE);                                                      \
  }

#define LOGGING_WARNING(message) \
  { std::cerr << "[WARNING] " << __FUNCTION__ << ": " << message << std::endl; }

#define LOGGING_INFO(message) \
  { std::cerr << "[INFO] " << __FUNCTION__ << ": " << message << std::endl; }

#define INTERNAL_ERROR(message)                                             \
  {                                                                         \
    std::cerr << "[[[INTERNAL ERROR]]] " << __FUNCTION__ << ": " << message \
              << std::endl;                                                 \
    exit(EXIT_FAILURE);                                                     \
  }

#endif