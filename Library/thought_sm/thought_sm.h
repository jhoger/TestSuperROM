#ifndef THOUGHT_SM_H
#define THOUGHT_SM_H

#include <stdbool.h>

/**
 * States of the THOUGHT Outline Processor State Machine
 */
typedef enum {
    THOUGHT_STATE_PC8201A_MENU,
    THOUGHT_STATE_BASIC_PROMPT,
    THOUGHT_STATE_SUPERROM_MENU,
    THOUGHT_STATE_SUPERROM_FILE_PICK,
    THOUGHT_STATE_THOUGHT_CREATE,
    THOUGHT_STATE_THOUGHT_REVIEW,
    THOUGHT_STATE_DRAG_MODE,
    THOUGHT_STATE_SELECT_MODE,
    THOUGHT_STATE_DOCUMENT_EDIT,
    THOUGHT_STATE_THOUGHT_PRINT
} thought_state_t;

/**
 * Events that trigger state transitions
 */
typedef enum {
    /* PC8201A Menu Events */
    THOUGHT_EVENT_SELECT_BASIC,
    THOUGHT_EVENT_SELECT_SUPER,
    THOUGHT_EVENT_SELECT_CT_FILE,

    /* BASIC Prompt Events */
    THOUGHT_EVENT_EXEC_62394,

    /* SuperROM Menu Events */
    THOUGHT_EVENT_F3_THOUGHT,
    THOUGHT_EVENT_F8_EXIT,
    THOUGHT_EVENT_SHIFT_F5_EXIT,

    /* SuperROM File Pick Events */
    THOUGHT_EVENT_ENTER_NEW_FILE,
    THOUGHT_EVENT_ENTER_EXISTING_FILE,
    THOUGHT_EVENT_SHIFT_BREAK_EXIT,

    /* Thought Create Events */
    THOUGHT_EVENT_ENTER_EMPTY_LINE,
    THOUGHT_EVENT_ENTER_TEXT_LINE,
    THOUGHT_EVENT_SHIFT_F5_EMPTY,
    THOUGHT_EVENT_SHIFT_F5_TEXT,
    THOUGHT_EVENT_TYPE_DO_FILENAME,
    THOUGHT_EVENT_F2_LOAD_CASSETTE,

    /* Document Edit Events */
    THOUGHT_EVENT_SHIFT_F5_EXIT_TO_OUTLINE,
    THOUGHT_EVENT_SHIFT_BREAK_EXIT_TO_MENU,

    /* Thought Review Events */
    THOUGHT_EVENT_ENTER_LINE,
    THOUGHT_EVENT_ENTER_DOCUMENT_REF,
    THOUGHT_EVENT_PASTE,
    THOUGHT_EVENT_F5_DRAG,
    THOUGHT_EVENT_SHIFT_F2_SELECT,
    THOUGHT_EVENT_SHIFT_F1_JUMP_DOC,
    THOUGHT_EVENT_PRINT_KEY,
    THOUGHT_EVENT_SHIFT_F3_PRINT,

    /* Drag Mode Events */
    THOUGHT_EVENT_ESC_CANCEL,
    THOUGHT_EVENT_ENTER_DROP,

    /* Select Mode Events */
    THOUGHT_EVENT_SELECT_EXIT,

    /* Thought Print Events */
    THOUGHT_EVENT_F1_PRINT_GO,
    THOUGHT_EVENT_PRINT_EXECUTE
} thought_event_t;

typedef struct {
    thought_state_t current_state;
} thought_sm_t;

/**
 * Initializes the state machine to the default starting state.
 */
void thought_sm_init(thought_sm_t* sm);

/**
 * Processes an event and updates the state machine's current state.
 * Returns the new state.
 */
thought_state_t thought_sm_handle_event(thought_sm_t* sm, thought_event_t event);

#endif // THOUGHT_SM_H
