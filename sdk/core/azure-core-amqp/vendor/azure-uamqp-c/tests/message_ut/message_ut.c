// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"

// TODO: Add tests for each part of the message where the value is cleared and then read

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"
#include "azure_uamqp_c/amqpvalue.h"
#include "azure_uamqp_c/amqp_definitions.h"

#undef ENABLE_MOCKS

#include "azure_uamqp_c/message.h"

TEST_DEFINE_ENUM_TYPE(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_VALUES)

static const HEADER_HANDLE test_header = (HEADER_HANDLE)0x4242;
static const HEADER_HANDLE cloned_header = (HEADER_HANDLE)0x4243;
static const delivery_annotations test_delivery_annotations = (delivery_annotations)0x4244;
static const delivery_annotations cloned_delivery_annotations = (delivery_annotations)0x4245;
static const delivery_annotations other_cloned_delivery_annotations = (delivery_annotations)0x4246;
static const message_annotations test_message_annotations = (message_annotations)0x4247;
static const message_annotations cloned_message_annotations = (message_annotations)0x4248;
static const message_annotations other_cloned_message_annotations = (message_annotations)0x4249;
static const PROPERTIES_HANDLE test_message_properties = (PROPERTIES_HANDLE)0x424A;
static const PROPERTIES_HANDLE cloned_message_properties = (PROPERTIES_HANDLE)0x4250;
static const PROPERTIES_HANDLE other_cloned_message_properties = (PROPERTIES_HANDLE)0x4251;
static const AMQP_VALUE test_application_properties = (AMQP_VALUE)0x4252;
static const AMQP_VALUE cloned_application_properties = (AMQP_VALUE)0x4253;
static const AMQP_VALUE other_cloned_application_properties = (AMQP_VALUE)0x4254;
static const annotations test_footer = (annotations)0x4255;
static const annotations cloned_footer = (annotations)0x4256;
static const annotations other_cloned_footer = (annotations)0x4257;
static const AMQP_VALUE test_amqp_value_1 = (AMQP_VALUE)0x4258;
static const AMQP_VALUE test_amqp_value_2 = (AMQP_VALUE)0x4259;
static const AMQP_VALUE cloned_amqp_value = (AMQP_VALUE)0x425A;
static const AMQP_VALUE test_sequence_1 = (AMQP_VALUE)0x425B;
static const AMQP_VALUE cloned_sequence_1 = (AMQP_VALUE)0x425C;
static const AMQP_VALUE test_sequence_2 = (AMQP_VALUE)0x425D;
static const AMQP_VALUE cloned_sequence_2 = (AMQP_VALUE)0x4260;

static const HEADER_HANDLE another_test_header = (HEADER_HANDLE)0x4261;

static TEST_MUTEX_HANDLE g_testByTest;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static void stringify_bytes(const unsigned char* bytes, size_t byte_count, char* output_string)
{
    size_t i;
    size_t pos = 0;

    output_string[pos++] = '[';
    for (i = 0; i < byte_count; i++)
    {
        (void)sprintf(&output_string[pos], "0x%02X", bytes[i]);
        if (i < byte_count - 1)
        {
            strcat(output_string, ",");
        }
        pos = strlen(output_string);
    }
    output_string[pos++] = ']';
    output_string[pos++] = '\0';
}

BEGIN_TEST_SUITE(message_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_realloc, my_gballoc_realloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    REGISTER_GLOBAL_MOCK_RETURN(header_clone, cloned_header);
    REGISTER_GLOBAL_MOCK_RETURN(annotations_clone, cloned_delivery_annotations);
    REGISTER_GLOBAL_MOCK_RETURN(properties_clone, cloned_message_properties);
    REGISTER_UMOCK_ALIAS_TYPE(HEADER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AMQP_VALUE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(PROPERTIES_HANDLE, void*);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(test_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* message_create */

/* Tests_SRS_MESSAGE_01_001: [`message_create` shall create a new AMQP message instance and on success it shall return a non-NULL handle for the newly created message instance.] */
TEST_FUNCTION(message_create_succeeds)
{
    // arrange
    MESSAGE_HANDLE message;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));

    // act
    message = message_create();

    // assert
    ASSERT_IS_NOT_NULL(message);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_001: [`message_create` shall create a new AMQP message instance and on success it shall return a non-NULL handle for the newly created message instance.] */
TEST_FUNCTION(message_create_2_times_yields_2_different_message_instances)
{
    // arrange
    MESSAGE_HANDLE message1;
    MESSAGE_HANDLE message2;

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));

    // act
    message1 = message_create();
    message2 = message_create();

    // assert
    ASSERT_IS_NOT_NULL(message1, "Creating the first message failed");
    ASSERT_IS_NOT_NULL(message2, "Creating the second message failed");
    ASSERT_ARE_NOT_EQUAL(void_ptr, message1, message2);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message1);
    message_destroy(message2);
}

/* Tests_SRS_MESSAGE_01_002: [If allocating memory for the message fails, `message_create` shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_memory_for_the_message_fails_then_message_create_fails)
{
    // arrange
    MESSAGE_HANDLE message;
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    message = message_create();

    // assert
    ASSERT_IS_NULL(message);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* message_clone */

/* Tests_SRS_MESSAGE_01_003: [`message_clone` shall clone a message entirely and on success return a non-NULL handle to the cloned message.] */
/* Tests_SRS_MESSAGE_01_005: [If a header exists on the source message it shall be cloned by using `header_clone`.] */
/* Tests_SRS_MESSAGE_01_006: [If delivery annotations exist on the source message they shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_007: [If message annotations exist on the source message they shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_008: [If message properties exist on the source message they shall be cloned by using `properties_clone`.] */
/* Tests_SRS_MESSAGE_01_009: [If application properties exist on the source message they shall be cloned by using `amqpvalue_clone`.] */
/* Tests_SRS_MESSAGE_01_010: [If a footer exists on the source message it shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_011: [If an AMQP data has been set as message body on the source message it shall be cloned by allocating memory for the binary payload.] */
TEST_FUNCTION(message_clone_with_a_message_that_has_all_fields_set_and_amqp_data_body_succeeds)
{
    // arrange
    MESSAGE_HANDLE message;
    MESSAGE_HANDLE source_message = message_create();
    unsigned char data_section[2] = { 0x42, 0x43 };
    BINARY_DATA binary_data;

    binary_data.bytes = data_section;
    binary_data.length = sizeof(data_section);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(source_message, test_header);
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(source_message, test_delivery_annotations);
    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(source_message, test_message_annotations);
    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(source_message, test_message_properties);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(source_message, test_application_properties);
    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(source_message, test_footer);
    (void)message_add_body_amqp_data(source_message, binary_data);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(header_clone(cloned_header));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations));
    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_application_properties));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(sizeof(data_section)));

    // act
    message = message_clone(source_message);

    // assert
    ASSERT_IS_NOT_NULL(message);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(source_message);
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_003: [`message_clone` shall clone a message entirely and on success return a non-NULL handle to the cloned message.] */
/* Tests_SRS_MESSAGE_01_005: [If a header exists on the source message it shall be cloned by using `header_clone`.] */
/* Tests_SRS_MESSAGE_01_006: [If delivery annotations exist on the source message they shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_007: [If message annotations exist on the source message they shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_008: [If message properties exist on the source message they shall be cloned by using `properties_clone`.] */
/* Tests_SRS_MESSAGE_01_009: [If application properties exist on the source message they shall be cloned by using `amqpvalue_clone`.] */
/* Tests_SRS_MESSAGE_01_010: [If a footer exists on the source message it shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_159: [If an AMQP value has been set as message body on the source message it shall be cloned by calling `amqpvalue_clone`. ]*/
TEST_FUNCTION(message_clone_with_a_message_that_has_all_fields_set_and_amqp_value_body_succeeds)
{
    // arrange
    MESSAGE_HANDLE message;
    MESSAGE_HANDLE source_message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(source_message, test_header);
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(source_message, test_delivery_annotations);
    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(source_message, test_message_annotations);
    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(source_message, test_message_properties);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(source_message, test_application_properties);
    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(source_message, test_footer);
    STRICT_EXPECTED_CALL(annotations_clone(test_amqp_value_1))
        .SetReturn(cloned_amqp_value);
    (void)message_set_body_amqp_value(source_message, test_amqp_value_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(header_clone(cloned_header));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations));
    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_application_properties));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_amqp_value));

    // act
    message = message_clone(source_message);

    // assert
    ASSERT_IS_NOT_NULL(message);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(source_message);
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_003: [`message_clone` shall clone a message entirely and on success return a non-NULL handle to the cloned message.] */
/* Tests_SRS_MESSAGE_01_005: [If a header exists on the source message it shall be cloned by using `header_clone`.] */
/* Tests_SRS_MESSAGE_01_006: [If delivery annotations exist on the source message they shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_007: [If message annotations exist on the source message they shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_008: [If message properties exist on the source message they shall be cloned by using `properties_clone`.] */
/* Tests_SRS_MESSAGE_01_009: [If application properties exist on the source message they shall be cloned by using `amqpvalue_clone`.] */
/* Tests_SRS_MESSAGE_01_010: [If a footer exists on the source message it shall be cloned by using `annotations_clone`.] */
/* Tests_SRS_MESSAGE_01_160: [ If AMQP sequences are set as AMQP body they shall be cloned by calling `amqpvalue_clone`. ] */
TEST_FUNCTION(message_clone_with_a_message_that_has_all_fields_set_and_amqp_sequence_body_succeeds)
{
    // arrange
    MESSAGE_HANDLE message;
    MESSAGE_HANDLE source_message = message_create();
    unsigned char data_section[2] = { 0x42, 0x43 };
    BINARY_DATA binary_data;

    binary_data.bytes = data_section;
    binary_data.length = sizeof(data_section);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(source_message, test_header);
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(source_message, test_delivery_annotations);
    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(source_message, test_message_annotations);
    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(source_message, test_message_properties);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(source_message, test_application_properties);
    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(source_message, test_footer);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    (void)message_add_body_amqp_sequence(source_message, test_sequence_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(header_clone(cloned_header));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations));
    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_application_properties));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_sequence_1));

    // act
    message = message_clone(source_message);

    // assert
    ASSERT_IS_NOT_NULL(message);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(source_message);
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_012: [ If any cloning operation for the members of the source message fails, then `message_clone` shall fail and return NULL. ]*/
TEST_FUNCTION(when_any_clone_operations_fails_message_clone_for_a_message_with_data_body_fails)
{
    // arrange
    MESSAGE_HANDLE source_message = message_create();
    unsigned char data_section[2] = { 0x42, 0x43 };
    BINARY_DATA binary_data;
    size_t count;
    size_t index;

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    binary_data.bytes = data_section;
    binary_data.length = sizeof(data_section);

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(source_message, test_header);
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(source_message, test_delivery_annotations);
    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(source_message, test_message_annotations);
    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(source_message, test_message_properties);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(source_message, test_application_properties);
    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(source_message, test_footer);
    (void)message_add_body_amqp_data(source_message, binary_data);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(header_clone(cloned_header));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations));
    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_application_properties));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(sizeof(data_section)));

    umock_c_negative_tests_snapshot();

    count = umock_c_negative_tests_call_count();
    for (index = 0; index < count - 4; index++)
    {
        MESSAGE_HANDLE message;
        char tmp_msg[128];
        (void)sprintf(tmp_msg, "Failure in test %u/%u", (unsigned int)(index + 1), (unsigned int)count);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        // act
        message = message_clone(source_message);

        // assert
        ASSERT_IS_NULL(message, tmp_msg);
    }

    // cleanup
    umock_c_negative_tests_deinit();
    message_destroy(source_message);
}

/* Tests_SRS_MESSAGE_01_012: [ If any cloning operation for the members of the source message fails, then `message_clone` shall fail and return NULL. ]*/
TEST_FUNCTION(when_any_clone_operations_fails_message_clone_for_a_message_with_value_body_fails)
{
    // arrange
    MESSAGE_HANDLE source_message = message_create();
    size_t count;
    size_t index;

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(source_message, test_header);
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(source_message, test_delivery_annotations);
    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(source_message, test_message_annotations);
    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(source_message, test_message_properties);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(source_message, test_application_properties);
    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(source_message, test_footer);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_1))
        .SetReturn(cloned_amqp_value);
    (void)message_set_body_amqp_value(source_message, test_amqp_value_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(header_clone(cloned_header));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations));
    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_application_properties));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_amqp_value));

    umock_c_negative_tests_snapshot();

    count = umock_c_negative_tests_call_count();
    for (index = 0; index < count - 4; index++)
    {
        MESSAGE_HANDLE message;
        char tmp_msg[128];
        (void)sprintf(tmp_msg, "Failure in test %u/%u", (unsigned int)(index + 1), (unsigned int)count);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        // act
        message = message_clone(source_message);

        // assert
        ASSERT_IS_NULL(message, tmp_msg);
    }

    // cleanup
    umock_c_negative_tests_deinit();
    message_destroy(source_message);
}

/* Tests_SRS_MESSAGE_01_012: [ If any cloning operation for the members of the source message fails, then `message_clone` shall fail and return NULL. ]*/
TEST_FUNCTION(when_any_clone_operations_fails_message_clone_for_a_message_with_sequence_body_fails)
{
    // arrange
    MESSAGE_HANDLE source_message = message_create();
    size_t count;
    size_t index;

    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(source_message, test_header);
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(source_message, test_delivery_annotations);
    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(source_message, test_message_annotations);
    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(source_message, test_message_properties);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(source_message, test_application_properties);
    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(source_message, test_footer);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    (void)message_add_body_amqp_sequence(source_message, test_sequence_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(header_clone(cloned_header));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations));
    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_application_properties));
    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer));
    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(cloned_sequence_1));

    umock_c_negative_tests_snapshot();

    count = umock_c_negative_tests_call_count();
    for (index = 0; index < count - 4; index++)
    {
        MESSAGE_HANDLE message;
        char tmp_msg[128];
        (void)sprintf(tmp_msg, "Failure in test %u/%u", (unsigned int)(index + 1), (unsigned int)count);

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(index);

        // act
        message = message_clone(source_message);

        // assert
        ASSERT_IS_NULL(message, tmp_msg);
    }

    // cleanup
    umock_c_negative_tests_deinit();
    message_destroy(source_message);
}

/* Tests_SRS_MESSAGE_01_062: [If `source_message` is NULL, `message_clone` shall fail and return NULL.] */
TEST_FUNCTION(message_clone_with_NULL_message_source_fails)
{
    // arrange

    // act
    MESSAGE_HANDLE message = message_clone(NULL);

    // assert
    ASSERT_IS_NULL(message);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_004: [If allocating memory for the new cloned message fails, `message_clone` shall fail and return NULL.] */
TEST_FUNCTION(when_allocating_memory_fails_then_message_clone_fails)
{
    // arrange
    MESSAGE_HANDLE message;
    MESSAGE_HANDLE source_message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    message = message_clone(source_message);

    // assert
    ASSERT_IS_NULL(message);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(source_message);
}

/* message_destroy */

/* Tests_SRS_MESSAGE_01_013: [ `message_destroy` shall free all resources allocated by the message instance identified by the `message` argument. ]*/
TEST_FUNCTION(message_destroy_frees_the_allocated_memory)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_014: [ If `message` is NULL, `message_destroy` shall do nothing. ]*/
TEST_FUNCTION(message_destroy_with_NULL_does_nothing)
{
    // arrange

    // act
    message_destroy(NULL);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_015: [ The message header shall be freed by calling `header_destroy`. ]*/
TEST_FUNCTION(when_a_header_was_set_it_is_destroyed)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(header_clone(test_header));
    (void)message_set_header(message, test_header);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_destroy(cloned_header));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_016: [ The delivery annotations shall be freed by calling `annotations_destroy`. ]*/
TEST_FUNCTION(when_delivery_annotations_were_set_they_are_destroyed)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(message, test_delivery_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_delivery_annotations));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_017: [ The message annotations shall be freed by calling `annotations_destroy`. ]*/
TEST_FUNCTION(when_message_annotations_were_set_they_are_destroyed)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(message, test_message_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_message_annotations));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_018: [ The message properties shall be freed by calling `properties_destroy`. ]*/
TEST_FUNCTION(when_message_properties_were_set_they_are_destroyed)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(message, test_message_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_destroy(cloned_message_properties));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_019: [ The application properties shall be freed by calling `amqpvalue_destroy`. ]*/
TEST_FUNCTION(when_application_properties_were_set_they_are_destroyed)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(message, test_application_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_application_properties));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_020: [ The message footer shall be freed by calling `annotations_destroy`. ]*/
TEST_FUNCTION(when_message_footer_was_set_it_is_destroyed)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(message, test_footer);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_footer));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_021: [ If the message body is made of an AMQP value, the value shall be freed by calling `amqpvalue_destroy`. ]*/
TEST_FUNCTION(when_an_AMQP_value_is_set_as_body_message_destroy_frees_it)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_1))
        .SetReturn(cloned_amqp_value);
    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_amqp_value));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_136: [ If the message body is made of several AMQP data items, they shall all be freed. ]*/
TEST_FUNCTION(when_an_AMQP_data_is_set_as_body_message_destroy_frees_it)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    unsigned char data_bytes_1[] = { 0x42 };
    BINARY_DATA binary_data_1;
    binary_data_1.bytes = data_bytes_1;
    binary_data_1.length = sizeof(data_bytes_1);
    umock_c_reset_all_calls();
    (void)message_add_body_amqp_data(message, binary_data_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_136: [ If the message body is made of several AMQP data items, they shall all be freed. ]*/
TEST_FUNCTION(when_two_AMQP_data_items_are_set_as_body_message_destroy_frees_them)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    unsigned char data_bytes_1[] = { 0x42 };
    BINARY_DATA binary_data_1;
    unsigned char data_bytes_2[] = { 0x43 };
    BINARY_DATA binary_data_2;
    binary_data_1.bytes = data_bytes_1;
    binary_data_1.length = sizeof(data_bytes_1);
    binary_data_2.bytes = data_bytes_2;
    binary_data_2.length = sizeof(data_bytes_2);
    umock_c_reset_all_calls();
    (void)message_add_body_amqp_data(message, binary_data_1);
    (void)message_add_body_amqp_data(message, binary_data_2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_136: [ If the message body is made of several AMQP sequences, they shall all be freed. ]*/
/* Tests_SRS_MESSAGE_01_137: [ Each sequence shall be freed by calling `amqpvalue_destroy`. ]*/
TEST_FUNCTION(when_one_AMQP_sequence_is_set_as_body_message_destroy_frees_it)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_sequence_1));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_136: [ If the message body is made of several AMQP sequences, they shall all be freed. ]*/
/* Tests_SRS_MESSAGE_01_137: [ Each sequence shall be freed by calling `amqpvalue_destroy`. ]*/
TEST_FUNCTION(when_two_AMQP_sequences_are_set_as_body_message_destroy_frees_them)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_2))
        .SetReturn(cloned_sequence_2);
    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    (void)message_add_body_amqp_sequence(message, test_sequence_2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_sequence_1));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_sequence_2));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_015: [ The message header shall be freed by calling `header_destroy`. ]*/
/* Tests_SRS_MESSAGE_01_016: [ The delivery annotations shall be freed by calling `annotations_destroy`. ]*/
/* Tests_SRS_MESSAGE_01_017: [ The message annotations shall be freed by calling `annotations_destroy`. ]*/
/* Tests_SRS_MESSAGE_01_018: [ The message properties shall be freed by calling `properties_destroy`. ]*/
/* Tests_SRS_MESSAGE_01_019: [ The application properties shall be freed by calling `amqpvalue_destroy`. ]*/
/* Tests_SRS_MESSAGE_01_020: [ The message footer shall be freed by calling `annotations_destroy`. ]*/
/* Tests_SRS_MESSAGE_01_136: [ If the message body is made of several AMQP sequences, they shall all be freed. ]*/
/* Tests_SRS_MESSAGE_01_137: [ Each sequence shall be freed by calling `amqpvalue_destroy`. ]*/
TEST_FUNCTION(when_all_message_sections_are_set_and_seuqnces_are_used_then_they_are_all_destroyed)
{
    // arrange
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(message, test_footer);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(message, test_application_properties);
    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(message, test_message_properties);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(message, test_message_annotations);
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(message, test_delivery_annotations);
    STRICT_EXPECTED_CALL(header_clone(test_header));
    (void)message_set_header(message, test_header);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_2))
        .SetReturn(cloned_sequence_2);
    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    (void)message_add_body_amqp_sequence(message, test_sequence_2);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_destroy(cloned_header));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_delivery_annotations));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_message_annotations));
    STRICT_EXPECTED_CALL(properties_destroy(cloned_message_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_application_properties));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_footer));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_sequence_1));
    STRICT_EXPECTED_CALL(amqpvalue_destroy(cloned_sequence_2));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // act
    message_destroy(message);

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* message_set_header */

/* Tests_SRS_MESSAGE_01_022: [ `message_set_header` shall copy the contents of `message_header` as the header for the message instance identified by message. ]*/
/* Tests_SRS_MESSAGE_01_023: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_025: [ Cloning the header shall be done by calling `header_clone`. ]*/
TEST_FUNCTION(message_set_header_copies_the_header)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header));

    // act
    result = message_set_header(message, test_header);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_024: [ If `message` is NULL, `message_set_header` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_header_with_NULL_message_fails)
{
    // arrange
    int result;

    // act
    result = message_set_header(NULL, test_header);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_024: [ If `message` is NULL, `message_set_header` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_header_with_NULL_is_allowed)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_set_header(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_026: [ If `header_clone` fails, `message_set_header` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_header_clone_fails_message_set_header_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(NULL);

    // act
    result = message_set_header(message, test_header);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_138: [ If setting the header fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_header_clone_fails_previous_header_is_kept)
{
    // arrange
    HEADER_HANDLE result_header;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(message, test_header);
    STRICT_EXPECTED_CALL(header_clone(another_test_header))
        .SetReturn(NULL);
    (void)message_set_header(message, another_test_header);
    STRICT_EXPECTED_CALL(header_clone(cloned_header))
        .SetReturn(cloned_header);

    // act
    result = message_get_header(message, &result_header);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, cloned_header, result_header);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_139: [ If `message_header` is NULL, the previously stored header associated with `message` shall be freed. ]*/
TEST_FUNCTION(when_setting_a_NULL_header_previous_header_is_freed)
{
    // arrange
    HEADER_HANDLE result_header;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(message, test_header);
    STRICT_EXPECTED_CALL(header_destroy(cloned_header));
    (void)message_set_header(message, NULL);

    // act
    result = message_get_header(message, &result_header);

    // assert
    ASSERT_IS_NULL(result_header);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_139: [ If `message_header` is NULL, the previously stored header associated with `message` shall be freed. ]*/
TEST_FUNCTION(when_setting_a_NULL_header_twice_does_not_crash)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(message, test_header);
    (void)message_set_header(message, NULL);
    umock_c_reset_all_calls();

    // act
    result = message_set_header(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_header */

/* Tests_SRS_MESSAGE_01_027: [ `message_get_header` shall copy the contents of header for the message instance identified by `message` into the argument `message_header`. ]*/
/* Tests_SRS_MESSAGE_01_028: [ On success, `message_get_header` shall return 0.]*/
/* Tests_SRS_MESSAGE_01_030: [ Cloning the header shall be done by calling `header_clone`. ]*/
TEST_FUNCTION(message_get_header_gets_the_value)
{
    // arrange
    HEADER_HANDLE expected_header = (HEADER_HANDLE)0x5678;
    HEADER_HANDLE result_header;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(message, test_header);

    STRICT_EXPECTED_CALL(header_clone(cloned_header))
        .SetReturn(expected_header);

    // act
    result = message_get_header(message, &result_header);

    // assert
    ASSERT_ARE_EQUAL(void_ptr, expected_header, result_header);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_028: [ On success, `message_get_header` shall return 0.]*/
/* Tests_SRS_MESSAGE_01_143: [ If no header has been set, `message_get_header` shall set `message_header` to NULL. ]*/
TEST_FUNCTION(message_get_header_when_no_header_was_set_yields_NULL)
{
    // arrange
    HEADER_HANDLE result_header;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_header(message, &result_header);

    // assert
    ASSERT_IS_NULL(result_header);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_029: [ If `message` or `message_header` is NULL, `message_get_header` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_header_with_NULL_message_header_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(message, test_header);

    // act
    result = message_get_header(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_029: [ If `message` or `message_header` is NULL, `message_get_header` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_header_with_NULL_message_fails)
{
    // arrange
    int result;
    HEADER_HANDLE result_header;

    // act
    result = message_get_header(NULL, &result_header);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_031: [ If `header_clone` fails, `message_get_header` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_header_clone_fails_message_get_header_fails)
{
    // arrange
    HEADER_HANDLE result_header;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(header_clone(test_header))
        .SetReturn(cloned_header);
    (void)message_set_header(message, test_header);

    STRICT_EXPECTED_CALL(header_clone(cloned_header))
        .SetReturn(NULL);

    // act
    result = message_get_header(message, &result_header);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_set_delivery_annotations */

/* Tests_SRS_MESSAGE_01_032: [ `message_set_delivery_annotations` shall copy the contents of `annotations` as the delivery annotations for the message instance identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_033: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_035: [ Cloning the delivery annotations shall be done by calling `annotations_clone`. ]*/
TEST_FUNCTION(message_set_delivery_annotations_copies_the_annotations)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations));

    // act
    result = message_set_delivery_annotations(message, test_delivery_annotations);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_032: [ `message_set_delivery_annotations` shall copy the contents of `annotations` as the delivery annotations for the message instance identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_033: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_035: [ Cloning the delivery annotations shall be done by calling `annotations_clone`. ]*/
TEST_FUNCTION(message_set_delivery_annotations_with_NULL_annotations_succeeds)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_set_delivery_annotations(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_034: [ If `message` is NULL, `message_set_delivery_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_delivery_annotations_with_NULL_message_fails)
{
    // arrange
    int result;

    // act
    result = message_set_delivery_annotations(NULL, test_delivery_annotations);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_141: [ If `annotations` is NULL, the previously stored delivery annotations associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_delivery_annotations_with_NULL_delivery_annotations_frees_previous_delivery_annotations)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(message, test_delivery_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_destroy(cloned_delivery_annotations));

    // act
    result = message_set_delivery_annotations(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_141: [ If `annotations` is NULL, the previously stored delivery annotations associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_delivery_annotations_with_NULL_twice_does_not_crash)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    (void)message_set_delivery_annotations(message, test_delivery_annotations);
    (void)message_set_delivery_annotations(message, NULL);
    umock_c_reset_all_calls();

    // act
    result = message_set_delivery_annotations(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_036: [ If `annotations_clone` fails, `message_set_delivery_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_cloning_delivery_anootations_fails_message_set_delivery_annotations_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(NULL);

    // act
    result = message_set_delivery_annotations(message, test_delivery_annotations);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_140: [** If setting the delivery annotations fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_cloning_delivery_anootations_fails_the_previous_value_is_preserved)
{
    // arrange
    delivery_annotations stored_delivery_annotations;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    message_set_delivery_annotations(message, test_delivery_annotations);
    umock_c_reset_all_calls();

    // fail the set delivery annotations
    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(NULL);
    message_set_delivery_annotations(message, test_delivery_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations))
        .SetReturn(other_cloned_delivery_annotations);

    // act
    result = message_get_delivery_annotations(message, &stored_delivery_annotations);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_delivery_annotations, (void*)stored_delivery_annotations);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_delivery_annotations */

/* Tests_SRS_MESSAGE_01_037: [ `message_get_delivery_annotations` shall copy the contents of delivery annotations for the message instance identified by `message` into the argument `annotations`. ]*/
/* Tests_SRS_MESSAGE_01_038: [ On success, `message_get_delivery_annotations` shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_040: [ Cloning the delivery annotations shall be done by calling `annotations_clone`. ]*/
TEST_FUNCTION(message_get_delivery_annotations_clones_thestored_delivery_annotations)
{
    // arrange
    delivery_annotations stored_delivery_annotations;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    message_set_delivery_annotations(message, test_delivery_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations))
        .SetReturn(other_cloned_delivery_annotations);

    // act
    result = message_get_delivery_annotations(message, &stored_delivery_annotations);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_delivery_annotations, (void*)stored_delivery_annotations);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_039: [ If `message` or `annotations` is NULL, `message_get_delivery_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_delivery_annotations_with_NULL_delivery_annotations_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    message_set_delivery_annotations(message, test_delivery_annotations);
    umock_c_reset_all_calls();

    // act
    result = message_get_delivery_annotations(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_039: [ If `message` or `annotations` is NULL, `message_get_delivery_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_delivery_annotations_with_NULL_message)
{
    // arrange
    delivery_annotations stored_delivery_annotations;
    int result;

    // act
    result = message_get_delivery_annotations(NULL, &stored_delivery_annotations);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_041: [ If `annotations_clone` fails, `message_get_delivery_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_annotations_clone_fails_message_get_delivery_annotations_fails)
{
    // arrange
    delivery_annotations stored_delivery_annotations;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_delivery_annotations))
        .SetReturn(cloned_delivery_annotations);
    message_set_delivery_annotations(message, test_delivery_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_delivery_annotations))
        .SetReturn(NULL);

    // act
    result = message_get_delivery_annotations(message, &stored_delivery_annotations);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_142: [ If no delivery annotations have been set, `message_get_delivery_annotations` shall set `annotations` to NULL. ]*/
TEST_FUNCTION(message_get_delivery_annotations_when_not_set_yields_NULL)
{
    // arrange
    delivery_annotations stored_delivery_annotations = (delivery_annotations)0x0001;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_delivery_annotations(message, &stored_delivery_annotations);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, (void*)stored_delivery_annotations);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_set_message_annotations */

/* Tests_SRS_MESSAGE_01_042: [ `message_set_message_annotations` shall copy the contents of `message_annotations` as the message annotations for the message instance identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_043: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_045: [ Cloning the message annotations shall be done by calling `annotations_clone`. ]*/
TEST_FUNCTION(message_set_message_annotations_copies_the_message_annotations)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations));

    // act
    result = message_set_message_annotations(message, test_message_annotations);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_044: [ If `message` is NULL, `message_set_message_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_message_annotations_with_NULL_message_fails)
{
    // arrange
    int result;

    // act
    result = message_set_message_annotations(NULL, test_message_annotations);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_046: [ If `annotations_clone` fails, `message_set_message_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_annotations_clone_fails_message_set_message_annotations_also_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(NULL);

    // act
    result = message_set_message_annotations(message, test_message_annotations);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_144: [ If setting the message annotations fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_annotations_clone_fails_message_set_message_annotations_preserves_the_previous_value)
{
    // arrange
    message_annotations stored_message_annotations;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(message, test_message_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(NULL);
    (void)message_set_message_annotations(message, test_message_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations))
        .SetReturn(other_cloned_message_annotations);

    // act
    result = message_get_message_annotations(message, &stored_message_annotations);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_message_annotations, (void*)stored_message_annotations);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_145: [ If `annotations` is NULL, the previously stored message annotations associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_message_annotations_with_NULL_frees_previous_value)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(message, test_message_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_destroy(cloned_message_annotations));

    // act
    result = message_set_message_annotations(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_145: [ If `annotations` is NULL, the previously stored message annotations associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_message_annotations_with_NULL_twice_does_not_crash)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(message, test_message_annotations);
    message_set_message_annotations(message, NULL);
    umock_c_reset_all_calls();

    // act
    result = message_set_message_annotations(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_message_annotations */

/* Tests_SRS_MESSAGE_01_047: [ `message_get_message_annotations` shall copy the contents of message annotations for the message instance identified by `message` into the argument `annotations`. ]*/
/* Tests_SRS_MESSAGE_01_048: [ On success, `message_get_message_annotations` shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_050: [ Cloning the message annotations shall be done by calling `annotations_clone`. ]*/
TEST_FUNCTION(message_get_message_annotations_clones_the_stored_message_annotations)
{
    // arrange
    message_annotations stored_message_annotations;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(message, test_message_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations))
        .SetReturn(other_cloned_message_annotations);

    // act
    result = message_get_message_annotations(message, &stored_message_annotations);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_message_annotations, (void*)stored_message_annotations);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_049: [ If `message` or `annotations` is NULL, `message_get_message_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_message_annotations_with_NULL_message_annotations_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(message, test_message_annotations);
    umock_c_reset_all_calls();

    // act
    result = message_get_message_annotations(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_049: [ If `message` or `annotations` is NULL, `message_get_message_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_message_annotations_with_NULL_message_fails)
{
    // arrange
    message_annotations stored_message_annotations;
    int result;

    // act
    result = message_get_message_annotations(NULL, &stored_message_annotations);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_051: [ If `annotations_clone` fails, `message_get_message_annotations` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_annotations_clone_fails_message_get_message_annotations_fails)
{
    // arrange
    message_annotations stored_message_annotations;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_message_annotations))
        .SetReturn(cloned_message_annotations);
    (void)message_set_message_annotations(message, test_message_annotations);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_message_annotations))
        .SetReturn(NULL);

    // act
    result = message_get_message_annotations(message, &stored_message_annotations);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_146: [ If no message annotations have been set, `message_get_message_annotations` shall set `annotations` to NULL. ]*/
/* Tests_SRS_MESSAGE_01_048: [ On success, `message_get_message_annotations` shall return 0. ]*/
TEST_FUNCTION(when_no_message_annotations_have_been_set_message_get_message_annotations_yields_NULL)
{
    // arrange
    message_annotations stored_message_annotations;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_message_annotations(message, &stored_message_annotations);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, (void*)stored_message_annotations);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_set_properties */

/* Tests_SRS_MESSAGE_01_052: [ `message_set_properties` shall copy the contents of `properties` as the message properties for the message instance identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_053: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_055: [ Cloning the message properties shall be done by calling `properties_clone`. ]*/
TEST_FUNCTION(message_set_properties_copies_the_message_properties)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties));

    // act
    result = message_set_properties(message, test_message_properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_054: [ If `message` is NULL, `message_set_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_properties_with_NULL_message_fails)
{
    // arrange
    int result;

    // act
    result = message_set_properties(NULL, test_message_properties);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_056: [ If `properties_clone` fails, `message_set_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_properties_clone_fails_message_set_properties_also_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(NULL);

    // act
    result = message_set_properties(message, test_message_properties);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_063: [ If setting the message properties fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_properties_clone_fails_message_set_properties_preserves_the_previous_value)
{
    // arrange
    PROPERTIES_HANDLE stored_message_properties;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(message, test_message_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(NULL);
    (void)message_set_properties(message, test_message_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties))
        .SetReturn(other_cloned_message_properties);

    // act
    result = message_get_properties(message, &stored_message_properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_message_properties, stored_message_properties);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_147: [ If `properties` is NULL, the previously stored message properties associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_properties_with_NULL_message_properties_frees_the_previously_stored_value)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(message, test_message_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_destroy(cloned_message_properties));

    // act
    result = message_set_properties(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_147: [ If `properties` is NULL, the previously stored message properties associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_properties_with_NULL_twice_message_properties_does_not_crash)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(message, test_message_properties);
    (void)message_set_properties(message, NULL);
    umock_c_reset_all_calls();

    // act
    result = message_set_properties(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_properties */

/* Tests_SRS_MESSAGE_01_057: [ `message_get_properties` shall copy the contents of message properties for the message instance identified by `message` into the argument `properties`. ]*/
/* Tests_SRS_MESSAGE_01_058: [ On success, `message_get_properties` shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_060: [ Cloning the message properties shall be done by calling `properties_clone`. ]*/
TEST_FUNCTION(message_get_properties_clones_the_stored_properties)
{
    // arrange
    PROPERTIES_HANDLE stored_message_properties;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(message, test_message_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties))
        .SetReturn(other_cloned_message_properties);

    // act
    result = message_get_properties(message, &stored_message_properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_message_properties, (void*)stored_message_properties);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_059: [ If `message` or `properties` is NULL, `message_get_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_properties_with_NULL_properties_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(message, test_message_properties);
    umock_c_reset_all_calls();

    // act
    result = message_get_properties(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_059: [ If `message` or `properties` is NULL, `message_get_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_properties_with_NULL_message_fails)
{
    // arrange
    PROPERTIES_HANDLE stored_message_properties;
    int result;

    // act
    result = message_get_properties(NULL, &stored_message_properties);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_061: [ If `properties_clone` fails, `message_get_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_properties_clone_fails_message_get_properties_fails)
{
    // arrange
    PROPERTIES_HANDLE stored_message_properties;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(test_message_properties))
        .SetReturn(cloned_message_properties);
    (void)message_set_properties(message, test_message_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(properties_clone(cloned_message_properties))
        .SetReturn(NULL);

    // act
    result = message_get_properties(message, &stored_message_properties);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_148: [ If no message properties have been set, `message_get_properties` shall set `properties` to NULL. ]*/
/* Tests_SRS_MESSAGE_01_058: [ On success, `message_get_properties` shall return 0. ]*/
TEST_FUNCTION(message_get_properties_when_no_message_properties_have_been_set_yields_NULL)
{
    // arrange
    PROPERTIES_HANDLE stored_message_properties;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_properties(message, &stored_message_properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, (void*)stored_message_properties);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_set_application_properties */

/* Tests_SRS_MESSAGE_01_064: [ `message_set_application_properties` shall copy the contents of `application_properties` as the application properties for the message instance identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_065: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_067: [ Cloning the message properties shall be done by calling `application_properties_clone`. ]*/
TEST_FUNCTION(message_set_application_properties_copies_the_application_properties)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(test_application_properties));

    // act
    result = message_set_application_properties(message, test_application_properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_066: [ If `message` is NULL, `message_set_application_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_application_properties_with_NULL_message_fails)
{
    // arrange
    int result;

    // act
    result = message_set_application_properties(NULL, test_application_properties);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_068: [ If `application_properties_clone` fails, `message_set_application_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_cloning_the_application_properties_fails_message_set_application_properties_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(test_application_properties))
        .SetReturn(NULL);

    // act
    result = message_set_application_properties(message, test_application_properties);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_069: [ If setting the application properties fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_cloning_the_application_properties_fails_the_previous_value_is_preserved)
{
    // arrange
    AMQP_VALUE stored_application_properties;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(test_application_properties))
        .SetReturn(cloned_application_properties);

    (void)message_set_application_properties(message, test_application_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(cloned_application_properties))
        .SetReturn(NULL);
    (void)message_set_application_properties(message, cloned_application_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(cloned_application_properties))
        .SetReturn(other_cloned_application_properties);

    // act
    result = message_get_application_properties(message, &stored_application_properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_application_properties, (void*)stored_application_properties);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_149: [ If `application_properties` is NULL, the previously stored application properties associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_application_properties_with_NULL_application_properties_frees_the_previous_value)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(test_application_properties))
        .SetReturn(cloned_application_properties);

    (void)message_set_application_properties(message, test_application_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_destroy(cloned_application_properties));

    // act
    result = message_set_application_properties(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_149: [ If `application_properties` is NULL, the previously stored application properties associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_application_properties_with_NULL_twice_does_not_crash)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(test_application_properties))
        .SetReturn(cloned_application_properties);

    (void)message_set_application_properties(message, test_application_properties);
    (void)message_set_application_properties(message, NULL);
    umock_c_reset_all_calls();

    // act
    result = message_set_application_properties(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_application_properties */

/* Tests_SRS_MESSAGE_01_070: [ `message_get_application_properties` shall copy the contents of application message properties for the message instance identified by `message` into the argument `application_properties`. ]*/
/* Tests_SRS_MESSAGE_01_071: [ On success, `message_get_application_properties` shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_073: [ Cloning the application properties shall be done by calling `application_properties_clone`. ]*/
TEST_FUNCTION(message_get_application_properties_clones_the_stored_application_properties)
{
    // arrange
    AMQP_VALUE stored_application_properties;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(message, test_application_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(cloned_application_properties))
        .SetReturn(other_cloned_application_properties);

    // act
    result = message_get_application_properties(message, &stored_application_properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_application_properties, (void*)stored_application_properties);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_072: [ If `message` or `application_properties` is NULL, `message_get_application_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_application_properties_with_NULL_application_properties_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(message, test_application_properties);
    umock_c_reset_all_calls();

    // act
    result = message_get_application_properties(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_072: [ If `message` or `application_properties` is NULL, `message_get_application_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_application_properties_with_NULL_message_fails)
{
    // arrange
    AMQP_VALUE stored_application_properties;
    int result;

    // act
    result = message_get_application_properties(NULL, &stored_application_properties);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_074: [ If `application_properties_clone` fails, `message_get_application_properties` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_cloning_the_application_properties_fails_message_get_application_properties_fails)
{
    // arrange
    AMQP_VALUE stored_application_properties;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(test_application_properties))
        .SetReturn(cloned_application_properties);
    (void)message_set_application_properties(message, test_application_properties);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(application_properties_clone(cloned_application_properties))
        .SetReturn(NULL);

    // act
    result = message_get_application_properties(message, &stored_application_properties);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_150: [ If no application properties have been set, `message_get_application_properties` shall set `application_properties` to NULL. ]*/
/* Tests_SRS_MESSAGE_01_071: [ On success, `message_get_application_properties` shall return 0. ]*/
TEST_FUNCTION(message_get_application_properties_when_no_application_properties_have_been_set_yields_NULL)
{
    // arrange
    AMQP_VALUE stored_application_properties;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_application_properties(message, &stored_application_properties);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, (void*)stored_application_properties);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_set_footer */

/* Tests_SRS_MESSAGE_01_075: [ `message_set_footer` shall copy the contents of `footer` as the footer contents for the message instance identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_076: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_078: [ Cloning the footer shall be done by calling `annotations_clone`. ]*/
TEST_FUNCTION(message_set_footer_copies_the_footer)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);

    // act
    result = message_set_footer(message, test_footer);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_077: [ If `message` is NULL, `message_set_footer` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_footer_with_NULL_message_fails)
{
    // arrange
    int result;

    // act
    result = message_set_footer(NULL, test_footer);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_079: [ If `annotations_clone` fails, `message_set_footer` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_annotations_clone_fails_message_set_footer_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(NULL);

    // act
    result = message_set_footer(message, test_footer);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_080: [ If setting the footer fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_annotations_clone_fails_the_footer_on_the_message_is_preserved)
{
    // arrange
    annotations stored_footer;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(message, test_footer);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(NULL);
    (void)message_set_footer(message, test_footer);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer))
        .SetReturn(other_cloned_footer);

    // act
    result = message_get_footer(message, &stored_footer);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_footer, stored_footer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_151: [ If `footer` is NULL, the previously stored footer associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_footer_with_NULL_footer_frees_the_previous_value)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(message, test_footer);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_destroy(cloned_footer));

    // act
    result = message_set_footer(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_151: [ If `footer` is NULL, the previously stored footer associated with `message` shall be freed. ]*/
TEST_FUNCTION(message_set_footer_with_NULL_footer_twice_does_not_crash)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(message, test_footer);
    (void)message_set_footer(message, NULL);
    umock_c_reset_all_calls();

    // act
    result = message_set_footer(message, NULL);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_footer */

/* Tests_SRS_MESSAGE_01_081: [ `message_get_footer` shall copy the contents of footer for the message instance identified by `message` into the argument `footer`. ]*/
/* Tests_SRS_MESSAGE_01_082: [ On success, `message_get_footer` shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_084: [ Cloning the footer shall be done by calling `annotations_clone`. ]*/
TEST_FUNCTION(message_get_footer_clones_the_footer)
{
    // arrange
    annotations stored_footer;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(message, test_footer);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer))
        .SetReturn(other_cloned_footer);

    // act
    result = message_get_footer(message, &stored_footer);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, (void*)other_cloned_footer, (void*)stored_footer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_083: [ If `message` or `footer` is NULL, `message_get_footer` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_footer_with_NULL_footer_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(message, test_footer);
    umock_c_reset_all_calls();

    // act
    result = message_get_footer(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_083: [ If `message` or `footer` is NULL, `message_get_footer` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_footer_with_NULL_message_fails)
{
    // arrange
    annotations stored_footer;
    int result;

    // act
    result = message_get_footer(NULL, &stored_footer);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_085: [ If `annotations_clone` fails, `message_get_footer` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_cloning_the_footer_fails_message_get_footer_fails)
{
    // arrange
    annotations stored_footer;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(test_footer))
        .SetReturn(cloned_footer);
    (void)message_set_footer(message, test_footer);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(annotations_clone(cloned_footer))
        .SetReturn(NULL);

    // act
    result = message_get_footer(message, &stored_footer);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_152: [ If no footer has been set, `message_get_footer` shall set `footer` to NULL. ]*/
TEST_FUNCTION(message_get_footer_when_no_footer_has_been_set_yields_NULL)
{
    // arrange
    annotations stored_footer;
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_footer(message, &stored_footer);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, NULL, (void*)stored_footer);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_add_body_amqp_data */

/* Tests_SRS_MESSAGE_01_086: [ `message_add_body_amqp_data` shall add the contents of `amqp_data` to the list of AMQP data values for the body of the message identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_087: [ On success it shall return 0. ]*/
TEST_FUNCTION(message_add_body_amqp_data_adds_one_amqp_data_item)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    // act
    result = message_add_body_amqp_data(message, amqp_data);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_088: [ If `message` is NULL, `message_add_body_amqp_data` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_data_with_NULL_message_fails)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    // act
    result = message_add_body_amqp_data(NULL, amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_089: [ If the `bytes` member of `amqp_data` is NULL and the `size` member is non-zero, `message_add_body_amqp_data` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_data_with_NULL_buffer_and_non_zero_size_fails)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = NULL;
    amqp_data.length = 1;

    // act
    result = message_add_body_amqp_data(message, amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_089: [ If the `bytes` member of `amqp_data` is NULL and the `size` member is non-zero, `message_add_body_amqp_data` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_data_with_NULL_buffer_and_zero_size_succeeds)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = NULL;
    amqp_data.length = 0;

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    result = message_add_body_amqp_data(message, amqp_data);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_089: [ If the `bytes` member of `amqp_data` is NULL and the `size` member is non-zero, `message_add_body_amqp_data` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_data_with_non_NULL_buffer_and_zero_size_succeeds)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = 0;

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    // act
    result = message_add_body_amqp_data(message, amqp_data);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_153: [ If allocating memory to store the added AMQP data fails, `message_add_body_amqp_data` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_reallocating_memory_for_the_data_array_fails_message_add_body_amqp_data_fails)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = 0;

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = message_add_body_amqp_data(message, amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_153: [ If allocating memory to store the added AMQP data fails, `message_add_body_amqp_data` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_AMQP_data_item_fails_message_add_body_amqp_data_fails)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = message_add_body_amqp_data(message, amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_090: [ If adding the body AMQP data fails, the previous body content shall be preserved. ]*/
TEST_FUNCTION(when_reallocating_memory_for_the_data_array_fails_message_add_body_amqp_data_preserves_the_previous_empty_body_value)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    size_t body_amqp_data_count;
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);
    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_count(message, &body_amqp_data_count);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    (void)message_get_body_type(message, &body_type);
    ASSERT_ARE_EQUAL(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_NONE, body_type);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_090: [ If adding the body AMQP data fails, the previous body content shall be preserved. ]*/
TEST_FUNCTION(when_reallocating_memory_for_the_data_array_fails_message_add_body_amqp_data_preserves_the_previous_body_value_with_1_amqp_data)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    BINARY_DATA read_amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    size_t body_amqp_data_count;
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    char expected_bytes[128];
    char actual_bytes[128];
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    message_add_body_amqp_data(message, amqp_data);

    // act
    result = message_get_body_amqp_data_count(message, &body_amqp_data_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, body_amqp_data_count);
    (void)message_get_body_type(message, &body_type);
    ASSERT_ARE_EQUAL(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_DATA, body_type);
    (void)message_get_body_amqp_data_in_place(message, 0, &read_amqp_data);
    stringify_bytes(read_amqp_data.bytes, read_amqp_data.length, actual_bytes);
    stringify_bytes(amqp_data_bytes, sizeof(amqp_data_bytes), expected_bytes);
    ASSERT_ARE_EQUAL(char_ptr, expected_bytes, actual_bytes);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_091: [ If the body was already set to an AMQP value or a list of AMQP sequences, `message_add_body_amqp_data` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_data_when_body_is_a_list_of_sequences_fails)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    size_t body_amqp_sequence_count;
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_add_body_amqp_data(message, amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    message_get_body_amqp_sequence_count(message, &body_amqp_sequence_count);
    ASSERT_ARE_EQUAL(size_t, 1, body_amqp_sequence_count);
    (void)message_get_body_type(message, &body_type);
    ASSERT_ARE_EQUAL(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_SEQUENCE, body_type);

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_091: [ If the body was already set to an AMQP value or a list of AMQP sequences, `message_add_body_amqp_data` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_reallocating_memory_for_the_data_array_fails_message_add_body_amqp_data_preserves_the_previous_body_value_with_amqp_value)
{
    // arrange
    int result;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_1));

    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    // act
    result = message_add_body_amqp_data(message, amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    message_get_body_type(message, &body_type);
    ASSERT_ARE_EQUAL(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_VALUE, body_type);

    // cleanup
    message_destroy(message);
}

/* message_get_body_amqp_data_in_place */

/* Tests_SRS_MESSAGE_01_092: [ `message_get_body_amqp_data_in_place` shall place the contents of the `index`th AMQP data for the message instance identified by `message` into the argument `amqp_data`, without copying the binary payload memory. ]*/
/* Tests_SRS_MESSAGE_01_093: [ On success, `message_get_body_amqp_data_in_place` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_data_yields_the_amqp_data)
{
    // arrange
    int result;
    unsigned char amqp_data_bytes[] = { 0x42 };
    BINARY_DATA amqp_data;
    BINARY_DATA read_amqp_data;
    MESSAGE_HANDLE message = message_create();
    char expected_bytes[128];
    char actual_bytes[128];
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_in_place(message, 0, &read_amqp_data);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    stringify_bytes(read_amqp_data.bytes, read_amqp_data.length, actual_bytes);
    stringify_bytes(amqp_data_bytes, sizeof(amqp_data_bytes), expected_bytes);
    ASSERT_ARE_EQUAL(char_ptr, expected_bytes, actual_bytes);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_092: [ `message_get_body_amqp_data_in_place` shall place the contents of the `index`th AMQP data for the message instance identified by `message` into the argument `amqp_data`, without copying the binary payload memory. ]*/
/* Tests_SRS_MESSAGE_01_093: [ On success, `message_get_body_amqp_data_in_place` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_data_can_get_both_values_when_2_values_are_in_the_body)
{
    // arrange
    int result1;
    int result2;
    unsigned char amqp_data_bytes_1[] = { 0x42 };
    unsigned char amqp_data_bytes_2[] = { 0x43, 0x44 };
    BINARY_DATA amqp_data_1;
    BINARY_DATA amqp_data_2;
    BINARY_DATA read_amqp_data_1;
    BINARY_DATA read_amqp_data_2;
    MESSAGE_HANDLE message = message_create();
    char expected_bytes[128];
    char actual_bytes[128];
    umock_c_reset_all_calls();

    amqp_data_1.bytes = amqp_data_bytes_1;
    amqp_data_1.length = sizeof(amqp_data_bytes_1);
    amqp_data_2.bytes = amqp_data_bytes_2;
    amqp_data_2.length = sizeof(amqp_data_bytes_2);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data_2);
    umock_c_reset_all_calls();

    // act
    result1 = message_get_body_amqp_data_in_place(message, 0, &read_amqp_data_1);
    result2 = message_get_body_amqp_data_in_place(message, 1, &read_amqp_data_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result1);
    ASSERT_ARE_EQUAL(int, 0, result2);
    stringify_bytes(read_amqp_data_1.bytes, read_amqp_data_1.length, actual_bytes);
    stringify_bytes(amqp_data_bytes_1, sizeof(amqp_data_bytes_1), expected_bytes);
    ASSERT_ARE_EQUAL(char_ptr, expected_bytes, actual_bytes);
    stringify_bytes(read_amqp_data_2.bytes, read_amqp_data_2.length, actual_bytes);
    stringify_bytes(amqp_data_bytes_2, sizeof(amqp_data_bytes_2), expected_bytes);
    ASSERT_ARE_EQUAL(char_ptr, expected_bytes, actual_bytes);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_095: [ If `index` indicates an AMQP data entry that is out of bounds, `message_get_body_amqp_data_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_data_in_place_when_no_amqp_data_was_added_fails)
{
    // arrange
    int result;
    BINARY_DATA read_amqp_data;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_in_place(message, 0, &read_amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_095: [ If `index` indicates an AMQP data entry that is out of bounds, `message_get_body_amqp_data_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_data_in_place_when_index_is_out_of_bounds_with_one_element_added_fails)
{
    // arrange
    int result;
    unsigned char amqp_data_bytes[] = { 0x42 };
    BINARY_DATA amqp_data;
    BINARY_DATA read_amqp_data;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_in_place(message, 1, &read_amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_094: [ If `message` or `amqp_data` is NULL, `message_get_body_amqp_data_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_data_with_NULL_amqp_data_fails)
{
    // arrange
    int result;
    unsigned char amqp_data_bytes[] = { 0x42 };
    BINARY_DATA amqp_data;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_in_place(message, 0, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_094: [ If `message` or `amqp_data` is NULL, `message_get_body_amqp_data_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_data_with_NULL_message_data_fails)
{
    // arrange
    int result;
    BINARY_DATA read_amqp_data;

    // act
    result = message_get_body_amqp_data_in_place(NULL, 0, &read_amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_096: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_DATA`, `message_get_body_amqp_data_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_data_when_body_is_amqp_value)
{
    // arrange
    int result;
    BINARY_DATA read_amqp_data;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_in_place(message, 1, &read_amqp_data);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_body_amqp_data_count */

/* Tests_SRS_MESSAGE_01_097: [ `message_get_body_amqp_data_count` shall fill in `count` the number of AMQP data chunks that are stored by the message identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_098: [ On success, `message_get_body_amqp_data_count` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_data_count_with_one_amqp_data_returns_1)
{

    // arrange
    int result;
    unsigned char amqp_data_bytes[] = { 0x42 };
    BINARY_DATA amqp_data;
    MESSAGE_HANDLE message = message_create();
    size_t amqp_data_count;
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_count(message, &amqp_data_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, amqp_data_count);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_097: [ `message_get_body_amqp_data_count` shall fill in `count` the number of AMQP data chunks that are stored by the message identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_098: [ On success, `message_get_body_amqp_data_count` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_data_count_with_two_amqp_data_returns_2)
{

    // arrange
    int result;
    unsigned char amqp_data_bytes_1[] = { 0x42 };
    unsigned char amqp_data_bytes_2[] = { 0x42 };
    BINARY_DATA amqp_data_1;
    BINARY_DATA amqp_data_2;
    MESSAGE_HANDLE message = message_create();
    size_t amqp_data_count;
    umock_c_reset_all_calls();

    amqp_data_1.bytes = amqp_data_bytes_1;
    amqp_data_1.length = sizeof(amqp_data_bytes_1);
    amqp_data_2.bytes = amqp_data_bytes_2;
    amqp_data_2.length = sizeof(amqp_data_bytes_2);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data_1);
    (void)message_add_body_amqp_data(message, amqp_data_2);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_count(message, &amqp_data_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 2, amqp_data_count);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_100: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_DATA`, `message_get_body_amqp_data_count` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_data_count_when_no_amqp_data_values_are_set_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    size_t amqp_data_count;
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_count(message, &amqp_data_count);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_099: [ If `message` or `count` is NULL, `message_get_body_amqp_data_count` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_data_count_with_NULL_count_fails)
{

    // arrange
    int result;
    unsigned char amqp_data_bytes[] = { 0x42 };
    BINARY_DATA amqp_data;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_data_count(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_099: [ If `message` or `count` is NULL, `message_get_body_amqp_data_count` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_data_count_with_NULL_message_fails)
{

    // arrange
    int result;
    size_t amqp_data_count;

    // act
    result = message_get_body_amqp_data_count(NULL, &amqp_data_count);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* message_set_body_amqp_value */

/* Tests_SRS_MESSAGE_01_101: [ `message_set_body_amqp_value` shall set the contents of body as being the AMQP value indicate by `body_amqp_value`. ]*/
/* Tests_SRS_MESSAGE_01_102: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_154: [ Cloning the amqp value shall be done by calling `amqpvalue_clone`. ]*/
TEST_FUNCTION(message_set_body_amqp_value_sets_the_body_to_the_amqp_value)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_1));

    // act
    result = message_set_body_amqp_value(message, test_amqp_value_1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_103: [ If `message` or `body_amqp_value` is NULL, `message_set_body_amqp_value` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_body_amqp_value_with_NULL_body_amqp_value_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_set_body_amqp_value(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Codes_SRS_MESSAGE_01_103: [ If `message` or `body_amqp_value` is NULL, `message_set_body_amqp_value` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_body_amqp_value_with_NULL_message_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_set_body_amqp_value(NULL, test_amqp_value_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_155: [ If `amqpvalue_clone` fails, `message_set_body_amqp_value` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_amqpvalue_clone_fails_message_set_body_amqp_value_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_1))
        .SetReturn(NULL);

    // act
    result = message_set_body_amqp_value(message, test_amqp_value_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_104: [ If setting the body AMQP value fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_cloning_the_amqp_value_fails_the_previous_value_is_preserved)
{

    // arrange
    int result;
    AMQP_VALUE read_amqp_value;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_1))
        .SetReturn(cloned_amqp_value);
    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_2))
        .SetReturn(NULL);
    (void)message_set_body_amqp_value(message, test_amqp_value_2);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_value_in_place(message, &read_amqp_value);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, cloned_amqp_value, read_amqp_value);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_105: [ If the body was already set to an AMQP data list or a list of AMQP sequences, `message_set_body_amqp_value` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_body_amqp_value_when_the_body_was_set_to_amqp_data_list_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_set_body_amqp_value(message, test_amqp_value_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_105: [ If the body was already set to an AMQP data list or a list of AMQP sequences, `message_set_body_amqp_value` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_body_amqp_value_when_the_body_was_set_to_amqp_sequence_list_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_set_body_amqp_value(message, test_amqp_value_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_body_amqp_value_in_place */

/* Tests_SRS_MESSAGE_01_106: [ `message_get_body_amqp_value_in_place` shall get the body AMQP value for the message instance identified by `message` in place (not cloning) into the `body_amqp_value` argument. ]*/
/* Tests_SRS_MESSAGE_01_107: [ On success, `message_get_body_amqp_value_in_place` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_value_in_place_gets_the_amqp_value_that_was_set)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    AMQP_VALUE read_amqp_value;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_1))
        .SetReturn(cloned_amqp_value);

    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_value_in_place(message, &read_amqp_value);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, cloned_amqp_value, read_amqp_value);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_108: [ If `message` or `body_amqp_value` is NULL, `message_get_body_amqp_value_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_value_in_place_with_NULL_amqp_value_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(amqpvalue_clone(test_amqp_value_2))
        .SetReturn(cloned_amqp_value);

    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_value_in_place(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_108: [ If `message` or `body_amqp_value` is NULL, `message_get_body_amqp_value_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_value_in_place_with_NULL_message_fails)
{

    // arrange
    int result;
    AMQP_VALUE read_amqp_value;

    // act
    result = message_get_body_amqp_value_in_place(NULL, &read_amqp_value);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_109: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_VALUE`, `message_get_body_amqp_value_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_value_in_place_when_the_body_type_is_amqp_data_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    AMQP_VALUE read_amqp_value;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_value_in_place(message, &read_amqp_value);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_109: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_VALUE`, `message_get_body_amqp_value_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_value_in_place_when_the_body_type_is_amqp_sequence_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    AMQP_VALUE read_amqp_value;
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_value_in_place(message, &read_amqp_value);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_109: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_VALUE`, `message_get_body_amqp_value_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_value_in_place_when_no_body_was_set_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    AMQP_VALUE read_amqp_value;
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_value_in_place(message, &read_amqp_value);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_add_body_amqp_sequence */

/* Tests_SRS_MESSAGE_01_110: [ `message_add_body_amqp_sequence` shall add the contents of `sequence` to the list of AMQP sequences for the body of the message identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_111: [ On success it shall return 0. ]*/
/* Tests_SRS_MESSAGE_01_156: [ The AMQP sequence shall be cloned by calling `amqpvalue_clone`. ]*/
TEST_FUNCTION(message_add_body_amqp_sequence_adds_the_sequence_to_the_body)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1));

    // act
    result = message_add_body_amqp_sequence(message, test_sequence_1);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_158: [ If allocating memory in order to store the sequence fails, `message_add_body_amqp_sequence` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_sequence_list_fails_message_add_body_amqp_sequence_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = message_add_body_amqp_sequence(message, test_sequence_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_157: [ If `amqpvalue_clone` fails, `message_add_body_amqp_sequence` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(when_amqpvalue_clone_fails_message_add_body_amqp_sequence_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(NULL);

    // act
    result = message_add_body_amqp_sequence(message, test_sequence_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_112: [ If `message` or `sequence` is NULL, `message_add_body_amqp_sequence` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_sequence_with_NULL_sequence_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_add_body_amqp_sequence(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_112: [ If `message` or `sequence` is NULL, `message_add_body_amqp_sequence` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_sequence_with_NULL_message_fails)
{

    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_add_body_amqp_sequence(NULL, test_sequence_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_114: [ If adding the AMQP sequence fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_allocating_memory_for_the_sequence_fails_message_add_body_amqp_sequence_preserves_the_previous_sequence_body)
{
    // arrange
    int result;
    size_t sequence_count;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG))
        .SetReturn(NULL);

    // act
    result = message_add_body_amqp_sequence(message, test_sequence_2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    (void)message_get_body_amqp_sequence_count(message, &sequence_count);
    ASSERT_ARE_EQUAL(size_t, 1, sequence_count);

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_114: [ If adding the AMQP sequence fails, the previous value shall be preserved. ]*/
TEST_FUNCTION(when_cloning_the_sequence_fails_message_add_body_amqp_sequence_preserves_the_previous_sequence_body)
{
    // arrange
    int result;
    size_t sequence_count;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_2))
        .SetReturn(NULL);

    // act
    result = message_add_body_amqp_sequence(message, test_sequence_2);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    (void)message_get_body_amqp_sequence_count(message, &sequence_count);
    ASSERT_ARE_EQUAL(size_t, 1, sequence_count);

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_115: [ If the body was already set to an AMQP data list or an AMQP value, `message_add_body_amqp_sequence` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_sequence_when_the_body_was_set_to_amqp_data_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_add_body_amqp_sequence(message, test_sequence_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_115: [ If the body was already set to an AMQP data list or an AMQP value, `message_add_body_amqp_sequence` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_add_body_amqp_sequence_when_the_body_was_set_to_amqp_value_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    // act
    result = message_add_body_amqp_sequence(message, test_sequence_1);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_body_amqp_sequence_in_place */

/* Tests_SRS_MESSAGE_01_116: [ `message_get_body_amqp_sequence_in_place` shall return in `sequence` the content of the `index`th AMQP seuquence entry for the message instance identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_117: [ On success, `message_get_body_amqp_sequence_in_place` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_in_place_gets_the_first_item)
{
    // arrange
    int result;
    AMQP_VALUE read_sequence;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_in_place(message, 0, &read_sequence);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(void_ptr, cloned_sequence_1, read_sequence);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_116: [ `message_get_body_amqp_sequence_in_place` shall return in `sequence` the content of the `index`th AMQP seuquence entry for the message instance identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_117: [ On success, `message_get_body_amqp_sequence_in_place` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_in_place_gets_2_items)
{
    // arrange
    int result1;
    int result2;
    AMQP_VALUE read_sequence_1;
    AMQP_VALUE read_sequence_2;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_2))
        .SetReturn(cloned_sequence_2);
    message_add_body_amqp_sequence(message, test_sequence_1);
    message_add_body_amqp_sequence(message, test_sequence_2);
    umock_c_reset_all_calls();

    // act
    result1 = message_get_body_amqp_sequence_in_place(message, 0, &read_sequence_1);
    result2 = message_get_body_amqp_sequence_in_place(message, 1, &read_sequence_2);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result1);
    ASSERT_ARE_EQUAL(int, 0, result2);
    ASSERT_ARE_EQUAL(void_ptr, cloned_sequence_1, read_sequence_1);
    ASSERT_ARE_EQUAL(void_ptr, cloned_sequence_2, read_sequence_2);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_118: [ If `message` or `sequence` is NULL, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_in_place_with_NULL_sequence_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_realloc(IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(amqpvalue_clone(test_sequence_1))
        .SetReturn(cloned_sequence_1);
    message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_in_place(message, 0, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_118: [ If `message` or `sequence` is NULL, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_in_place_with_NULL_message_fails)
{
    // arrange
    int result;
    AMQP_VALUE read_sequence;
    // act
    result = message_get_body_amqp_sequence_in_place(NULL, 0, &read_sequence);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_120: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_SEQUENCE`, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_in_place_when_no_sequences_have_been_added_fails)
{
    // arrange
    int result;
    AMQP_VALUE read_sequence;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_in_place(message, 0, &read_sequence);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_120: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_SEQUENCE`, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_in_place_when_body_type_is_AMQP_DATA_fails)
{
    // arrange
    int result;
    AMQP_VALUE read_sequence;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_in_place(message, 0, &read_sequence);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_120: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_SEQUENCE`, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_in_place_when_body_type_is_AMQP_VALUE_fails)
{
    // arrange
    int result;
    AMQP_VALUE read_sequence;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_in_place(message, 0, &read_sequence);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_119: [ If `index` indicates an AMQP sequence entry that is out of bounds, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_for_index_2_when_only_one_sequence_items_is_in_the_body_fails)
{
    // arrange
    int result;
    AMQP_VALUE read_sequence;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_in_place(message, 1, &read_sequence);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_119: [ If `index` indicates an AMQP sequence entry that is out of bounds, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_for_index_3_when_2_sequence_items_are_in_the_body_fails)
{
    // arrange
    int result;
    AMQP_VALUE read_sequence;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    (void)message_add_body_amqp_sequence(message, test_sequence_2);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_in_place(message, 2, &read_sequence);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_body_amqp_sequence_count */

/* Tests_SRS_MESSAGE_01_121: [ `message_get_body_amqp_sequence_count` shall fill in `count` the number of AMQP sequences that are stored by the message identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_122: [ On success, `message_get_body_amqp_sequence_count` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_count_with_1_sequence_item_returns_1)
{
    // arrange
    int result;
    size_t sequence_count;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_count(message, &sequence_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 1, sequence_count);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_121: [ `message_get_body_amqp_sequence_count` shall fill in `count` the number of AMQP sequences that are stored by the message identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_122: [ On success, `message_get_body_amqp_sequence_count` shall return 0. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_count_with_2_sequence_item_returns_2)
{
    // arrange
    int result;
    size_t sequence_count;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    (void)message_add_body_amqp_sequence(message, test_sequence_2);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_count(message, &sequence_count);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(size_t, 2, sequence_count);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_123: [ If `message` or `count` is NULL, `message_get_body_amqp_sequence_count` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_count_with_NULL_count_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_count(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_123: [ If `message` or `count` is NULL, `message_get_body_amqp_sequence_count` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_count_with_NULL_message_fails)
{
    // arrange
    int result;
    size_t sequence_count;

    // act
    result = message_get_body_amqp_sequence_count(NULL, &sequence_count);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_124: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_SEQUENCE`, `message_get_body_amqp_sequence_count` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_count_when_no_body_was_added_to_the_message_fails)
{
    // arrange
    int result;
    size_t sequence_count;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_count(message, &sequence_count);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_124: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_SEQUENCE`, `message_get_body_amqp_sequence_count` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_count_when_body_is_AMQP_DATA_fails)
{
    // arrange
    int result;
    size_t sequence_count;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_count(message, &sequence_count);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_124: [ If the body for `message` is not of type `MESSAGE_BODY_TYPE_SEQUENCE`, `message_get_body_amqp_sequence_count` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_amqp_sequence_count_when_body_is_AMQP_VALUE_fails)
{
    // arrange
    int result;
    size_t sequence_count;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_amqp_sequence_count(message, &sequence_count);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_get_body_type */

/* Tests_SRS_MESSAGE_01_125: [ `message_get_body_type` shall fill in `body_type` the AMQP message body type. ]*/
/* Tests_SRS_MESSAGE_01_126: [ On success, `message_get_body_type` shall return 0. ]*/
TEST_FUNCTION(message_get_body_type_when_body_is_AMQP_DATA_yields_AMQP_VALUE)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    BINARY_DATA amqp_data;
    unsigned char amqp_data_bytes[] = { 0x42 };
    umock_c_reset_all_calls();

    amqp_data.bytes = amqp_data_bytes;
    amqp_data.length = sizeof(amqp_data_bytes);

    (void)message_add_body_amqp_data(message, amqp_data);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_type(message, &body_type);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_DATA, body_type);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_125: [ `message_get_body_type` shall fill in `body_type` the AMQP message body type. ]*/
/* Tests_SRS_MESSAGE_01_126: [ On success, `message_get_body_type` shall return 0. ]*/
TEST_FUNCTION(message_get_body_type_when_body_is_AMQP_VALUE_yields_AMQP_VALUE)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    umock_c_reset_all_calls();

    (void)message_set_body_amqp_value(message, test_amqp_value_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_type(message, &body_type);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_VALUE, body_type);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_125: [ `message_get_body_type` shall fill in `body_type` the AMQP message body type. ]*/
/* Tests_SRS_MESSAGE_01_126: [ On success, `message_get_body_type` shall return 0. ]*/
TEST_FUNCTION(message_get_body_type_when_body_is_AMQP_SEQUENCE_yields_AMQP_SEQUENCE)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_type(message, &body_type);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_SEQUENCE, body_type);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_127: [ If `message` or `body_type` is NULL, `message_get_body_type` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_type_with_NULL_body_type_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_type(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_127: [ If `message` or `body_type` is NULL, `message_get_body_type` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_body_type_with_NULL_message_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    umock_c_reset_all_calls();

    (void)message_add_body_amqp_sequence(message, test_sequence_1);
    umock_c_reset_all_calls();

    // act
    result = message_get_body_type(NULL, &body_type);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_128: [ If no body has been set on the message, `body_type` shall be `MESSAGE_BODY_TYPE_NONE`. ]*/
TEST_FUNCTION(message_get_body_type_when_no_body_was_set_yields_MESSAGE_BODY_TYPE_NONE)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    MESSAGE_BODY_TYPE body_type;
    umock_c_reset_all_calls();

    // act
    result = message_get_body_type(message, &body_type);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(MESSAGE_BODY_TYPE, MESSAGE_BODY_TYPE_NONE, body_type);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* message_set_message_format */

/* Tests_SRS_MESSAGE_01_129: [ `message_set_message_format` shall set the message format for the message identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_130: [ On success, `message_set_message_format` shall return 0. ]*/
TEST_FUNCTION(message_set_message_format_sets_the_message_format)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_set_message_format(message, 0);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_129: [ `message_set_message_format` shall set the message format for the message identified by `message`. ]*/
/* Tests_SRS_MESSAGE_01_130: [ On success, `message_set_message_format` shall return 0. ]*/
TEST_FUNCTION(message_set_message_format_with_0x42_sets_the_message_format)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_set_message_format(message, 0x42);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_131: [ If `message` is NULL, `message_set_message_format` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_set_message_format_with_NULL_message_fails)
{
    // arrange
    int result;

    // act
    result = message_set_message_format(NULL, 0);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* message_get_message_format */

/* Tests_SRS_MESSAGE_01_132: [ `message_get_message_format` shall get the message format for the message identified by `message` and return it in the `message_fomrat` argument. ]*/
/* Tests_SRS_MESSAGE_01_133: [ On success, `message_get_message_format` shall return 0. ]*/
TEST_FUNCTION(message_get_message_format_gets_the_previously_set_message_format)
{
    // arrange
    int result;
    uint32_t read_message_format;
    MESSAGE_HANDLE message = message_create();
    (void)message_set_message_format(message, 0x42);
    umock_c_reset_all_calls();

    // act
    result = message_get_message_format(message, &read_message_format);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 0x42, read_message_format);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_134: [ If `message` or `message_format` is NULL, `message_get_message_format` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_message_format_with_NULL_message_format_fails)
{
    // arrange
    int result;
    MESSAGE_HANDLE message = message_create();
    (void)message_set_message_format(message, 0x42);
    umock_c_reset_all_calls();

    // act
    result = message_get_message_format(message, NULL);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

/* Tests_SRS_MESSAGE_01_134: [ If `message` or `message_format` is NULL, `message_get_message_format` shall fail and return a non-zero value. ]*/
TEST_FUNCTION(message_get_message_format_with_NULL_message_fails)
{
    // arrange
    int result;
    uint32_t read_message_format;

    // act
    result = message_get_message_format(NULL, &read_message_format);

    // assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_MESSAGE_01_135: [ By default a message on which `message_set_message_format` was not called shall have message format set to 0. ]*/
TEST_FUNCTION(message_get_message_format_without_any_Set_shall_return_the_default_of_0)
{
    // arrange
    int result;
    uint32_t read_message_format;
    MESSAGE_HANDLE message = message_create();
    umock_c_reset_all_calls();

    // act
    result = message_get_message_format(message, &read_message_format);

    // assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(uint32_t, 0, read_message_format);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // cleanup
    message_destroy(message);
}

END_TEST_SUITE(message_ut)
