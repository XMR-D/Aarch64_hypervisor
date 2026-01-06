#ifndef LOG_H
#define LOG_H

//HERE MACRO TO PRINT LOGS
#define INFO(msg) puts("[\033[1;36mINFO\033[0m] " msg "\n")
#define WARN(msg) puts("[\033[1;33mWARN\033[0m] " msg "\n")
#define SUCCESS(msg) puts("[\033[1;32mSUCC\033[0m] " msg "\n")
#define ERROR(msg) puts("[\033[1;31mERR\033[0m] " msg "\n")

#endif /* !LOG_H */