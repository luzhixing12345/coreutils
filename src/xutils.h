#define XBOX_LOG(fmt, ...) printf("[%s]:[%4d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define XBOX_TYPE(t) #t
#define XBOX_NAME(name) _##name