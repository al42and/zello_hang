#include <iostream>
#include <stdlib.h>
#include <CL/sycl.hpp>

int main()
{
    cl::sycl::device dev(cl::sycl::gpu_selector{});
    cl::sycl::queue q{dev};
    std::cout << "Device name " << dev.get_info<cl::sycl::info::device::name>() << std::endl;
    std::cout << "Driver version " << dev.get_info<cl::sycl::info::device::driver_version>() << std::endl;

    cl::sycl::event e = q.submit_barrier();
    e.wait_and_throw();
    
    return 0;
}


