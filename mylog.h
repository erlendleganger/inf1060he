#define MYLOGLEVEL_OFF 0
#define MYLOGLEVEL_INFO 1
#define MYLOGLEVEL_DEBUG 2
#define MYLOGLEVEL_TRACE 3

#define MYLOG(mylevel, myformat, ...) fprintf (stderr, "%s:%d:%s[%d]:%s: " myformat "\n", mylevel, getpid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define MYLOG_INFO(...) if(MYLOGLEVEL >= MYLOGLEVEL_INFO) {MYLOG("I",__VA_ARGS__);}
#define MYLOG_DEBUG(...) if(MYLOGLEVEL >= MYLOGLEVEL_DEBUG) {MYLOG("D",__VA_ARGS__);}
#define MYLOG_TRACE(...) if(MYLOGLEVEL >= MYLOGLEVEL_TRACE) {MYLOG("T",__VA_ARGS__);}
