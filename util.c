#include "util.h"
#include "pj/log.h"
#include "pj/string.h"
#include "pj/types.h"
#include <pjsua-lib/pjsua.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void on_call_state(pjsua_call_id call_id, pjsip_event *e) {
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);
    UtilCallState *state = pjsua_call_get_user_data(call_id);
    switch (ci.state) {

    case PJSIP_INV_STATE_NULL:
        *state = UTIL_CALL_STATE_NONE;
        break;
    case PJSIP_INV_STATE_CALLING:
        *state = UTIL_CALL_STATE_CALLING;
        break;
    case PJSIP_INV_STATE_CONFIRMED:
        *state = UTIL_CALL_STATE_CONFIRMED;
        break;
    case PJSIP_INV_STATE_DISCONNECTED:
        *state = UTIL_CALL_STATE_DISCONNECTED;
        break;
    default:
        break;
    }
}

static void on_call_media_state(pjsua_call_id call_id) {
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
        // When media is active, connect call to sound device.
        pjsua_conf_connect(ci.conf_slot, 0);
        pjsua_conf_connect(0, ci.conf_slot);
    }
}

UtilStatus util_make_call(const char *domain, const char *user,
                          const char *password, const char *recipent,
                          UtilCallState *call_state, char *errmsg,
                          int bufsize) {
    pjsua_acc_id acc_id;
    pj_status_t status;

    char recipent_url[200] = {};

    sprintf(recipent_url, "sip:%s@%s", recipent, domain);

    status = pjsua_verify_url(recipent_url);
    if (status != PJ_SUCCESS) {
        char msg[100] = {};
        pj_strerror(status, msg, 100);
        sprintf(errmsg, "Invalid URL in argv: %s", msg);
        return UTIL_STATUS_ERR;
    }

    /* Register to SIP server by creating SIP account. */
    {
        pjsua_acc_config cfg;

        pjsua_acc_config_default(&cfg);

        char cfg_id[200] = {};
        sprintf(cfg_id, "sip:%s@%s", user, domain);

        char reg_uri[200] = {};
        sprintf(reg_uri, "sip:%s", domain);

        cfg.id = pj_str(cfg_id);
        cfg.reg_uri = pj_str(reg_uri);
        cfg.cred_count = 1;
        cfg.cred_info[0].realm = pj_str("*");
        cfg.cred_info[0].scheme = pj_str("digest");
        cfg.cred_info[0].username = pj_str(user);
        cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
        cfg.cred_info[0].data = pj_str(password);
        cfg.allow_sdp_nat_rewrite = PJ_TRUE;
        cfg.allow_contact_rewrite = PJ_TRUE;

        status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
        if (status != PJ_SUCCESS) {
            char msg[100] = {};
            pj_strerror(status, msg, 100);
            sprintf(errmsg, "Error adding account: %s", msg);
            return UTIL_STATUS_ERR;
        }
    }

    pj_str_t uri = pj_str(recipent_url);

    status = pjsua_call_make_call(acc_id, &uri, 0, call_state, NULL, NULL);
    if (status != PJ_SUCCESS) {
        char msg[100] = {};
        pj_strerror(status, msg, 100);
        sprintf(errmsg, "Error making call: %s", msg);
        return UTIL_STATUS_ERR;
    }

    return UTIL_STATUS_OK;
}

UtilStatus util_init(int log_level, char *errmsg, int bufsize) {
    pj_status_t status;

    status = pjsua_create();
    if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, bufsize);
        return UTIL_STATUS_ERR;
    }

    /* Init pjsua */
    {
        pjsua_config cfg;
        pjsua_logging_config log_cfg;

        pjsua_config_default(&cfg);
        cfg.cb.on_call_media_state = &on_call_media_state;
        cfg.cb.on_call_state = &on_call_state;

        pjsua_logging_config_default(&log_cfg);
        log_cfg.console_level = log_level;

        pjsua_media_config media_cfg;
        pjsua_media_config_default(&media_cfg);
        media_cfg.enable_ice = PJ_TRUE;

        char *stun_server = getenv("STUN_SERVER");
        if (stun_server && strlen(stun_server)) {
            cfg.stun_srv_cnt = 1; // Number of STUN servers
            cfg.stun_srv[0] = pj_str(stun_server);
            PJ_LOG(1, (__FILE__, "STUN configured"));
        } else {
            PJ_LOG(1, (__FILE__, "No STUN config provided"));
        }

        char *turn_server = getenv("TURN_SERVER");
        char *turn_username = getenv("TURN_USERNAME");
        char *turn_password = getenv("TURN_USERNAME");

        if (turn_server && strlen(turn_server) && turn_username &&
            strlen(turn_username) && turn_password && strlen(turn_password)) {
            media_cfg.enable_turn = PJ_TRUE;
            media_cfg.turn_server = pj_str(turn_server);
            media_cfg.turn_auth_cred.type = PJ_STUN_AUTH_CRED_STATIC;
            media_cfg.turn_conn_type = PJ_TURN_TP_UDP;
            media_cfg.turn_auth_cred.data.static_cred.username =
                pj_str(turn_server);
            media_cfg.turn_auth_cred.data.static_cred.data_type =
                PJ_STUN_PASSWD_PLAIN;
            media_cfg.turn_auth_cred.data.static_cred.data =
                pj_str(turn_server);
            media_cfg.turn_auth_cred.data.static_cred.realm = pj_str("*");
            PJ_LOG(1, (__FILE__, "TURN configured"));
        } else {
            PJ_LOG(1, (__FILE__, "No TURN config provided"));
        }

        status = pjsua_init(&cfg, &log_cfg, &media_cfg);
        if (status != PJ_SUCCESS) {
            char msg[100] = {};
            pj_strerror(status, msg, 100);
            sprintf(errmsg, "Error in pjsua_init(): %s", msg);
            return UTIL_STATUS_ERR;
        }
    }

    /* Add UDP transport. */
    {
        pjsua_transport_config cfg;

        pjsua_transport_config_default(&cfg);
        cfg.port = 5060;
        status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
        if (status != PJ_SUCCESS) {
            char msg[100] = {};
            pj_strerror(status, msg, 100);
            sprintf(errmsg, "Error creating transport: %s", msg);
            return UTIL_STATUS_ERR;
        }
    }

    status = pjsua_start();
    if (status != PJ_SUCCESS) {
        char msg[100] = {};
        pj_strerror(status, msg, 100);
        sprintf(errmsg, "Error starting pjsua: %s", msg);
        return UTIL_STATUS_ERR;
    }

    return UTIL_STATUS_OK;
}

void util_deinit() { pjsua_destroy(); }

void util_hangup() { pjsua_call_hangup_all(); }
