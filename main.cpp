#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include "httplib.h"
#include "json.hpp"

#define PORT 3030

using json = nlohmann::json;

void add_value_route(httplib::Server &svr, const char *route, signed short (*value_function)()) {
    svr.Get(route, [value_function](const httplib::Request &req, httplib::Response &res) {
        json data;
        data["value"] = value_function();
        res.set_content(data.dump(), "application/json");
    });
}

int main() {
    using namespace httplib;
    void *lib_handle = dlopen("/usr/lib/libkipr.so", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Error loading libkipr.so: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    Server svr;

    add_value_route(svr, "/gyro_x", (signed short (*)()) dlsym(lib_handle, "gyro_x"));
    add_value_route(svr, "/gyro_y", (signed short (*)()) dlsym(lib_handle, "gyro_y"));
    add_value_route(svr, "/gyro_z", (signed short (*)()) dlsym(lib_handle, "gyro_z"));

    add_value_route(svr, "/magneto_x", (signed short (*)()) dlsym(lib_handle, "magneto_x"));
    add_value_route(svr, "/magneto_y", (signed short (*)()) dlsym(lib_handle, "magneto_y"));
    add_value_route(svr, "/magneto_z", (signed short (*)()) dlsym(lib_handle, "magneto_z"));


    svr.listen("0.0.0.0", PORT);
    dlclose(lib_handle);
}