#include "mocks.h"
#include "constraint.h"
#include "reporter.h"
#include "vector.h"
#include "assertions.h"
#include <stdlib.h>
#include <string.h>

typedef struct _RecordedResult {
    const char *function;
    intptr_t result;
    int should_keep;
} RecordedResult;

typedef struct _RecordedExpectation {
    const char *function;
    const char *test_file;
    int test_line;
    int should_keep;
} RecordedExpectation;

typedef struct _UnwantedCall {
    const char *test_file;
    int test_line;
    const char *function;
} UnwantedCall;

static Vector *result_queue = NULL;
static Vector *expectation_queue = NULL;
static Vector *unwanted_calls = NULL;

static void ensure_result_queue_exists();
static void ensure_expectation_queue_exists();
static void ensure_unwanted_calls_list_exists();
RecordedResult *find_result(const char *function);
static void unwanted_check(const char *function, const char *parameter);

void _checked_integer(const char *check_file, int check_line, const char *function, const char *parameter, intptr_t integer) {
    unwanted_check(function, parameter);
}

void _checked_string(const char *check_file, int check_line, const char *function, const char *parameter, const char *string) {
    unwanted_check(function, parameter);
}

intptr_t _stubbed_result(const char *function, const char *file, int line) {
    RecordedResult *result = find_result(function);
    if (result == NULL) {
        (*get_test_reporter()->assert_true)(
                get_test_reporter(),
                file,
                line,
                0,
                "No return value set for function [%s]", function);
        return 0;
    }
    intptr_t value = result->result;
    if (! result->should_keep) {
        free(result);
    }
    return value;
}

void _expect(const char *function, const char *test_file, int test_line, ...) {
    ensure_expectation_queue_exists();
    RecordedExpectation *expectation = (RecordedExpectation *)malloc(sizeof(RecordedExpectation));
    expectation->function = function;
    expectation->test_file = test_file;
    expectation->test_line = test_line;
    expectation->should_keep = 0;
    vector_add(expectation_queue, expectation);
}

void _expect_always(const char *function, const char *test_file, int test_line, ...) {
}

void _expect_never(const char *function, const char *test_file, int test_line) {
    ensure_unwanted_calls_list_exists();
    UnwantedCall *unwanted = (UnwantedCall *)malloc(sizeof(UnwantedCall));
    unwanted->test_file = test_file;
    unwanted->test_line = test_line;
    unwanted->function = function;
    vector_add(unwanted_calls, unwanted);
}

void _will_return(const char *function, intptr_t result) {
    ensure_result_queue_exists();
    RecordedResult *record = (RecordedResult *)malloc(sizeof(RecordedResult));
    record->function = function;
    record->result = result;
    record->should_keep = 0;
    vector_add(result_queue, record);
}

void _always_return(const char *function, intptr_t result) {
    ensure_result_queue_exists();
    RecordedResult *record = (RecordedResult *)malloc(sizeof(RecordedResult));
    record->function = function;
    record->result = result;
    record->should_keep = 1;
    vector_add(result_queue, record);
}

void clear_mocks() {
    if (result_queue != NULL) {
        destroy_vector(result_queue);
    }
    if (expectation_queue != NULL) {
        destroy_vector(expectation_queue);
    }
    if (unwanted_calls != NULL) {
        destroy_vector(unwanted_calls);
    }
}

void tally_mocks(TestReporter *reporter) {
    clear_mocks();
}

static void ensure_result_queue_exists() {
    if (result_queue == NULL) {
        result_queue = create_vector(&free);
    }
}

static void ensure_expectation_queue_exists() {
    if (expectation_queue == NULL) {
        expectation_queue = create_vector(&free);
    }
}

static void ensure_unwanted_calls_list_exists() {
    if (unwanted_calls == NULL) {
        unwanted_calls = create_vector(&free);
    }
}

RecordedResult *find_result(const char *function) {
    int i;
    for (i = 0; i < vector_size(result_queue); i++) {
        RecordedResult *result = (RecordedResult *)vector_get(result_queue, i);
        if (strcmp(result->function, function) == 0) {
            if (! result->should_keep) {
                return vector_remove(result_queue, i);
            }
            return result;
        }
    }
    return NULL;
}

static void unwanted_check(const char *function, const char *parameter) {
    int i;
    for (i = 0; i < vector_size(unwanted_calls); i++) {
        UnwantedCall *unwanted = vector_get(unwanted_calls, i);
        if (strcmp(unwanted->function, function) == 0) {
            (*get_test_reporter()->assert_true)(
                    get_test_reporter(),
                    unwanted->test_file,
                    unwanted->test_line,
                    0,
                    "Unexpected call %s(..., %s, ...)", function, parameter);
        }
    }
}
