/*
 *
 * Copyright (C) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
#include <stdlib.h>
#include <memory>
#include <fenv.h>
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

    FILE* f = fopen("igc_fpe.spv", "rb");
    fseek(f, 0, SEEK_END);
    long spv_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *spv_data = (unsigned char*) malloc(spv_size + 1);
    fread(spv_data, 1, spv_size, f);
    fclose(f);

    ze_module_desc_t module_desc = {
        ZE_STRUCTURE_TYPE_MODULE_DESC,
        nullptr,
        ZE_MODULE_FORMAT_IL_SPIRV,
        spv_size,
        spv_data,
        nullptr,
        nullptr
    };


    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);

    ze_module_handle_t mdl;
    zeModuleCreate(context, pDevice, &module_desc, &mdl, nullptr);

    return 0;
}
