

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is generated. DO NOT EDIT it manually.
// The generator that produces it is located at /uamqp_generator/uamqp_generator.sln

#ifndef AMQP_DEFINITIONS_DELIVERY_ANNOTATIONS_H
#define AMQP_DEFINITIONS_DELIVERY_ANNOTATIONS_H


#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "azure_uamqp_c/amqpvalue.h"
#include "umock_c/umock_c_prod.h"


    typedef annotations delivery_annotations;

    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_delivery_annotations, delivery_annotations, value);

    MOCKABLE_FUNCTION(, bool, is_delivery_annotations_type_by_descriptor, AMQP_VALUE, descriptor);

    #define amqpvalue_get_delivery_annotations amqpvalue_get_annotations



#ifdef __cplusplus
}
#endif

#endif /* AMQP_DEFINITIONS_DELIVERY_ANNOTATIONS_H */


