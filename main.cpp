#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include "httplib.h"

#define PORT 3030

void gyro_x(httplib::Server &svr, void *lib_handle) {
    auto (*gyro_x)() = (signed short (*)()) dlsym(lib_handle, "gyro_x");
    if (!gyro_x) {
        fprintf(stderr, "Error getting gyro_x function: %s\n", dlerror());
        dlclose(lib_handle);
        exit(EXIT_FAILURE);
    }

    svr.Get("/gyro_x", [gyro_x](const httplib::Request &req, httplib::Response &res) {
        signed short gyro_x_result = gyro_x();
        res.set_content(std::to_string(gyro_x_result), "text/plain");
    });
}

int main() {
    void *lib_handle = dlopen("/usr/lib/libkipr.so", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Error loading libkipr.so: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    httplib::Server svr;

    gyro_x(svr, lib_handle);

    svr.listen("localhost", PORT);

    dlclose(lib_handle);
    return 0;
}