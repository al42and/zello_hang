zello_hang: zello_hang.cpp zello_init.h zello_log.h
	g++ zello_hang.cpp -g -o zello_hang -lze_loader

igc_fpe: igc_fpe.cpp zello_init.h zello_log.h
	g++ igc_fpe.cpp -g -o igc_fpe -lze_loader

sycl_hang: sycl_hang.cpp
	icpx -fsycl sycl_hang.cpp -g -o sycl_hang
