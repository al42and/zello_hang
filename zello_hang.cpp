/*
 *
 * Copyright (C) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
#include <stdlib.h>
#include <memory>
#include "zello_init.h"

//////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
    ze_result_t status;
    const ze_device_type_t type = ZE_DEVICE_TYPE_GPU;

    ze_driver_handle_t pDriver = nullptr;
    ze_device_handle_t pDevice = nullptr;
    if( init_ze() )
    {
        uint32_t driverCount = 0;
        status = zeDriverGet(&driverCount, nullptr);
        if(status != ZE_RESULT_SUCCESS) {
            std::cout << "zeDriverGet Failed with return code: " << to_string(status) << std::endl;
            exit(1);
        }

        std::vector<ze_driver_handle_t> drivers( driverCount );
        status = zeDriverGet( &driverCount, drivers.data() );
        if(status != ZE_RESULT_SUCCESS) {
            std::cout << "zeDriverGet Failed with return code: " << to_string(status) << std::endl;
            exit(1);
        }

        for( uint32_t driver = 0; driver < driverCount; ++driver )
        {
            pDriver = drivers[driver];
            pDevice = findDevice( pDriver, type );
            if( pDevice )
            {
                break;
            }
        }
    }

    if( !pDevice )
    {
        std::cout << "Did NOT find matching " << to_string(type) <<" device!" << "\n";
        return -1;
    }


    // Create the context
    ze_context_handle_t context;
    ze_context_desc_t context_desc = {};
    context_desc.stype = ZE_STRUCTURE_TYPE_CONTEXT_DESC;
    status = zeContextCreate(pDriver, &context_desc, &context);
    if(status != ZE_RESULT_SUCCESS) {
        std::cout << "zeContextCreate Failed with return code: " << to_string(status) << std::endl;
        exit(1);
    }

    // Create an immediate command list for direct submission
    ze_command_queue_desc_t altdesc = {};
    ze_command_list_handle_t command_list_immediate = {};
    status = zeCommandListCreateImmediate(context, pDevice, &altdesc, &command_list_immediate);
    if(status != ZE_RESULT_SUCCESS) {
        std::cout << "zeCommandListCreateImmediate Failed with return code: " << to_string(status) << std::endl;
        exit(1);
    }

    ze_command_list_desc_t cldesc = {};
    ze_command_list_handle_t command_list = {};
    status = zeCommandListCreate(context, pDevice, &cldesc, &command_list);
    if(status != ZE_RESULT_SUCCESS) {
        std::cout << "zeCommandListCreate Failed with return code: " << to_string(status) << std::endl;
        exit(1);
    }

    // Create an event to be signaled by the device
    ze_event_pool_desc_t ep_desc = {};
    ep_desc.stype = ZE_STRUCTURE_TYPE_EVENT_POOL_DESC;
    ep_desc.count = 1;
    ep_desc.flags = ZE_EVENT_POOL_FLAG_HOST_VISIBLE;
    ze_event_desc_t ev_desc = {};
    ev_desc.stype = ZE_STRUCTURE_TYPE_EVENT_DESC;
    ev_desc.signal = ZE_EVENT_SCOPE_FLAG_HOST;
    ev_desc.wait = ZE_EVENT_SCOPE_FLAG_HOST;
    ze_event_handle_t event;
    ze_event_pool_handle_t event_pool;

    status = zeEventPoolCreate(context, &ep_desc, 1, &pDevice, &event_pool);
    if(status != ZE_RESULT_SUCCESS) {
        std::cout << "zeEventPoolCreate Failed with return code: " << to_string(status) << std::endl;
        exit(1);
    }

    status = zeEventCreate(event_pool, &ev_desc, &event);
    if(status != ZE_RESULT_SUCCESS) {
        std::cout << "zeEventCreate Failed with return code: " << to_string(status) << std::endl;
        exit(1);
    }

    std::cout << "Enqueueing event" << std::endl;
    //zeCommandListAppendBarrier(command_list_immediate, event, 0, nullptr);
    //zeCommandListAppendBarrier(command_list, event, 0, nullptr);
    zeCommandListAppendSignalEvent(command_list, event);
    std::cout << "Checking event status" << std::endl;
    status = zeEventHostSynchronize(event, 0);
    std::cout << "Status: " << to_string(status) << std::endl;
    std::cout << "Waiting on the event" << std::endl;
    zeEventHostSynchronize(event, UINT64_MAX);
    std::cout << "Done!" << std::endl;

    zeContextDestroy(context);
    zeCommandListDestroy(command_list);
    zeCommandListDestroy(command_list_immediate);
    zeEventDestroy(event);
    zeEventPoolDestroy(event_pool);

    return 0;
}
