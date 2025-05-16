#include <math.h>
#include <stdint.h>
#include <stdio.h>
#define RAYGUI_IMPLEMENTATION

#include "raygui.h"
#include "util.h"
#include <stdlib.h>

int main() {
    UtilStatus status;
    char err_msg[200] = {};

    status = util_init(1, err_msg, 200);

    SetTraceLogLevel(LOG_ERROR);

    if (status == UTIL_STATUS_ERR) {
        InitWindow(400, 200, "siphone");
        SetTargetFPS(60);
        while (!WindowShouldClose()) {

            BeginDrawing();
            ClearBackground(BLACK);
            if (1 == GuiMessageBox((Rectangle){0, 0, 400, 200}, "error",
                                   err_msg, "exit")) {
                CloseWindow();
                break;
            }
            EndDrawing();
        }
        return 1;
    }

    InitWindow(600, 400, "siphone");
    SetTargetFPS(60);

    char sip_domain[50] = {};
    TextCopy(sip_domain, getenv("SIP_DOMAIN"));

    char sip_user[50] = {};
    TextCopy(sip_user,
             getenv("SIP_USERNAME") ? getenv("SIP_USERNAME") : "+123456789");

    char sip_password[50] = {};
    TextCopy(sip_password,
             getenv("SIP_PASSWORD") ? getenv("SIP_PASSWORD") : "");

    char sip_recipent[50] = {};
    TextCopy(sip_recipent,
             getenv("SIP_RECIPIENT") ? getenv("SIP_RECIPIENT") : "+123456789");

    bool domain_edit = false;
    bool user_edit = false;
    bool password_edit = false;
    bool recipent_edit = false;

    bool show_err_message = false;

    UtilCallState call_state = UTIL_CALL_STATE_NONE;
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (show_err_message) {
            if (1 == GuiMessageBox((Rectangle){0, 0, 600, 400}, "error",
                                   err_msg, "ok")) {
                show_err_message = false;
                continue;
            }
            EndDrawing();
            continue;
        }

        GuiLabel((Rectangle){15, 10, 250, 40}, "sip domain");
        if (GuiTextBox((Rectangle){10, 40, 250, 40}, sip_domain, 50,
                       domain_edit)) {
            domain_edit = !domain_edit;
        }

        GuiLabel((Rectangle){15, 100, 250, 40}, "sip user");
        if (GuiTextBox((Rectangle){10, 130, 250, 40}, sip_user, 50,
                       user_edit)) {
            user_edit = !user_edit;
        }

        GuiLabel((Rectangle){15, 190, 250, 40}, "sip password");
        if (GuiTextBox((Rectangle){10, 220, 250, 40}, sip_password, 50,
                       password_edit)) {
            password_edit = !password_edit;
        }

        GuiLabel((Rectangle){15, 260, 250, 40}, "sip recipent");
        if (GuiTextBox((Rectangle){10, 300, 250, 40}, sip_recipent, 50,
                       recipent_edit)) {
            recipent_edit = !recipent_edit;
        }

        if (GuiButton((Rectangle){300, 40, 250, 40}, "#131#call")) {
            if (call_state == UTIL_CALL_STATE_NONE ||
                call_state == UTIL_CALL_STATE_DISCONNECTED) {
                status =
                    util_make_call(sip_domain, sip_user, sip_password,
                                   sip_recipent, &call_state, err_msg, 200);
                if (status == UTIL_STATUS_ERR) {
                    show_err_message = true;
                }
            }
        }

        int color_brightness = 0xff;
        uint32_t color = 0x000000ff;
        char *call_text;
        if (call_state == UTIL_CALL_STATE_NONE) {
            color_brightness = (int)(sinhf((float)GetTime() * 2) * 100 + 155);
            color |= 0xa0a0a000;
            call_text = "waiting";
        }
        if (call_state == UTIL_CALL_STATE_CALLING) {
            color_brightness = (int)(sinf((float)GetTime() * 2) * 100 + 155);
            color |= color_brightness << (2 * 4 * 2);
            color |= color_brightness << (2 * 4 * 3);
            call_text = "calling";
        }
        if (call_state == UTIL_CALL_STATE_CONFIRMED) {
            color |= color_brightness << (2 * 4 * 2);
            call_text = "speaking";
        }
        if (call_state == UTIL_CALL_STATE_DISCONNECTED) {
            color |= color_brightness << (2 * 4 * 3);
            call_text = "disconnected";
        }

        GuiLabel((Rectangle){300, 130, 250, 40}, call_text);
        DrawCircle(450, 150, 10, GetColor(color));

        if (GuiButton((Rectangle){300, 220, 250, 40}, "hangup")) {
            util_hangup();
        }

        ClearBackground(BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
