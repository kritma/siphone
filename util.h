#include <stdbool.h>

typedef enum UtilStatus {
    UTIL_STATUS_OK,
    UTIL_STATUS_ERR,
} UtilStatus;

typedef enum UtilCallState {
    UTIL_CALL_STATE_NONE,
    UTIL_CALL_STATE_CALLING,
    UTIL_CALL_STATE_CONFIRMED,
    UTIL_CALL_STATE_DISCONNECTED,
} UtilCallState;

UtilStatus util_make_call(const char *domain, const char *user,
                          const char *password, const char *recipent,
                          UtilCallState *call_state, char *errmsg, int bufsize);
void util_hangup();
void util_deinit();
UtilStatus util_init(int log_level, char *errmsg, int bufsize);
