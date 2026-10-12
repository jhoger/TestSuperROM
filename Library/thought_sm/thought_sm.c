#include "thought_sm.h"

void thought_sm_init(thought_sm_t* sm) {
    if (sm) {
        sm->current_state = THOUGHT_STATE_PC8201A_MENU;
    }
}

thought_state_t thought_sm_handle_event(thought_sm_t* sm, thought_event_t event) {
    if (!sm) return THOUGHT_STATE_PC8201A_MENU;

    thought_state_t next_state = sm->current_state;

    switch (sm->current_state) {
        case THOUGHT_STATE_PC8201A_MENU:
            if (event == THOUGHT_EVENT_SELECT_BASIC) {
                next_state = THOUGHT_STATE_BASIC_PROMPT;
            } else if (event == THOUGHT_EVENT_SELECT_SUPER) {
                next_state = THOUGHT_STATE_SUPERROM_MENU;
            } else if (event == THOUGHT_EVENT_SELECT_CT_FILE) {
                next_state = THOUGHT_STATE_THOUGHT_CREATE;
            }
            break;

        case THOUGHT_STATE_BASIC_PROMPT:
            if (event == THOUGHT_EVENT_EXEC_62394) {
                next_state = THOUGHT_STATE_SUPERROM_MENU;
            }
            break;

        case THOUGHT_STATE_SUPERROM_MENU:
            if (event == THOUGHT_EVENT_F3_THOUGHT) {
                next_state = THOUGHT_STATE_SUPERROM_FILE_PICK;
            } else if (event == THOUGHT_EVENT_SHIFT_F5_EXIT || event == THOUGHT_EVENT_F8_EXIT) {
                next_state = THOUGHT_STATE_PC8201A_MENU;
            }
            break;

        case THOUGHT_STATE_SUPERROM_FILE_PICK:
            if (event == THOUGHT_EVENT_ENTER_NEW_FILE) {
                next_state = THOUGHT_STATE_THOUGHT_CREATE;
            } else if (event == THOUGHT_EVENT_ENTER_EXISTING_FILE) {
                next_state = THOUGHT_STATE_THOUGHT_REVIEW;
            } else if (event == THOUGHT_EVENT_SHIFT_F5_EXIT || event == THOUGHT_EVENT_SHIFT_BREAK_EXIT) {
                next_state = THOUGHT_STATE_SUPERROM_MENU;
            }
            break;

        case THOUGHT_STATE_THOUGHT_CREATE:
            if (event == THOUGHT_EVENT_ENTER_EMPTY_LINE || event == THOUGHT_EVENT_SHIFT_F5_EMPTY) {
                next_state = THOUGHT_STATE_THOUGHT_REVIEW;
            } else if (event == THOUGHT_EVENT_TYPE_DO_FILENAME) {
                next_state = THOUGHT_STATE_DOCUMENT_EDIT;
            } else if (event == THOUGHT_EVENT_F2_LOAD_CASSETTE) {
                next_state = THOUGHT_STATE_THOUGHT_CREATE;
            }
            break;

        case THOUGHT_STATE_DOCUMENT_EDIT:
            if (event == THOUGHT_EVENT_SHIFT_F5_EXIT_TO_OUTLINE) {
                next_state = THOUGHT_STATE_THOUGHT_REVIEW;
            } else if (event == THOUGHT_EVENT_SHIFT_BREAK_EXIT_TO_MENU) {
                next_state = THOUGHT_STATE_PC8201A_MENU;
            }
            break;

        case THOUGHT_STATE_THOUGHT_REVIEW:
            if (event == THOUGHT_EVENT_ENTER_LINE || event == THOUGHT_EVENT_PASTE) {
                next_state = THOUGHT_STATE_THOUGHT_CREATE;
            } else if (event == THOUGHT_EVENT_SHIFT_F5_EXIT) {
                next_state = THOUGHT_STATE_PC8201A_MENU;
            } else if (event == THOUGHT_EVENT_F5_DRAG) {
                next_state = THOUGHT_STATE_DRAG_MODE;
            } else if (event == THOUGHT_EVENT_SHIFT_F2_SELECT) {
                next_state = THOUGHT_STATE_SELECT_MODE;
            } else if (event == THOUGHT_EVENT_ENTER_DOCUMENT_REF || event == THOUGHT_EVENT_SHIFT_F1_JUMP_DOC) {
                next_state = THOUGHT_STATE_DOCUMENT_EDIT;
            } else if (event == THOUGHT_EVENT_PRINT_KEY || event == THOUGHT_EVENT_SHIFT_F3_PRINT) {
                next_state = THOUGHT_STATE_THOUGHT_PRINT;
            }
            break;

        case THOUGHT_STATE_DRAG_MODE:
            if (event == THOUGHT_EVENT_ENTER_DROP || event == THOUGHT_EVENT_ESC_CANCEL) {
                next_state = THOUGHT_STATE_THOUGHT_REVIEW;
            }
            break;

        case THOUGHT_STATE_SELECT_MODE:
            if (event == THOUGHT_EVENT_SELECT_EXIT || event == THOUGHT_EVENT_ESC_CANCEL || event == THOUGHT_EVENT_SHIFT_F5_EXIT) {
                next_state = THOUGHT_STATE_THOUGHT_REVIEW;
            }
            break;

        case THOUGHT_STATE_THOUGHT_PRINT:
            if (event == THOUGHT_EVENT_SHIFT_F5_EXIT || event == THOUGHT_EVENT_SHIFT_BREAK_EXIT || event == THOUGHT_EVENT_ESC_CANCEL) {
                next_state = THOUGHT_STATE_THOUGHT_REVIEW;
            } else if (event == THOUGHT_EVENT_F1_PRINT_GO || event == THOUGHT_EVENT_PRINT_EXECUTE) {
                next_state = THOUGHT_STATE_THOUGHT_REVIEW;
            }
            break;
    }

    sm->current_state = next_state;
    return next_state;
}
