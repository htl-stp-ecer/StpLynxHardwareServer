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

void add_digital_value_route(httplib::Server &svr, const char *route, int (*digital_value_function)(int)) {
    svr.Get(route, [digital_value_function](const httplib::Request &req, httplib::Response &res) {
        try {
            auto json_body = json::parse(req.body);
            if (json_body.find("port") == json_body.end())
                throw std::runtime_error("Missing port");

            int port = json_body["port"];
            json data;
            data["value"] = digital_value_function(port);
            res.set_content(data.dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("Bad Request: Invalid JSON payload", "text/plain");
        }
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

    add_value_route(svr, "/accel_x", (signed short (*)()) dlsym(lib_handle, "accel_x"));
    add_value_route(svr, "/accel_y", (signed short (*)()) dlsym(lib_handle, "accel_y"));
    add_value_route(svr, "/accel_z", (signed short (*)()) dlsym(lib_handle, "accel_z"));

    add_digital_value_route(svr, "/get_digital_value", (int (*)(int)) dlsym(lib_handle, "get_digital_value"));

    svr.listen("0.0.0.0", PORT);
    dlclose(lib_handle);
}